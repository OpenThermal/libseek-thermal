# libseek-thermal

![CMake](https://github.com/OpenThermal/libseek-thermal/workflows/CMake/badge.svg?branch=master)

## Description

libseek-thermal is a user space driver for the SEEK thermal camera series built on libusb and libopencv.

Supported cameras:
* [Seek Thermal Compact](http://www.thermal.com/products/compact)
* [Seek Thermal CompactXR](http://www.thermal.com/products/compactxr)
* [Seek Thermal CompactPRO](http://www.thermal.com/products/compactpro)

Seek Thermal CompactPRO example:

![Alt text](/doc/colormap_hot.png?raw=true "Colormap seek thermal pro")


**NOTE: The library does not support absolute temperature readings since we don't know how. Any pull requests to fix this are welcome!**


## Credits

The code is based on ideas from the following repo's:
* https://github.com/BjornVT/Masterproef.git
* https://github.com/zougloub/libseek

## Build

Dependencies:
* cmake
* libopencv-dev (>= 2.4)
* libusb-1.0-0-dev

NOTE: you can just 'apt-get install' all libs above

```
cd libseek-thermal
mkdir build
cd build
cmake ..
make
```

Install shared library, headers and binaries:

```
sudo make install
sudo ldconfig       # update linker runtime bindings
```

For more build options (debug/release, install prefix, opencv install dir, address sanitizer, debug verbosity) run

```
cmake-gui ../
```

### Windows

This library and example programs can be built on Windows with multiple versions of Microsoft Visual Studio. This is most readily done with Visual Studio 2015 or newer, as dependancy binaries for Windows are available from the official projects, as described below.

libusb is required, and Windows binaries are available from the [offical libusb project](https://libusb.info/).
* Download the latest binary release (files ending in `.7z`) from [libusb GitHub Releases](https://github.com/libusb/libusb/releases)
* Extract the archive
* Set `LIBUSB_DIR` to the extracted directory (e.g., `C:\local\libusb-1.0.23`)

OpenCV is required, and Windows binaries are available from the [official OpenCV project](https://opencv.org/).
* Download one of the Windows releases from the [OpenCV releases page](https://opencv.org/releases/) (3.x and 4.x work)
* Run the self-extracting archive
* Set `OpenCV_DIR` to the build directory containing `OpenCVConfig.cmake` (e.g., `C:\local\opencv-3.4.10\build`)

Consider setting the `CMAKE_INSTALL_PREFIX` to a location in your build directory. Then after running the INSTALL target, copy the libusb and OpenCV libraries (e.g., `libusb-1.0.dll` and `opencv_world430.dll`) to the `bin\` directory containing `seek_test.exe`.

Before this library or example programs will work, you will need to set the driver for the USB device. The simplest way to do this is to use [Zadig](https://zadig.akeo.ie/). Run Zadig, then select `iAP Interface`, select `libusb-win32`, then click `Install Driver`.

## Getting USB access

You need to add a udev rule to be able to run the program as non root user:

Udev rule:

```
SUBSYSTEM=="usb", ATTRS{idVendor}=="289d", ATTRS{idProduct}=="XXXX", MODE="0666", GROUP="users"
```

Replace 'XXXX' with:
* 0010: Seek Thermal Compact/CompactXR
* 0011: Seek Thermal CompactPRO

or manually chmod the device file after plugging the usb cable:

```
sudo chmod 666 /dev/bus/usb/00x/00x
```

with '00x' the usb bus found with the lsusb command

## Running example binaries

```
./examples/seek_test       # Minimal Thermal Compact/CompactXR example
./examples/seek_test_pro   # Minimal Thermal CompactPRO example
./examples/seek_viewer     # Example with more features supporting all cameras, run with --help for command line options
```

Or if you installed the library you can run from any location:

```
seek_test
seek_test_pro
seek_viewer
```

### seek_viewer
seek_viewer is bare bones UI for the seek thermal devices. It can display video on screen, record it to a file, or stream it to a v4l2 loopback device for integration with image processing pipelines. It supports image rotation, scaling, and color mapping using any of the OpenCV color maps. While running `f` will set the display output full screen and `s` will freezeframe.

```
seek_viewer --camtype=seekpro --colormap=11 --rotate=0                          # view color mapped thermal video
seek_viewer --camtype=seekpro --colormap=11 --mode=file --output=seek.avi       # record color mapped thermal video
seek_viewer --camtype=seekpro --colormap=11 --mode=v4l2 --output=/dev/video0    # stream the thermal video to v4l2 device
```

## Linking the library to another program

After you installed the library you can compile your own programs/libs with:
```
g++ my_program.cpp -o my_program -lseek `pkg-config opencv --libs`
```

Using the following include:
```
#include <seek/seek.h>
```

## Apply additional flat field calibration

To get better image quality, you can optionally apply an additional flat-field calibration.
This will cancel out the 'white glow' in the corners and reduces spacial noise.
The disadvantage is that this calibration is temperature sensitive and should only be applied
when the camera has warmed up. Note that you might need to redo the procedure over time. Result of calibration on the Thermal Compact pro:

Without additional flat field calibration | With additional flat field calibration
------------------------------------------|---------------------------------------
![Alt text](/doc/not_ffc_calibrated.png?raw=true "Without additional flat field calibration") | ![Alt text](/doc/ffc_calibrated.png?raw=true "With additional flat field calibration")

Procedure:
1) Cover the lens of your camera with an object of uniform temperature
2) Run:
```
# when using the Seek Thermal compact
seek_create_flat_field -tseek

# When using the Seek Thermal compact pro
seek_create_flat_field -tseekpro
```
The program will run for a few seconds and produces a flat_field.png file.

3) Provide the produced .png file to one of the test programs:

```
# when using the Seek Thermal compact
seek_test flat_field.png
seek_viewer -t seek -F flat_field.png

# When using the Seek Thermal compact pro
seek_test_pro flat_field.png
seek_viewer -t seekpro -F flat_field.png
```
