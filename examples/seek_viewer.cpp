// Seek Thermal Viewer/Streamer
// http://github.com/fnoop/maverick

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "seek.h"
#include "SeekCam.h"
#include <iostream>
#include <string>
#include <signal.h>
#include <memory>
#include "args.h"
#include <linux/videodev2.h>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

using namespace cv;
using namespace LibSeek;

// Setup sig handling
static volatile sig_atomic_t sigflag = 0;

void handle_sig(int sig) {
    (void)sig;
    sigflag = 1;
}

// Function to process a raw (corrected) seek frame
void process_frame(Mat &inframe, Mat &outframe, float scale, int colormap, int rotate, int cont, int bright) {
    Mat frame_g8, frame_g16; // Transient Mat containers for processing
   if(cont == 0){
    normalize(inframe, frame_g16, 0, 65535, NORM_MINMAX);
} else{
frame_g16=(inframe*cont)-bright;
}
    // Convert seek CV_16UC1 to CV_8UC1
    frame_g16.convertTo(frame_g8, CV_8UC1, 1.0/256.0 );

    // Rotate image
    if (rotate == 90) {
        transpose(frame_g8, frame_g8);
        flip(frame_g8, frame_g8, 1);
    } else if (rotate == 180) {
        flip(frame_g8, frame_g8, -1);
    } else if (rotate == 270) {
        transpose(frame_g8, frame_g8);
        flip(frame_g8, frame_g8, 0);
    }

    // Resize image: http://docs.opencv.org/3.2.0/da/d54/group__imgproc__transform.html#ga5bb5a1fea74ea38e1a5445ca803ff121
    // Note this is expensive computationally, only do if option set != 1
    if (scale != 1.0)
        resize(frame_g8, frame_g8, Size(), scale, scale, INTER_LINEAR);

    // Apply colormap: http://docs.opencv.org/3.2.0/d3/d50/group__imgproc__colormap.html#ga9a805d8262bcbe273f16be9ea2055a65
    if (colormap != 0) {
        applyColorMap(frame_g8, outframe, colormap-1);
    } else {
        cv::cvtColor(frame_g8, outframe, cv::COLOR_GRAY2BGR);
    }
}

// Function to setup output to a v4l2 device
int setupv4l2(std::string output, int width,int height){
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

int main(int argc, char** argv)
{
	    
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
    args::ValueFlag<int> _normalize(parser, "normalize", "0 for normal normalization, 1-50 (ish) for custom contrast", {'n',"normalize"});
    int colormap=0;
    int brightness=0;
    int normalize=0;
    int x=100;
    int y=100;	
    // Parse arguments
    try
    {
        parser.ParseCLI(argc, argv);
    }
    catch (args::Help)
    {
        std::cout << parser;
        return 0;
    }
    catch (args::ParseError e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }
    catch (args::ValidationError e)
    {
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
   // int colormap = -1;
    if (_colormap)
        colormap = args::get(_colormap);

    // Rotate default is landscape view to match camera logo/markings
    //Constant contrast or auto-adjust
    if(_normalize){
	    if(_ffc)  {
    brightness=620000;
    normalize=40;
    } else{
    brightness=590000;
    normalize=40;
    }
	}
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
    if (camtype == "seekpro")
        seek = &seekpro;
    else
        seek = &seekclassic;

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

    process_frame(seekframe, outframe, scale, colormap, rotate, normalize, brightness);

    // Setup video for linux if that output is chosen
    int v4l2 = -1;
    if (mode == "v4l2") {
        v4l2 = setupv4l2(output, outframe.size().width,outframe.size().height);
    }
    int frame_size = outframe.total() * outframe.elemSize();

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
    }

    // Main loop to retrieve frames from camera and write them out
    while (!sigflag) {

        // If signal for interrupt/termination was received, break out of main loop and exit
        if (!seek->read(seekframe)) {
            std::cout << "Failed to read frame from camera, exiting" << std::endl;
            return 1;
        }

        // Retrieve frame from seek and process
        process_frame(seekframe, outframe, scale, colormap, rotate, normalize, brightness);

        if (mode == "v4l2") {
            // Second colorspace conversion done here as applyColorMap only produces BGR. Do it in place.
            cvtColor(outframe, outframe, COLOR_BGR2RGB);
            int written = write(v4l2, outframe.data, frame_size);
            if (written < 0) {
                std::cout << "Error writing v4l2 device"  << std::endl;
                close(v4l2);
                return 1;
            }
        } else if (mode == "window") {
            imshow("SeekThermal", outframe);
		namedWindow("SeekThermal");
		moveWindow("SeekThermal", x,y);
            int c = waitKeyEx(5);
		//std::cout << (int)c << std::endl;
            if (c == 112) {
                waitKey(0);
            }
         else if  (c ==119 && _normalize){
		normalize++;
	} else if (c ==115 && _normalize){
		normalize--;
	} else if (c == 100 && _normalize){  // normally 100
		brightness-=5000;
	}else if (c == 97 && _normalize){    //normally 97
		brightness+=5000;
	}else if(c == 99){
		if(colormap<12){
			colormap++;
		}else{
		colormap=0;
	}
	}else if(c==114){
		if(rotate==270){
			rotate=0;
		}
		else{
			rotate+=90;
		}
	}else if(c==65579){
		scale +=0.05;
	}else if(c==45){
		scale -=0.05;
	}else if(c==65362){
		y-=10;
	}else if(c==65364){
		y+=10;
	}else if(c==65363){
		x+=10;
	}else if(c==65361){
		x-=10;
	}else {
            writer << outframe;
        }
	}
    }

    std::cout << "Break signal detected, exiting" << std::endl;
    return 0;
}
