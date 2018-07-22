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
    /*
     *  ffc_filename:
     *      Filename for additional flat field calibration and corner
     *      gradient elimination. If provided and found, the image will
     *      be subtracted from each retrieved frame. If not, no additional
     *      flat field calibration will be applied
     */
    SeekThermalPro(std::string ffc_filename);

    virtual bool init_cam();
    virtual int frame_id();
    virtual int frame_counter();
    virtual uint16_t device_temp_sensor();

private:
    uint16_t m_buffer[THERMAL_PRO_RAW_SIZE];

};

} /* LibSeek */

#endif /* SEEK_THERMAL_PRO_H */
