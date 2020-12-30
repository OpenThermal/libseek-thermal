/*
 *  Test program seek lib
 *  Author: Maarten Vandersteegen
 */
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "seek.h"
#include <iostream>
#include "args.h"
using namespace cv;
using namespace LibSeek;


int main(int argc, char** argv)
{
    int i;
    cv::Mat frame_u16, frame, avg_frame;
    LibSeek::SeekThermalPro seekpro;
    LibSeek::SeekThermal seek;
    LibSeek::SeekCam* cam;

    args::ArgumentParser parser("Capture a single frame");
    args::HelpFlag help(parser, "help", "Display this help menu", { 'h', "help" });
    args::ValueFlag<std::string> _output(parser, "outfile", "Name of the file to write to - default output.png", { 'o', "outfile" });
    args::ValueFlag<int> _smoothing(parser, "smoothing", "Smoothing factor, number of frames to collect and average - default 1", { 's', "smoothing" });
    args::ValueFlag<std::string> _camtype(parser, "camtype", "Seek Thermal Camera Model - seek or seekpro", { 't', "camtype" });
    args::ValueFlag<int> _warmup(parser, "warmup", "Warmup, number of frames to discard before sampling - default 10", { 'w', "warmup" });
    args::ValueFlag<int> _colormap(parser, "colormap", "Color Map - number between 0 and 21 (see: cv::ColormapTypes for maps available in your version of OpenCV)", { 'c', "colormap" });
    args::ValueFlag<int> _rotate(parser, "rotate", "Rotation - 0, 90, 180 or 270 (default) degrees", { 'r', "rotate" });

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
    int smoothing = 1;
    if (_smoothing)
        smoothing = args::get(_smoothing);

    int warmup = 10;
    if (_warmup)
        warmup = args::get(_warmup);

    std::string outfile = "output.png";
    if (_output)
        outfile = args::get(_output);

    std::string camtype = "seek";
    if (_camtype)
        camtype = args::get(_camtype);

    // Colormap int corresponding to enum: http://docs.opencv.org/3.2.0/d3/d50/group__imgproc__colormap.html
    int colormap = -1;
    if (_colormap)
        colormap = args::get(_colormap);

    // Rotate default is landscape view to match camera logo/markings
    int rotate = 270;
    if (_rotate)
        rotate = args::get(_rotate);

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

    Mat frame_g8, outframe; // Transient Mat containers for processing

    normalize(frame_u16, frame_u16, 0, 65535, NORM_MINMAX);

    // Convert seek CV_16UC1 to CV_8UC1
    frame_u16.convertTo(frame_g8, CV_8UC1, 1.0 / 256.0);

    // Apply colormap: https://docs.opencv.org/master/d3/d50/group__imgproc__colormap.html#ga9a805d8262bcbe273f16be9ea2055a65
    if (colormap != -1) {
        applyColorMap(frame_g8, outframe, colormap);
    }
    else {
        cv::cvtColor(frame_g8, outframe, cv::COLOR_GRAY2BGR);
    }

    // Rotate image
    if (rotate == 90) {
        transpose(outframe, outframe);
        flip(outframe, outframe, 1);
    }
    else if (rotate == 180) {
        flip(outframe, outframe, -1);
    }
    else if (rotate == 270) {
        transpose(outframe, outframe);
        flip(outframe, outframe, 0);
    }

    cv::imwrite(outfile, outframe);
    return 0;
}
