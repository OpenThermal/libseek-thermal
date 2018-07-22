/*
 *  Seek thermal implementation
 *  Author: Maarten Vandersteegen
 */

#include "SeekThermal.h"
#include "SeekLogging.h"
#include <endian.h>

using namespace LibSeek;

SeekThermal::SeekThermal() :
    SeekThermal(std::string())
{ }

SeekThermal::SeekThermal(std::string ffc_filename) :
    SeekCam(0x289d, 0x0010, m_buffer,
            THERMAL_RAW_HEIGHT, THERMAL_RAW_WIDTH,
            cv::Rect(0, 1, THERMAL_WIDTH, THERMAL_HEIGHT), ffc_filename)
{ }

bool SeekThermal::init_cam()
{
    {
        std::vector<uint8_t> data = { 0x01 };
        if (!m_dev.request_set(DeviceCommand::TARGET_PLATFORM, data)) {
            /* deinit and retry if cam was not properly closed */
            close();
            if (!m_dev.request_set(DeviceCommand::TARGET_PLATFORM, data))
                return false;
        }
    }
    {
        std::vector<uint8_t> data = { 0x00, 0x00 };
        if (!m_dev.request_set(DeviceCommand::SET_OPERATION_MODE, data))
            return false;
    }
    {
        std::vector<uint8_t> data(4);
        if (!m_dev.request_get(DeviceCommand::GET_FIRMWARE_INFO, data))
            return false;
        print_usb_data(data);
    }
    {
        std::vector<uint8_t> data(12);
        if (!m_dev.request_get(DeviceCommand::READ_CHIP_ID, data))
            return false;
        print_usb_data(data);
    }
    {
        std::vector<uint8_t> data = { 0x20, 0x00, 0x30, 0x00, 0x00, 0x00 };
        if (!m_dev.request_set(DeviceCommand::SET_FACTORY_SETTINGS_FEATURES, data))
            return false;
    }
    {
        std::vector<uint8_t> data(64);
        if (!m_dev.request_get(DeviceCommand::GET_FACTORY_SETTINGS, data))
            return false;
        print_usb_data(data);
    }
    {
        std::vector<uint8_t> data = { 0x20, 0x00, 0x50, 0x00, 0x00, 0x00 };
        if (!m_dev.request_set(DeviceCommand::SET_FACTORY_SETTINGS_FEATURES, data))
            return false;
    }
    {
        std::vector<uint8_t> data(64);
        if (!m_dev.request_get(DeviceCommand::GET_FACTORY_SETTINGS, data))
            return false;
        print_usb_data(data);
    }
    {
        std::vector<uint8_t> data = { 0x0c, 0x00, 0x70, 0x00, 0x00, 0x00 };
        if (!m_dev.request_set(DeviceCommand::SET_FACTORY_SETTINGS_FEATURES, data))
            return false;
    }
    {
        std::vector<uint8_t> data(24);
        if (!m_dev.request_get(DeviceCommand::GET_FACTORY_SETTINGS, data))
            return false;
        print_usb_data(data);
    }
    {
        std::vector<uint8_t> data = { 0x06, 0x00, 0x08, 0x00, 0x00, 0x00 };
        if (!m_dev.request_set(DeviceCommand::SET_FACTORY_SETTINGS_FEATURES, data))
            return false;
    }
    {
        std::vector<uint8_t> data(12);
        if (!m_dev.request_get(DeviceCommand::GET_FACTORY_SETTINGS, data))
            return false;
        print_usb_data(data);
    }
    {
        std::vector<uint8_t> data = { 0x08, 0x00 };
        if (!m_dev.request_set(DeviceCommand::SET_IMAGE_PROCESSING_MODE, data))
            return false;
    }
    {
        std::vector<uint8_t> data(2);
        if (!m_dev.request_get(DeviceCommand::GET_OPERATION_MODE, data))
            return false;
        print_usb_data(data);
    }
    {
        std::vector<uint8_t> data = { 0x08, 0x00 };
        if (!m_dev.request_set(DeviceCommand::SET_IMAGE_PROCESSING_MODE, data))
            return false;
    }
    {
        std::vector<uint8_t> data = { 0x01, 0x00 };
        if (!m_dev.request_set(DeviceCommand::SET_OPERATION_MODE, data))
            return false;
    }
    {
        std::vector<uint8_t> data(2);
        if (!m_dev.request_get(DeviceCommand::GET_OPERATION_MODE, data))
            return false;
        print_usb_data(data);
    }

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

uint16_t SeekThermal::device_temp_sensor()
{
    return m_raw_data[1];
}