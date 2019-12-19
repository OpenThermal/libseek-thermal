# libseek-thermal

[![Build Status](https://travis-ci.org/maartenvds/libseek-thermal.svg?branch=master)](https://travis-ci.org/maartenvds/libseek-thermal) master

[![Build Status](https://travis-ci.org/maartenvds/libseek-thermal.svg?branch=development)](https://travis-ci.org/maartenvds/libseek-thermal) development

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
* libboost-program-options-dev

NOTE: you can just 'apt-get install' all libs above

```
cd libseek-thermal
mkdir build
cd build
cmake ../
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

Some example command lines:

```
seek_viewer --camtype=seekpro --colormap=11 --rotate=0                     # view color mapped thermal video
seek_viewer --camtype=seekpro --colormap=11 --rotate=0 --output=seek.avi   # record color mapped thermal video
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
seek_create_flat_field -c seek seek_ffc.png

# When using the Seek Thermal compact pro
seek_create_flat_field -c seekpro seekpro_ffc.png
```
The program will run for a few seconds and produces a .png file.

3) Provide the produced .png file to one of the test programs:

```
# when using the Seek Thermal compact
seek_test seek_ffc.png
seek_viewer -t seek -F seek_ffc.png

# When using the Seek Thermal compact pro
seek_test_pro seekpro_ffc.png
seek_viewer -t seekpro -F seekpro_ffc.png
```
## Added functionality
### Color map selecting
Cycle through all 12 color maps by pressing 'c'. Black and white is not a standard "colormap" but it is preseved at the 0th position, for a total of 13 possible maps.

### Rotating
Pressing 'r' will change the orientation (4 possible)

### Brightness & contrast
By providing the '-n 1' command line switch, brightness and contrast are no longer adjusted dynamically. 

'w'  will increase the contast

's'  will decrease the contrast

'a'  will decrease the brightness

'd'  will increase the brightness

This function provides quite granular control, but this also means brightness has to be tweaked to match for each adjustment to contrast. 
It is useful for having the field of vision not suddendly change in appearance completely when a hot or cold object is viewed.

### Scaling
One can also adjust the size of the output (scaling value) by pressing + or -.
Note that in this case, "+" means Shift + "=" and "-" is just the normal "-" key. The Numpad "+" and "-" return different scan codes so don't work at this time.

### Window position
At least on Ubuntu (will check Debian later) the arrow keys allow moving the window position. Scancodes are different for each platform (supposedly for different renderers even), so this may not work on your environment.

## Feedback
I've started a thread about this updated version on EEVBlog, please comment for questions / issues: https://www.eevblog.com/forum/thermal-imaging/slightly-updated-libseek-thermal-(keyboard-interface)/
