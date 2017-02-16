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

		seek.retrieve(frame);
        // covert to 8-bit grayscale
        //frame_raw.convertTo(frame, CV_8UC1, 0.015625);
        cv::normalize(frame, frame, 0, 65535, cv::NORM_MINMAX);
        cv::GaussianBlur(frame, frame, cv::Size(3,3), 0);

        cv::imshow("LWIR", frame);

        char c = cv::waitKey(10);
        if (c == 's') {
            cv::waitKey(0);
        }
    }
}
