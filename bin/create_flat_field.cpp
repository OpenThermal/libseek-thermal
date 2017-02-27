/*
 *  Test program seek lib
 *  Author: Maarten Vandersteegen
 */
#include <opencv2/highgui/highgui.hpp>
#include "seek.h"
#include "boost/program_options.hpp"
#include <iostream>


/* ugly boost option parser that does not seem to have a pretty printer :( */
static bool parse_command_line_options(int argc, char** argv, int& smoothing, std::string& outfile)
{

    namespace po = boost::program_options;
    po::variables_map map;
    try
    {
        po::options_description desc("Options");
        desc.add_options()
            ("smoothing,s", po::value<int>(&smoothing)->default_value(100))
            ("outfile", po::value<std::string>(&outfile)->required());

        po::positional_options_description posdescr;
        posdescr.add("outfile", -1);

        po::store(po::command_line_parser(argc, argv).options(desc).positional(posdescr).run(), map);
        po::notify(map);
    }
    catch (const po::error &ex)
    {
        /* doing it myself then... */
        std::cout << "Usage: " << argv[0] << " [OPTIONS] outfile" << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "\t-s, --smoothing\t number of frames to accumulate to average out the static noise" << std::endl;
        std::cerr << ex.what() << std::endl;
        return false;
    }
    return true;
}

int main(int argc, char** argv)
{
    int i;
    cv::Mat frame_u16, frame, avg_frame;
    LibSeek::SeekThermalPro seek;
    std::string outfile;
    int smoothing;

    if (!parse_command_line_options(argc, argv, smoothing, outfile))
        return -1;

    if (!seek.open()) {
        std::cout << "failed to open seek cam" << std::endl;
        return -1;
    }

    for (i=0; i<smoothing; i++) {

        if (!seek.grab()) {
            std::cout << "no more LWIR img" << endl;
            return -1;
        }

        seek.retrieve(frame_u16);
        frame_u16.convertTo(frame, CV_32FC1);

        if (avg_frame.rows == 0) {
            frame.copyTo(avg_frame);
        } else {
            avg_frame += frame;
        }

        cv::waitKey(10);
    }

    avg_frame /= smoothing;
    avg_frame.convertTo(frame_u16, CV_16UC1);

    cv::imwrite(outfile, frame_u16);

    return 0;
}
