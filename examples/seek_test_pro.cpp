/*
 *  Test program SEEK Thermal CompactPRO
 *  Author: Maarten Vandersteegen
 */
#include <opencv2/highgui/highgui.hpp>
#include "seek.h"
#include <iostream>

int main(int argc, char** argv)
{
    LibSeek::SeekThermalPro seek(argc == 2 ? argv[1] : "");
    cv::Mat frame, grey_frame;

    if (!seek.open()) {
        std::cout << "failed to open seek cam" << std::endl;
        return -1;
    }

    while(1) {
        if (!seek.read(frame)) {
            std::cout << "no more LWIR img" << std::endl;
            return -1;
        }

        cv::normalize(frame, grey_frame, 0, 65535, cv::NORM_MINMAX);
        //cv::GaussianBlur(grey_frame, grey_frame, cv::Size(7,7), 0);

        cv::imshow("LWIR", grey_frame);

        char c = cv::waitKey(10);
        if (c == 's') {
            cv::waitKey(0);
        }
    }
}
