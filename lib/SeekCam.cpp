/*
 *  Seek camera interface
 *  Author: Maarten Vandersteegen
 */

#include "SeekCam.h"
#include "SeekDebug.h"

using namespace LibSeek;

SeekCam::SeekCam(int vendor_id, int product_id, uint16_t* buffer, size_t raw_height, size_t raw_width, cv::Rect roi, std::string ffc_filename) :
    m_offset(0x4000),
    m_ffc_filename(ffc_filename),
    m_is_opened(false),
    m_dev(vendor_id, product_id),
    m_raw_data(buffer),
    m_raw_data_size(raw_height * raw_width),
    m_raw_frame(raw_height,
                raw_width,
                CV_16UC1,
                buffer,
                cv::Mat::AUTO_STEP),
    m_flat_field_calibration_frame(),
    m_additional_ffc(),
    m_dead_pixel_mask()
{
    /* set ROI to exclude metadata frame regions */
    m_raw_frame = m_raw_frame(roi);
}

SeekCam::~SeekCam()
{
    close();
}

bool SeekCam::open()
{
    if (m_ffc_filename != std::string()) {
        m_additional_ffc = cv::imread(m_ffc_filename, cv::ImreadModes::IMREAD_UNCHANGED);

        if (m_additional_ffc.type() != m_raw_frame.type()) {
            debug("Error: '%s' not found or it has the wrong type: %d\n",
                    m_ffc_filename.c_str(), m_additional_ffc.type());
            return false;
        }

        if (m_additional_ffc.size() != m_raw_frame.size()) {
            debug("Error: expected '%s' to have size [%d,%d], got [%d,%d]\n",
                    m_ffc_filename.c_str(),
                    m_raw_frame.cols, m_raw_frame.rows,
                    m_additional_ffc.cols, m_additional_ffc.rows);
            return false;
        }
    }

    return open_cam();
}

void SeekCam::close()
{
    if (m_dev.isOpened()) {
        vector<uint8_t> data = { 0x00, 0x00 };
        m_dev.request_set(DeviceCommand::SET_OPERATION_MODE, data);
        m_dev.request_set(DeviceCommand::SET_OPERATION_MODE, data);
        m_dev.request_set(DeviceCommand::SET_OPERATION_MODE, data);
    }
    m_is_opened = false;
}

bool SeekCam::isOpened()
{
    return m_is_opened;
}

bool SeekCam::grab()
{
    int i;

    for (i=0; i<10; i++) {
        if(!get_frame()) {
            debug("Error: frame acquisition failed\n");
            return false;
        }

        if (frame_id() == 3) {
            return true;

        } else if (frame_id() == 1) {
            m_raw_frame.copyTo(m_flat_field_calibration_frame);
        }
    }

    return false;
}

void SeekCam::retrieve(cv::Mat& dst)
{
    /* apply flat field calibration */
    m_raw_frame += m_offset - m_flat_field_calibration_frame;
    /* filter out dead pixels */
    apply_dead_pixel_filter(m_raw_frame, dst);
    /* apply additional flat field calibration for degradient */
    if (!m_additional_ffc.empty())
        dst += m_offset - m_additional_ffc;
}

bool SeekCam::read(cv::Mat& dst)
{
    if (!grab())
        return false;

    retrieve(dst);

    return true;
}

void SeekCam::convertToGreyScale(cv::Mat& src, cv::Mat& dst)
{
    double tmin, tmax, rsize;
    double rnint = 0;
    double rnstart = 0;
    size_t n;
    size_t num_of_pixels = src.rows * src.cols;

    cv::minMaxLoc(src, &tmin, &tmax);
    rsize = (tmax - tmin) / 10.0;

    for (n=0; n<10; n++) {
        double min = tmin + n * rsize;
        cv::Mat mask;
        cv::Mat temp;

        rnstart += rnint;
        cv::inRange(src, cv::Scalar(min), cv::Scalar(min + rsize), mask);
        /* num_of_pixels_in_range / total_num_of_pixels * 256 */
        rnint = (cv::countNonZero(mask) << 8) / num_of_pixels;

        temp = ((src - min) * rnint) / rsize + rnstart;
        temp.copyTo(dst, mask);
    }
    dst.convertTo(dst, CV_8UC1);
}

bool SeekCam::open_cam()
{
    int i;

    if (!m_dev.open()) {
        debug("Error: open failed\n");
        return false;
    }

    /* init retry loop: sometimes cam skips first 512 bytes of first frame (needed for dead pixel filter) */
    for (i=0; i<3; i++) {
        /* cam specific configuration */
        if (!init_cam()) {
            debug("Error: init_cam failed\n");
            return false;
        }

        if (!get_frame()) {
            debug("Error: first frame acquisition failed, retry attempt %d\n", i+1);
            continue;
        }

        if (frame_id() != 4) {
            debug("Error: expected first frame to have id 4\n");
            return false;
        }

        m_raw_frame.convertTo(m_dead_pixel_mask, CV_8UC1);
        create_dead_pixel_list();

        if (!grab()) {
            debug("Error: first grab failed\n");
            return false;
        }

        m_is_opened = true;
        return true;
    }

    debug("Error: max init retry count exceeded\n");
    return false;
}

bool SeekCam::get_frame()
{
    /* request new frame */
    uint8_t* s = reinterpret_cast<uint8_t*>(&m_raw_data_size);

    vector<uint8_t> data = { s[0], s[1], s[2], s[3] };
    if (!m_dev.request_set(DeviceCommand::START_GET_IMAGE_TRANSFER, data))
        return false;

    /* store frame data */
    if (!m_dev.fetch_frame(m_raw_data, m_raw_data_size))
        return false;

    return true;
}

void SeekCam::print_usb_data(vector<uint8_t>& data)
{
    debug("Response: ");
    for (size_t i = 0; i < data.size(); i++) {
        debug(" %2x", data[i]);
    }
    debug("\n");
}

void SeekCam::create_dead_pixel_list()
{
    int x, y;
    m_dead_pixel_list.clear();

    for (y=0; y<m_dead_pixel_mask.rows; y++) {
        for (x=0; x<m_dead_pixel_mask.cols; x++) {
            if (m_dead_pixel_mask.at<uint8_t>(y, x) == 0) {
                m_dead_pixel_list.push_back(cv::Point(x, y));
            }
        }
    }
}

void SeekCam::apply_dead_pixel_filter(cv::Mat& src, cv::Mat& dst)
{
    size_t i;
    size_t size = m_dead_pixel_list.size();
    int right_border = src.cols - 1;
    int lower_border = src.rows - 1;

    dst.create(src.rows, src.cols, src.type()); /* ensure dst is properly allocated */
    dst.setTo(0xffff);                          /* value to identify dead pixels */
    src.copyTo(dst, m_dead_pixel_mask);         /* only copy non dead pixel values */

    /* replace dead pixel values (0xffff) with the mean of its non dead surrounding pixels */
    for (i=0; i<size; i++) {
        cv::Point p = m_dead_pixel_list[i];
        dst.at<uint16_t>(p) = calc_mean_value(dst, p, right_border, lower_border);
    }
}

uint16_t SeekCam::calc_mean_value(cv::Mat& img, cv::Point p, int right_border, int lower_border)
{
    uint32_t value = 0, temp;
    uint32_t div = 0;

    if (p.x != 0) {
        /* if not on the left border of the image */
        temp = img.at<uint16_t>(p.y, p.x-1);
        if (temp != 0xffff) {
            value += temp;
            div++;
        }
    }
    if (p.x != right_border) {
        /* if not on the right border of the image */
        temp = img.at<uint16_t>(p.y, p.x+1);
        if (temp != 0xffff) {
            value += temp;
            div++;
        }
    }
    if (p.y != 0) {
        // upper
        temp = img.at<uint16_t>(p.y-1, p.x);
        if (temp != 0xffff) {
            value += temp;
            div++;
        }
    }
    if (p.y != lower_border) {
        // lower
        temp = img.at<uint16_t>(p.y+1, p.x);
        if (temp != 0xffff) {
            value += temp;
            div++;
        }
    }

    if (div)
        return (value / div);

    return 0;
}
