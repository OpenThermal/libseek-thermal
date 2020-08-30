// Seek Thermal Viewer/Streamer
// http://github.com/fnoop/maverick

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "seek.h"
#include "SeekCam.h"
#include <iostream>
#include <string>
#include <signal.h>
#include <math.h>
#include <memory>
#include "args.h"
#include <sstream>
#include <fcntl.h>

#ifdef __linux__
#include <linux/videodev2.h>
#include <unistd.h>
#include <sys/ioctl.h>
#endif

using namespace cv;
using namespace LibSeek;

const std::string WINDOW_NAME = "SeekThermal";

// Setup sig handling
static volatile sig_atomic_t sigflag = 0;

void handle_sig(int sig) {
    (void)sig;
    sigflag = 1;
}

double device_sensor_to_k(double sensor) {
    // formula from http://aterlux.ru/article/ntcresistor-en
    double ref_temp = 297.0; // 23C from table
    double ref_sensor = 6616.0; // ref value from table
    double beta = 200; // best beta coef we've found
    double part3 = log(sensor) - log(ref_sensor);
    double parte = part3 / beta + 1.0 / ref_temp;
    return 1.0 / parte;
}

double temp_from_raw(int x, double device_k) {
    // Constants below are taken from linear trend line in Excel.
    // -273 is translation of Kelvin to Celsius
    // 330 is max temperature supported by Seek device
    // 16384 is full 14 bits value, max possible ()
    double base = x * 330 / 16384.0;
    double lin_k = -1.5276; // derived from Excel linear model
    double lin_offset= -470.8979; // same Excel model
    return base - device_k * lin_k + lin_offset - 273.0;
}

void overlay_values(Mat &outframe, Point coord, Scalar color) {
    int gap=2;
    int arrLen=7;
    int weight=1;
    line(outframe, coord-Point(-arrLen, -arrLen), coord-Point(-gap, -gap), color, weight);
    line(outframe, coord-Point(arrLen, arrLen), coord-Point(gap, gap), color, weight);
    line(outframe, coord-Point(-arrLen, arrLen), coord-Point(-gap, gap), color, weight);
    line(outframe, coord-Point(arrLen, -arrLen), coord-Point(gap, -gap), color, weight);
}

void draw_temp(Mat &outframe, double temp, Point coord, Scalar color) {
    char txt [64];
    sprintf(txt, "%5.1f", temp);
    putText(outframe, txt, coord-Point(20, -20), FONT_HERSHEY_COMPLEX_SMALL, 0.75, color, 1, CV_AA);
}

// Function to process a raw (corrected) seek frame
void process_frame(Mat &inframe, Mat &outframe, float scale, int colormap, int rotate, int device_temp_sensor) {
    Mat frame_g8_nograd, frame_g16; // Transient Mat containers for processing

    // get raw max/min/central values
    double min, max, central;
    minMaxIdx(inframe, &min, &max);
    Scalar valat=inframe.at<uint16_t>(Point(inframe.cols/2.0, inframe.rows/2.0));
    central=valat[0];

    double device_k=device_sensor_to_k(device_temp_sensor);

    double mintemp=temp_from_raw(min, device_k);
    double maxtemp=temp_from_raw(max, device_k);
    double centraltemp=temp_from_raw(central, device_k);

    printf("rmin,rmax,central,devtempsns: %d %d %d %d\t", (int)min, (int)max, (int)central, (int)device_temp_sensor);
    printf("min-max-center-device: %.1f %.1f %.1f %.1f\n", mintemp, maxtemp, centraltemp, device_k-273.0);

    normalize(inframe, frame_g16, 0, 65535, NORM_MINMAX);

    // Convert seek CV_16UC1 to CV_8UC1
    frame_g16.convertTo(frame_g8_nograd, CV_8UC1, 1.0/256.0 );

    // Rotate image
    if (rotate == 90) {
        transpose(frame_g8_nograd, frame_g8_nograd);
        flip(frame_g8_nograd, frame_g8_nograd, 1);
    } else if (rotate == 180) {
        flip(frame_g8_nograd, frame_g8_nograd, -1);
    } else if (rotate == 270) {
        transpose(frame_g8_nograd, frame_g8_nograd);
        flip(frame_g8_nograd, frame_g8_nograd, 0);
    }

    Point minp, maxp, centralp;
    minMaxLoc(frame_g8_nograd, NULL, NULL, &minp, &maxp); // doing it here, so we take rotation into account
    centralp=Point(frame_g8_nograd.cols/2.0, frame_g8_nograd.rows/2.0);
    minp*=scale;
    maxp*=scale;
    centralp*=scale;

    // Resize image: http://docs.opencv.org/3.2.0/da/d54/group__imgproc__transform.html#ga5bb5a1fea74ea38e1a5445ca803ff121
    // Note this is expensive computationally, only do if option set != 1
    if (scale != 1.0) {
        resize(frame_g8_nograd, frame_g8_nograd, Size(), scale, scale, INTER_LINEAR);
    }
    // add gradient
    Mat frame_g8(Size(frame_g8_nograd.cols+20, frame_g8_nograd.rows), CV_8U, Scalar(128));
    for (int r = 0; r < frame_g8.rows-1; r++)
    {
        frame_g8.row(r).setTo(255.0*(frame_g8.rows-r)/((float)frame_g8.rows));
    }
    frame_g8_nograd.copyTo(frame_g8(Rect(0,0,frame_g8_nograd.cols, frame_g8_nograd.rows)));

    // Apply colormap: http://docs.opencv.org/3.2.0/d3/d50/group__imgproc__colormap.html#ga9a805d8262bcbe273f16be9ea2055a65
    if (colormap != -1) {
        applyColorMap(frame_g8, outframe, colormap);
    } else {
        cv::cvtColor(frame_g8, outframe, cv::COLOR_GRAY2BGR);
    }

    // overlay marks
    draw_temp(outframe, mintemp, Point(outframe.cols-49, outframe.rows-29), Scalar(255, 255, 255));
    draw_temp(outframe, mintemp, Point(outframe.cols-51, outframe.rows-31), Scalar(0, 0, 0));
    draw_temp(outframe, mintemp, Point(outframe.cols-50, outframe.rows-30), Scalar(255, 0, 0));

    draw_temp(outframe, maxtemp, Point(outframe.cols-49, 0), Scalar(255, 255, 255));
    draw_temp(outframe, maxtemp, Point(outframe.cols-51, 2), Scalar(0, 0, 0));
    draw_temp(outframe, maxtemp, Point(outframe.cols-50, 1), Scalar(0, 0, 255));

    draw_temp(outframe, centraltemp, centralp+Point(-1, -1), Scalar(255, 255, 255));
    draw_temp(outframe, centraltemp, centralp+Point(1, 1), Scalar(0, 0, 0));
    draw_temp(outframe, centraltemp, centralp+Point(0, 0), Scalar(128, 128, 128));

    overlay_values(outframe, centralp+Point(-1,-1), Scalar(0,0,0));
    overlay_values(outframe, centralp+Point(1,1), Scalar(255,255,255));
    overlay_values(outframe, centralp, Scalar(128,128,128));

    overlay_values(outframe, minp+Point(-1,-1), Scalar(0,0,0));
    overlay_values(outframe, minp+Point(1,1), Scalar(255,255,255));
    overlay_values(outframe, minp, Scalar(255,0,0));

    overlay_values(outframe, maxp+Point(-1,-1), Scalar(0,0,0));
    overlay_values(outframe, maxp+Point(1,1), Scalar(255,255,255));
    overlay_values(outframe, maxp, Scalar(0,0,255));
}

void key_handler(char scancode) {
    switch (scancode) {
        case 'f': {
            int windowFlag = getWindowProperty(WINDOW_NAME, WindowPropertyFlags::WND_PROP_FULLSCREEN) == cv::WINDOW_FULLSCREEN ? WINDOW_NORMAL : WINDOW_FULLSCREEN;
            setWindowProperty(WINDOW_NAME, WindowPropertyFlags::WND_PROP_FULLSCREEN, windowFlag);
            break;
        }
        case 's': {
            waitKey(0);
            break;
        }
    }
}

#ifdef __linux__
// Function to setup output to a v4l2 device
int setup_v4l2(std::string output, int width,int height) {
    int v4l2 = open(output.c_str(), O_RDWR); 
    if(v4l2 < 0) {
        std::cout << "Error opening v4l2 device: " << strerror(errno) << std::endl;
        exit(1);
    }

    struct v4l2_format v;
    int t;
    v.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    t = ioctl(v4l2, VIDIOC_G_FMT, &v);
    if( t < 0 ) {
        std::cout << "VIDIOC_G_FMT error: " << strerror(errno) << std::endl;
        exit(t);
    }
    
    v.fmt.pix.width = width;
    v.fmt.pix.height = height;
    // BGR is not widely supported in v4l2 clients
    v.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
    v.fmt.pix.sizeimage = width * height * 3;
    t = ioctl(v4l2, VIDIOC_S_FMT, &v);
    if( t < 0 ) {
        std::cout << "VIDIOC_S_FMT error: " << strerror(errno) << std::endl;
        exit(t);
    }

    std::cout << "Opened v4l2 device" << std::endl;
    return v4l2;
}

void v4l2_out(int v4l2, Mat& outframe) {
    // Second colorspace conversion done here as applyColorMap only produces BGR. Do it in place.
    cvtColor(outframe, outframe, COLOR_BGR2RGB);
    int framesize = outframe.total() * outframe.elemSize();
    int written = write(v4l2, outframe.data, framesize);
    if (written < 0) {
        std::cout << "Error writing v4l2 device" << std::endl;
        close(v4l2);
        exit(1);
    }
}
#else
int setup_v4l2(std::string output, int width, int height) {
    std::cout << "v4l2 is not supported on this platform" << std::endl;
    exit(1);
    return -1; 
}
void v4l2_out(int v4l2, Mat& outframe) {}
#endif // __linux__


int main(int argc, char** argv) {
    // Setup arguments for parser
    args::ArgumentParser parser("Seek Thermal Viewer");
    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    args::ValueFlag<std::string> _mode(parser, "mode", "The mode to use - v4l2, window, file", {'m', "mode"});
    args::ValueFlag<std::string> _output(parser, "output", "Name of the file or video device to write to", {'o', "output"});
    args::ValueFlag<std::string> _ffc(parser, "FFC", "Additional Flat Field calibration - provide ffc file", {'F', "FFC"});
    args::ValueFlag<int> _fps(parser, "fps", "Video Output FPS - Kludge factor", {'f', "fps"});
    args::ValueFlag<float> _scale(parser, "scaling", "Output Scaling - multiple of original image", {'s', "scale"});
    args::ValueFlag<int> _colormap(parser, "colormap", "Color Map - number between 0 and 12", {'c', "colormap"});
    args::ValueFlag<int> _rotate(parser, "rotate", "Rotation - 0, 90, 180 or 270 (default) degrees", {'r', "rotate"});
    args::ValueFlag<std::string> _camtype(parser, "camtype", "Seek Thermal Camera Model - seek or seekpro", {'t', "camtype"});

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
    
    float scale = 1.0;
    if (_scale)
        scale = args::get(_scale);

    std::string mode = "window";
    if (_mode)
        mode = args::get(_mode);

    std::string camtype = "seek";
    if (_camtype)
        camtype = args::get(_camtype);

    // 7fps seems to be about what you get from a seek thermal compact
    // Note: fps doesn't influence how often frames are processed, just the VideoWriter interpolation
    int fps = 7;
    if (_fps)
        fps = args::get(_fps);

    // Colormap int corresponding to enum: http://docs.opencv.org/3.2.0/d3/d50/group__imgproc__colormap.html
    int colormap = -1;
    if (_colormap)
        colormap = args::get(_colormap);

    // Rotate default is landscape view to match camera logo/markings
    int rotate = 270;
    if (_rotate)
        rotate = args::get(_rotate);

    std::string output = "";
    if (_output)
        output = args::get(_output);

    if (output.empty()) {
        if (mode == "v4l2") {
            std::cout << "Please specify a video device to output to eg: /dev/video0" << std::endl;
            return 1;
        } else if (mode == "file") {
            std::cout << "Please specify a file to save the output to eg: seek.mp4" << std::endl;
            return 1;
        }
    }

    // Register signals
    signal(SIGINT, handle_sig);
    signal(SIGTERM, handle_sig);

    // Setup seek camera
    LibSeek::SeekCam* seek;
    LibSeek::SeekThermalPro seekpro(args::get(_ffc));
    LibSeek::SeekThermal seekclassic(args::get(_ffc));
    if (camtype == "seekpro") {
        seek = &seekpro;
    } else {
        seek = &seekclassic;
    }

    if (!seek->open()) {
        std::cout << "Error accessing camera" << std::endl;
        return 1;
    }

    // Mat containers for seek frames
    Mat seekframe, outframe;

    // Retrieve a single frame, resize to requested scaling value and then determine size of matrix
    //  so we can size the VideoWriter stream correctly
    if (!seek->read(seekframe)) {
        std::cout << "Failed to read initial frame from camera, exiting" << std::endl;
        return 1;
    }

    printf("WxH: %d %d\n", seekframe.cols, seekframe.rows);

    process_frame(seekframe, outframe, scale, colormap, rotate, seek->device_temp_sensor());

    // Setup video for linux if that output is chosen
    int v4l2 = -1;
    if (mode == "v4l2") {
        v4l2 = setup_v4l2(output, outframe.size().width,outframe.size().height);
    }

    // Create an output object, if mode specified then setup the pipeline unless mode is set to 'window'
    VideoWriter writer;
    if (mode == "file") {
#if CV_MAJOR_VERSION > 2
        writer.open(output, VideoWriter::fourcc('F', 'M', 'P', '4'), fps, Size(outframe.cols, outframe.rows));
#else  
        writer.open(output, CV_FOURCC('F', 'M', 'P', '4'), fps, Size(outframe.cols, outframe.rows));
#endif
        if (!writer.isOpened()) {
            std::cerr << "Error can't create video writer" << std::endl;
            return 1;
        }

        std::cout << "Video stream created, dimension: " << outframe.cols << "x" << outframe.rows << ", fps:" << fps << std::endl;
    } else if (mode == "window") {
        namedWindow(WINDOW_NAME, cv::WINDOW_NORMAL);
        setWindowProperty(WINDOW_NAME, WindowPropertyFlags::WND_PROP_ASPECT_RATIO, cv::WINDOW_KEEPRATIO);
        resizeWindow(WINDOW_NAME, seekframe.cols, seekframe.rows);
    }


    // Main loop to retrieve frames from camera and write them out
    while (!sigflag) {

        // If signal for interrupt/termination was received, break out of main loop and exit
        if (!seek->read(seekframe)) {
            std::cout << "Failed to read frame from camera, exiting" << std::endl;
            return 1;
        }

        // Retrieve frame from seek and process
        process_frame(seekframe, outframe, scale, colormap, rotate, seek->device_temp_sensor());

        if (mode == "v4l2") {
            v4l2_out(v4l2, outframe);
        } else if (mode == "window") {
            imshow(WINDOW_NAME, outframe);
            char c = waitKey(10);
            key_handler(c);

            // If the window is closed by the user all window properties will return -1 and we should terminate
            if (getWindowProperty(WINDOW_NAME, WindowPropertyFlags::WND_PROP_FULLSCREEN) == -1) {
                std::cout << "Window closed, exiting" << std::endl;
                return 0;
            }
        } else {
            writer << outframe;
        }

    }

    std::cout << "Break signal detected, exiting" << std::endl;
    return 0;
}
