/*
 *  Seek thermal interface
 *  Author: Maarten Vandersteegen
 */

#ifndef SEEK_THERMAL_H
#define SEEK_THERMAL_H

#include <opencv2/opencv.hpp>
#include "SeekCam.h"

#define THERMAL_WIDTH       207
#define THERMAL_HEIGHT      154
#define THERMAL_RAW_WIDTH   208
#define THERMAL_RAW_HEIGHT  156
#define THERMAL_RAW_SIZE    (THERMAL_RAW_WIDTH * THERMAL_RAW_HEIGHT)

namespace LibSeek {

class SeekThermal: public SeekCam
{
public:
    SeekThermal();

    uint16_t m_buffer[THERMAL_RAW_SIZE];

    virtual bool init_cam();
    virtual int frame_id();
    virtual int frame_counter();
};

} /* LibSeek */

#endif /* SEEK_THERMAL_H */
