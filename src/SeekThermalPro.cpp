/*
 *  Seek thermal pro implementation
 *  Author: Maarten Vandersteegen
 */

#include "SeekThermalPro.h"
#include "SeekLogging.h"
#include <endian.h>

using namespace LibSeek;

SeekThermalPro::SeekThermalPro() :
    SeekThermalPro(std::string())
{ }

SeekThermalPro::SeekThermalPro(std::string ffc_filename) :
    SeekCam(0x289d, 0x0011, m_buffer,
            THERMAL_PRO_RAW_HEIGHT, THERMAL_PRO_RAW_WIDTH,
            cv::Rect(1, 4, THERMAL_PRO_WIDTH, THERMAL_PRO_HEIGHT), ffc_filename)
{ }

bool SeekThermalPro::init_cam()
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
        std::vector<uint8_t> data = { 0x17, 0x00 };
        if (!m_dev.request_set(DeviceCommand::SET_FIRMWARE_INFO_FEATURES, data))
            return false;
    }
    {
        std::vector<uint8_t> data(64);
        if (!m_dev.request_get(DeviceCommand::GET_FIRMWARE_INFO, data))
            return false;
        print_usb_data(data);
    }
    {
        std::vector<uint8_t> data = { 0x01, 0x00, 0x00, 0x06, 0x00, 0x00 };
        if (!m_dev.request_set(DeviceCommand::SET_FACTORY_SETTINGS_FEATURES, data))
            return false;
    }
    {
        std::vector<uint8_t> data(2);
        if (!m_dev.request_get(DeviceCommand::GET_FACTORY_SETTINGS, data))
            return false;
        print_usb_data(data);
    }
    {
        std::vector<uint8_t> data = { 0x01, 0x00, 0x01, 0x06, 0x00, 0x00 };
        if (!m_dev.request_set(DeviceCommand::SET_FACTORY_SETTINGS_FEATURES, data))
            return false;
    }
    {
        std::vector<uint8_t> data(2);
        if (!m_dev.request_get(DeviceCommand::GET_FACTORY_SETTINGS, data))
            return false;
        print_usb_data(data);
    }

    uint16_t addr, addrle;
    uint8_t *addrle_p = reinterpret_cast<uint8_t*>(&addrle);

    for (addr=0; addr<2560; addr+=32) {
        {
            addrle = htole16(addr); /* mind endianness */
            std::vector<uint8_t> data = { 0x20, 0x00, addrle_p[0], addrle_p[1], 0x00, 0x00 };
            if (!m_dev.request_set(DeviceCommand::SET_FACTORY_SETTINGS_FEATURES, data))
                return false;
        }
        {
            std::vector<uint8_t> data(64);
            if (!m_dev.request_get(DeviceCommand::GET_FACTORY_SETTINGS, data))
                return false;
            print_usb_data(data);
        }
    }

    {
        std::vector<uint8_t> data = { 0x15, 0x00 };
        if (!m_dev.request_set(DeviceCommand::SET_FIRMWARE_INFO_FEATURES, data))
            return false;
    }
    {
        std::vector<uint8_t> data(64);
        if (!m_dev.request_get(DeviceCommand::GET_FIRMWARE_INFO, data))
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

    return true;
}

int SeekThermalPro::frame_id()
{
    return m_raw_data[2];
}

int SeekThermalPro::frame_counter()
{
    return m_raw_data[1];
}

uint16_t SeekThermalPro::device_temp_sensor()
{
    return m_raw_data[5];
}