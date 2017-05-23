/*
 *  Seek device interface
 *  Author: Maarten Vandersteegen
 */

#include <opencv2/opencv.hpp>
#include "SeekDevice.h"
#include "SeekLogging.h"
#include <endian.h>
#include <stdio.h>

using namespace LibSeek;

SeekDevice::SeekDevice(int vendor_id, int product_id, int timeout) :
    m_vendor_id(vendor_id),
    m_product_id(product_id),
    m_timeout(timeout),
    m_is_opened(false),
    m_counter(0) { }

SeekDevice::~SeekDevice()
{
    close();
};

bool SeekDevice::open()
{
    m_is_opened = true;
    return true;
}

void SeekDevice::close()
{
    m_is_opened = false;
}

bool SeekDevice::isOpened()
{
    return m_is_opened;
}

bool SeekDevice::request_set(DeviceCommand::Enum command, vector<uint8_t>& data)
{
    return true;
}

bool SeekDevice::request_get(DeviceCommand::Enum command, vector<uint8_t>& data)
{
    return true;
}

bool SeekDevice::fetch_frame(uint16_t* buffer, size_t size)
{
    cv::Mat frame = cv::imread("lib/frame_" + std::to_string(m_counter++) + ".png", -1);
    if (frame.empty())
        return false;

    /* raw copy image data to output buffer */
    memcpy(buffer, frame.data, 2 * size);
    return true;
}

bool SeekDevice::open_device()
{
    return true;
}


bool SeekDevice::control_transfer(bool direction, uint8_t req, uint16_t value, uint16_t index, vector<uint8_t>& data)
{
    return true;
}

void SeekDevice::correct_endianness(uint16_t* buffer, size_t size)
{
}
