/*
 *  Test program seek lib
 *  Author: Maarten Vandersteegen
 */
#include <opencv2/highgui/highgui.hpp>
#include "seek.h"
#include <iostream>
#include "args.h"

int main(int argc, char** argv)
{
    int i;
    cv::Mat frame_u16, frame, avg_frame;
    LibSeek::SeekThermalPro seekpro;
    LibSeek::SeekThermal seek;
    LibSeek::SeekCam* cam;

    args::ArgumentParser parser("Create Flat Frame");
    args::HelpFlag help(parser, "help", "Display this help menu", { 'h', "help" });
    args::ValueFlag<std::string> _output(parser, "outfile", "Name of the file to write to - default flat_field.png", { 'o', "outfile" });
    args::ValueFlag<int> _smoothing(parser, "smoothing", "Smoothing factor, number of frames to collect and average - default 100", { 's', "smoothing" });
    args::ValueFlag<std::string> _camtype(parser, "camtype", "Seek Thermal Camera Model - seek or seekpro", { 't', "camtype" });
    args::ValueFlag<int> _warmup(parser, "warmup", "Warmup, number of frames to discard before sampling - default 10", { 'w', "warmup" });

    // Parse arguments
    try {
        parser.ParseCLI(argc, argv);
    }
    catch (args::Help) {
        std::cout << parser;
        return 0;
    }
    catch (args::ParseError e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }
    catch (args::ValidationError e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }

    // Defaults 
    int smoothing = 100;
    if (_smoothing)
        smoothing = args::get(_smoothing);

    int warmup = 10;
    if (_warmup)
        warmup = args::get(_warmup);

    std::string outfile = "flat_field.png";
    if (_output)
        outfile = args::get(_output);

    std::string camtype = "seek";
    if (_camtype)
        camtype = args::get(_camtype);

    // Init correct cam type
    if (camtype == "seekpro") {
        cam = &seekpro;
    }
    else {
        cam = &seek;
    }

    if (!cam->open()) {
        std::cout << "failed to open " << camtype << " cam" << std::endl;
        return -1;
    }

    for (i = 0; i < warmup; i++) {
        if (!cam->grab()) {
            std::cout << "no more LWIR img" << std::endl;
            return -1;
        }
        cam->retrieve(frame_u16);
        cv::waitKey(10);
    }

    std::cout << "warmup complete" << std::endl;

    // Aquire frames
    for (i = 0; i < smoothing; i++) {
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

    // Average the collected frames
    avg_frame /= smoothing;
    avg_frame.convertTo(frame_u16, CV_16UC1);

    cv::imwrite(outfile, frame_u16);
    return 0;
}
