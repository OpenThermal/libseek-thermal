# libseek-thermal

## Description

libseek-thermal is a user space driver for the SEEK thermal camera series built on libusb and libopencv.
The code is based on https://github.com/BjornVT/Masterproef.git

## Compilation

The library depends on libusb-1.0 and opencv (tested with opencv 3.2).
To compile just type 'make' in the root directory.

## Running code

Currently only the seek thermal pro is supported.

You need to add a udev rule to be able to run the program:

Udev rule:

```
SUBSYSTEM=="usb", ATTRS{idVendor}=="289D", ATTRS{idProduct}=="0011", MODE="0666", GROUP="users"
```

or manually chmod the device file after plugging the usb cable:

```
sudo chmod 666 /dev/bus/usb/00x/00x
```

with '00x' the usb bus found with the lsusb command
