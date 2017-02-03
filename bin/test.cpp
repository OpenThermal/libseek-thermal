/*
 *  Test program seek lib
 *  Author: Maarten Vandersteegen
 */
#include <opencv2/highgui/highgui.hpp>
#include "seek.h"
#include <iostream>

int main(int argc, char** argv)
{
    LibSeek::SeekThermalPro seek;
    cv::Mat frame;

    if (!seek.open()) {
        std::cout << "failed to open seek cam" << std::endl;
        return -1;
    }

    while(1) {
		if (!seek.grab()) {
            std::cout << "no more LWIR img" << endl;
            return -1;
        }

		seek.retrieveRaw(frame);
        cv::normalize(frame, frame, 0, 65535, cv::NORM_MINMAX);

        cv::imshow("LWIR", frame);

        char c = cv::waitKey(10);
        if (c == 's') {
            cv::waitKey(0);
        }
    }
}
