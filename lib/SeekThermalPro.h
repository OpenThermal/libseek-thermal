/*
 *  Seek thermal pro interface
 *  Author: Maarten Vandersteegen
 */

#ifndef SEEK_THERMAL_PRO_H
#define SEEK_THERMAL_PRO_H

#include <opencv2/opencv.hpp>
#include "SeekCam.h"

#define THERMAL_PRO_WIDTH       320
#define THERMAL_PRO_HEIGHT      240
#define THERMAL_PRO_RAW_WIDTH   342
#define THERMAL_PRO_RAW_HEIGHT  260
#define THERMAL_PRO_RAW_SIZE    (THERMAL_PRO_RAW_WIDTH * THERMAL_PRO_RAW_HEIGHT)

namespace LibSeek {

class SeekThermalPro: public SeekCam
{
public:
    SeekThermalPro();

    uint16_t m_buffer[THERMAL_PRO_RAW_SIZE];

    virtual bool init_cam();
    virtual int frame_id();
    virtual int frame_counter();
};

} /* LibSeek */

#endif /* SEEK_THERMAL_PRO_H */
