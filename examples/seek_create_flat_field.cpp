/*
 *  Test program seek lib
 *  Author: Maarten Vandersteegen
 */
#include <opencv2/highgui/highgui.hpp>
#include "seek.h"
#include "boost/program_options.hpp"
#include <iostream>

namespace po = boost::program_options;

static po::options_description get_description()
{
    po::options_description description;

    description.add_options()
        ("help,h", "Show this help")
        ("cam,c", po::value<std::string>()->default_value(std::string("seekpro")), "Camera type: choises: seekpro|seek"
                                                                " -> default is 'seekpro'")
        ("smoothing,s", po::value<int>()->default_value(100), "Number of images to average before creating an output image")
        ("outfile", po::value<std::string>()->required(), "Name of the output file");

    return description;
}

static po::variables_map parse_options(int argc, char** argv)
{
    po::positional_options_description positional_description;
    positional_description.add("outfile", -1);
    po::variables_map variables_map;

    auto description = get_description();
    po::store(po::command_line_parser(argc, argv)
                    .options(description)
                    .positional(positional_description)
                    .run(), variables_map);

    if (variables_map.count("help") > 0) {
        return variables_map;
    }

    po::notify(variables_map);
    return variables_map;
}

static void print_help(const char* program_name)
{
    std::cout << "Usage: " << program_name << " [options] outputfilename" << std::endl;
    std::cout << get_description() << std::endl;
}

static bool process_command_line_options(int argc, char **argv, po::variables_map& options)
{
    try
    {
        options = parse_options(argc, argv);
    }
    catch (po::error &e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
        print_help(argv[0]);
        return false;
    }

    if (options.find("help") != options.end())
    {
        print_help(argv[0]);
        return false;
    }

    return true;
}

int main(int argc, char** argv)
{
    int i;
    cv::Mat frame_u16, frame, avg_frame;
    LibSeek::SeekThermalPro seekpro;
    LibSeek::SeekThermal seek;
    LibSeek::SeekCam* cam;
    po::variables_map options;

    if (!process_command_line_options(argc, argv, options))
        return -1;

    int smoothing = options["smoothing"].as<int>();
    std::string outfile = options["outfile"].as<std::string>();

    if (options["cam"].as<std::string>() == "seekpro")
        cam = &seekpro;
    else
        cam = &seek;

    if (!cam->open()) {
        std::cout << "failed to open seek cam" << std::endl;
        return -1;
    }

    for (i=0; i<smoothing; i++) {

        if (!cam->grab()) {
            std::cout << "no more LWIR img" << std::endl;
            return -1;
        }

        cam->retrieve(frame_u16);
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
