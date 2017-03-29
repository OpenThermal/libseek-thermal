/*
 *  Test program seek lib
 *  Author: Maarten Vandersteegen
 */
#include <opencv2/highgui/highgui.hpp>
#include "seek.h"
#include <iostream>

int main(int argc, char** argv)
{
    /* for improved image quality:
     * create an additional flat field calibration image by running './bin/create_flat_field ffc.png'
     * while covering the camera lens with something. Then uncomment the line below so the driver
     * can use the created image.
     * This image will undo the 'white' gradient at the edges caused by heat scatter of the camera
     * electronics and improves the flat field even more. The downside is that this calibration is
     * ambient temperature sensitive and has to be repeated every time before operating it
     */
    //LibSeek::SeekThermalPro seek(std::string("ffc.png"));
    LibSeek::SeekThermalPro seek;
    cv::Mat frame, grey_frame;

    if (!seek.open()) {
        std::cout << "failed to open seek cam" << std::endl;
        return -1;
    }

    while(1) {
        if (!seek.grab()) {
            std::cout << "no more LWIR img" << endl;
            return -1;
        }

        seek.retrieve(frame);
        cv::normalize(frame, grey_frame, 0, 65535, cv::NORM_MINMAX);
        //cv::GaussianBlur(grey_frame, grey_frame, cv::Size(7,7), 0);

        cv::imshow("LWIR", grey_frame);

        char c = cv::waitKey(10);
        if (c == 's') {
            cv::waitKey(0);
        }
    }
}
