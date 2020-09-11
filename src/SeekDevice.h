/*
 *  Seek device interface
 *  Author: Maarten Vandersteegen
 */

#ifndef SEEK_DEVICE_H
#define SEEK_DEVICE_H

#include <vector>
#include <cstdint>

/* forward struct declarations for libusb stuff */
struct libusb_context;
struct libusb_device_handle;

namespace LibSeek {

struct DeviceCommand {
    enum Enum {
     BEGIN_MEMORY_WRITE              = 82,
     COMPLETE_MEMORY_WRITE           = 81,
     GET_BIT_DATA                    = 59,
     GET_CURRENT_COMMAND_ARRAY       = 68,
     GET_DATA_PAGE                   = 65,
     GET_DEFAULT_COMMAND_ARRAY       = 71,
     GET_ERROR_CODE                  = 53,
     GET_FACTORY_SETTINGS            = 88,
     GET_FIRMWARE_INFO               = 78,
     GET_IMAGE_PROCESSING_MODE       = 63,
     GET_OPERATION_MODE              = 61,
     GET_RDAC_ARRAY                  = 77,
     GET_SHUTTER_POLARITY            = 57,
     GET_VDAC_ARRAY                  = 74,
     READ_CHIP_ID                    = 54,
     RESET_DEVICE                    = 89,
     SET_BIT_DATA_OFFSET             = 58,
     SET_CURRENT_COMMAND_ARRAY       = 67,
     SET_CURRENT_COMMAND_ARRAY_SIZE  = 66,
     SET_DATA_PAGE                   = 64,
     SET_DEFAULT_COMMAND_ARRAY       = 70,
     SET_DEFAULT_COMMAND_ARRAY_SIZE  = 69,
     SET_FACTORY_SETTINGS            = 87,
     SET_FACTORY_SETTINGS_FEATURES   = 86,
     SET_FIRMWARE_INFO_FEATURES      = 85,
     SET_IMAGE_PROCESSING_MODE       = 62,
     SET_OPERATION_MODE              = 60,
     SET_RDAC_ARRAY                  = 76,
     SET_RDAC_ARRAY_OFFSET_AND_ITEMS = 75,
     SET_SHUTTER_POLARITY            = 56,
     SET_VDAC_ARRAY                  = 73,
     SET_VDAC_ARRAY_OFFSET_AND_ITEMS = 72,
     START_GET_IMAGE_TRANSFER        = 83,
     TARGET_PLATFORM                 = 84,
     TOGGLE_SHUTTER                  = 55,
     UPLOAD_FIRMWARE_ROW_SIZE        = 79,
     WRITE_MEMORY_DATA               = 80,
    };
};

class SeekDevice
{
public:
    /*
     *  Constructor
     *  vendor_id:  usb vendor id
     *  product_id: usb product id
     *  timeout:    timeout usb requests
     */
    SeekDevice(int vendor_id, int product_id, int timeout=500);

    ~SeekDevice();

    /*
     *  Open usb device interface
     *  Returns true on success
     */
    bool open();

    /*
     *  Close usb device interface
     */
    void close();

    /*
     *  Return true when usb device is opened
     */
    bool isOpened();

    /*
     *  vendor specific requests for setting data
     *  command:    request command
     *  data:       configuration data to send
     *  Returns true on success
     */
    bool request_set(DeviceCommand::Enum command, std::vector<uint8_t>& data);

    /*
     *  vendor specific requests for getting data
     *  command:    request command
     *  data:       will be filled with retrieved configuration data
     *  Returns true on success
     */
    bool request_get(DeviceCommand::Enum command, std::vector<uint8_t>& data);

    /*
     *  get a raw camera frame previously requested
     *  buffer:         buffer to store the received frame
     *  size:           number of uint16_t words the buffer can hold
     *  request_size:   number of bytes to request in each read
     *  Returns true on success
     */
    bool fetch_frame(uint16_t* buffer, std::size_t size, std::size_t request_size);

private:
    int m_vendor_id;
    int m_product_id;
    int m_timeout;
    bool m_is_opened;

    struct libusb_context* m_ctx;
    struct libusb_device_handle* m_handle;

    bool open_device();
    bool control_transfer(bool direction, uint8_t req, uint16_t value, uint16_t index, std::vector<uint8_t>& data);
    void correct_endianness(uint16_t* buffer, std::size_t size);
};

} /* LibSeek */

#endif /* SEEK_DEVICE_H */
