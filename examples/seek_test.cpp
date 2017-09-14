/*
 *  Test program seek lib
 *  Author: Maarten Vandersteegen
 */
#include <opencv2/highgui/highgui.hpp>
#include "seek.h"
#include <iostream>
#include <memory>

int main(int argc, char** argv)
{
    std::shared_ptr<LibSeek::SeekCam> seek;
    cv::Mat frame, grey_frame;

    std::string ffc_file = argc == 2 ? argv[1] : "";
    seek = std::make_shared<LibSeek::SeekThermal>(ffc_file);

    if (!seek->open()) {
        std::cout << "failed to open seek cam" << std::endl;
        return -1;
    }

    while(1) {
        if (!seek->read(frame)) {
            std::cout << "no more LWIR img" << std::endl;
            return -1;
        }

        cv::normalize(frame, grey_frame, 0, 65535, cv::NORM_MINMAX);
        cv::imshow("LWIR", grey_frame);

        char c = cv::waitKey(10);
        if (c == 's') {
            cv::waitKey(0);
        }
    }
}
