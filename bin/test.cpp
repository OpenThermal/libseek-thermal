/*
 *  Test program seek lib
 *  Author: Maarten Vandersteegen
 */
#include <opencv2/highgui/highgui.hpp>
#include "seek.h"
#include <iostream>

int main(int argc, char** argv)
{
    LibSeek::SeekThermal seek(argc == 2 ? argv[1] : "");
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

        cv::imshow("LWIR", grey_frame);

        char c = cv::waitKey(0);
        if (c == 's') {
            cv::waitKey(0);
        }
    }
}
