/*
 *  Seek camera interface
 *  Author: Maarten Vandersteegen
 */

#include "SeekCam.h"

using namespace LibSeek;

bool SeekCam::isOpened()
{
    return m_is_open;
}

bool SeekCam::read(cv::Mat& dst)
{
	if (!grab())
        return false;

    if (!retrieve(dst))
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

