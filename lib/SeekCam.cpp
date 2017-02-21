/*
 *  Seek camera interface
 *  Author: Maarten Vandersteegen
 */

#include "SeekCam.h"
#include "SeekDebug.h"

using namespace LibSeek;

SeekCam::SeekCam(int vendor_id, int product_id, uint16_t* buffer, size_t raw_height, size_t raw_width, cv::Rect roi) :
    m_offset(0x4000),
    m_is_opened(false),
    m_dev(vendor_id, product_id),
    m_raw_data(buffer),
    m_raw_data_size(raw_height * raw_width),
    m_raw_frame(raw_height,
                raw_width,
                CV_16UC1,
                buffer,
                cv::Mat::AUTO_STEP),
    m_frame(roi.height, roi.width, CV_16UC1)
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

bool SeekCam::retrieve(cv::Mat& dst)
{
    /* apply flat field calibration */
    m_raw_frame += m_offset;
    m_raw_frame -= m_flat_field_calibration_frame;
    /* filter out dead pixels */
    apply_dead_pixel_filter();
    dst = m_frame;

    return true;
}

bool SeekCam::read(cv::Mat& dst)
{
    if (!grab())
        return false;

    if (!retrieve(dst))
        return false;

    return true;
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

void SeekCam::apply_dead_pixel_filter()
{
    size_t i;
    size_t size = m_dead_pixel_list.size();
    int right_border = m_frame.cols - 1;
    int lower_border = m_frame.rows - 1;

    m_frame.setTo(0xffff);                              /* value to identify dead pixels */
    m_raw_frame.copyTo(m_frame, m_dead_pixel_mask);     /* only copy non dead pixel values */

    /* replace dead pixel values (0xffff) with the mean of its non dead surrounding pixels */
    for (i=0; i<size; i++) {
        cv::Point p = m_dead_pixel_list[i];
        m_frame.at<uint16_t>(p) = calc_mean_value(p, right_border, lower_border);
    }
}

uint16_t SeekCam::calc_mean_value(cv::Point p, int right_border, int lower_border)
{
    uint32_t value = 0, temp;
    uint32_t div = 0;

    if (p.x != 0) {
        /* if not on the left border of the image */
        temp = m_frame.at<uint16_t>(p.y, p.x-1);
        if (temp != 0xffff) {
            value += temp;
            div++;
        }
    }
    if (p.x != right_border) {
        /* if not on the right border of the image */
        temp = m_frame.at<uint16_t>(p.y, p.x+1);
        if (temp != 0xffff) {
            value += temp;
            div++;
        }
    }
    if (p.y != 0) {
        // upper
        temp = m_frame.at<uint16_t>(p.y-1, p.x);
        if (temp != 0xffff) {
            value += temp;
            div++;
        }
    }
    if (p.y != lower_border) {
        // lower
        temp = m_frame.at<uint16_t>(p.y+1, p.x);
        if (temp != 0xffff) {
            value += temp;
            div++;
        }
    }

    if (div)
        return (value / div);

    return 0;
}
