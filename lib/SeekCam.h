/*
 *  Abstract seek camera class
 *  Author: Maarten Vandersteegen
 */

#ifndef SEEK_CAM_H
#define SEEK_CAM_H

#include "SeekDevice.h"
#include <opencv2/opencv.hpp>

namespace LibSeek {

class SeekCam
{
public:
    /*
     *  Initialize the camera
     *  Returns true on success
     */
    virtual bool open() = 0;

    /*
     *  Returns true when camera is initialized
     */
    bool isOpened();

    /*
     *  Grab a frame
     *  Returns true on success
     */
    virtual bool grab() = 0;

    /*void frameToHexString(char* filename);*/

    /*
     *  Retrieve the last grabbed frame
     *  Returns true on success
     */
    virtual bool retrieve(cv::Mat& dst) = 0;
    virtual bool retrieveRaw(cv::Mat& dst) = 0;

    /*
     *  Read grabs and retrieves a frame
     *  Returns true on success
     */
    bool read(cv::Mat& dst);

protected:
    bool m_is_open = false;
    int m_frame_counter;
    void print_usb_data(vector<uint8_t>& data);

};

} /* LibSeek */

#endif /* SEEK_CAM_H */
