/*
 *  Seek thermal implementation
 *  Author: Maarten Vandersteegen
 */

#include "SeekThermal.h"
#include <endian.h>

using namespace LibSeek;

SeekThermal::SeekThermal() :
    m_offset(0x4000),
    m_is_opened(false),
    m_dev(0x289d, 0x0010),
    m_raw_frame(THERMAL_RAW_HEIGHT,
                THERMAL_RAW_WIDTH,
                CV_16UC1,
                m_raw_data,
                cv::Mat::AUTO_STEP),
    m_frame(THERMAL_HEIGHT, THERMAL_WIDTH, CV_16UC1)
{
    /* set ROI to exclude metadata frame regions */
    m_raw_frame = m_raw_frame(cv::Rect(0, 1, THERMAL_WIDTH, THERMAL_HEIGHT));
}

SeekThermal::~SeekThermal()
{
    close();
}

bool SeekThermal::open()
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

void SeekThermal::close()
{
    if (m_dev.isOpened()) {
        vector<uint8_t> data = { 0x00, 0x00 };
        m_dev.request_set(DeviceCommand::SET_OPERATION_MODE, data);
        m_dev.request_set(DeviceCommand::SET_OPERATION_MODE, data);
        m_dev.request_set(DeviceCommand::SET_OPERATION_MODE, data);
    }
    m_is_opened = false;
}

bool SeekThermal::isOpened()
{
    return m_is_opened;
}

bool SeekThermal::grab()
{
    int i;

	for (i=0; i<10; i++) {
		if(!get_frame()) {
            debug("Error: frame acquisition failed\n");
			return false;
        }

		m_frame_counter = frame_counter();

		if (frame_id() == 3) {
			return true;

		} else if (frame_id() == 1) {
			m_raw_frame.copyTo(m_flat_field_calibration_frame);
        }
	}

	return false;
}

bool SeekThermal::retrieve(cv::Mat& dst)
{
    /* apply flat field calibration */
	m_raw_frame += m_offset;
    m_raw_frame -= m_flat_field_calibration_frame;
    /* filter out dead pixels */
    apply_dead_pixel_filter();
    dst = m_frame;

	return true;
}

// TODO keep as specific method
bool SeekThermal::init_cam()
{
    {
        vector<uint8_t> data = { 0x01 };
        if (!m_dev.request_set(DeviceCommand::TARGET_PLATFORM, data)) {
            /* deinit and retry if cam was not properly closed */
            close();
            if (!m_dev.request_set(DeviceCommand::TARGET_PLATFORM, data))
                return false;
        }
    }
    {
		vector<uint8_t> data = { 0x00, 0x00 };
        if (!m_dev.request_set(DeviceCommand::SET_OPERATION_MODE, data))
            return false;
    }
    {
		vector<uint8_t> data(4);
        if (!m_dev.request_get(DeviceCommand::GET_FIRMWARE_INFO, data))
            return false;
        print_usb_data(data);
    }
    {
		vector<uint8_t> data(12);
        if (!m_dev.request_get(DeviceCommand::READ_CHIP_ID, data))
            return false;
        print_usb_data(data);
    }
    {
		//vector<uint8_t> data = { 0x20, 0x00, 0x30, 0x00, 0x00, 0x00 };
		vector<uint8_t> data = { 0x60, 0x00, 0x80, 0x00, 0x00, 0x00 };
        if (!m_dev.request_set(DeviceCommand::SET_FACTORY_SETTINGS_FEATURES, data))
            return false;
    }
    {
		vector<uint8_t> data(64);
        if (!m_dev.request_get(DeviceCommand::GET_FACTORY_SETTINGS, data))
            return false;
        print_usb_data(data);
    }
    {
		vector<uint8_t> data = { 0x20, 0x00, 0x50, 0x00, 0x00, 0x00 };
        if (!m_dev.request_set(DeviceCommand::SET_FACTORY_SETTINGS_FEATURES, data))
            return false;
    }
    {
		vector<uint8_t> data(64);
        if (!m_dev.request_get(DeviceCommand::GET_FACTORY_SETTINGS, data))
            return false;
        print_usb_data(data);
    }
    {
		vector<uint8_t> data = { 0x0c, 0x00, 0x70, 0x00, 0x00, 0x00 };
        if (!m_dev.request_set(DeviceCommand::SET_FACTORY_SETTINGS_FEATURES, data))
            return false;
    }
    {
		vector<uint8_t> data(24);
        if (!m_dev.request_get(DeviceCommand::GET_FACTORY_SETTINGS, data))
            return false;
        print_usb_data(data);
    }
    {
		vector<uint8_t> data = { 0x06, 0x00, 0x08, 0x00, 0x00, 0x00 };
        if (!m_dev.request_set(DeviceCommand::SET_FACTORY_SETTINGS_FEATURES, data))
            return false;
    }
    {
		vector<uint8_t> data(12);
        if (!m_dev.request_get(DeviceCommand::GET_FACTORY_SETTINGS, data))
            return false;
        print_usb_data(data);
    }
    {
		vector<uint8_t> data = { 0x08, 0x00 };
        if (!m_dev.request_set(DeviceCommand::SET_IMAGE_PROCESSING_MODE, data))
            return false;
    }
    {
		vector<uint8_t> data(2);
        if (!m_dev.request_get(DeviceCommand::GET_OPERATION_MODE, data))
            return false;
        print_usb_data(data);
    }
    {
		vector<uint8_t> data = { 0x08, 0x00 };
        if (!m_dev.request_set(DeviceCommand::SET_IMAGE_PROCESSING_MODE, data))
            return false;
    }
    {
		vector<uint8_t> data = { 0x01, 0x00 };
        if (!m_dev.request_set(DeviceCommand::SET_IMAGE_PROCESSING_MODE, data))
            return false;
    }
    {
		vector<uint8_t> data(2);
        if (!m_dev.request_get(DeviceCommand::GET_OPERATION_MODE, data))
            return false;
        print_usb_data(data);
    }

    return true;
}

bool SeekThermal::get_frame()
{
    /* request new frame (number of pixels) */
    /* TODO: data = raw number of pixels */
    vector<uint8_t> data = { 0xC0, 0x7E, 0, 0 };

    if (!m_dev.request_set(DeviceCommand::START_GET_IMAGE_TRANSFER, data))
        return false;

    /* store frame data */
    if (!m_dev.fetch_frame(m_raw_data, THERMAL_RAW_SIZE))
        return false;

    return true;
}

int SeekThermal::frame_id()
{
	return m_raw_data[10];
}

int SeekThermal::frame_counter()
{
	return m_raw_data[40];
}

void SeekThermal::create_dead_pixel_list()
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

void SeekThermal::apply_dead_pixel_filter()
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

uint16_t SeekThermal::calc_mean_value(cv::Point p, int right_border, int lower_border)
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
