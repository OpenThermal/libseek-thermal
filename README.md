# libseek-thermal

## Description

libseek-thermal is a user space driver for the SEEK thermal camera series built on libusb and libopencv.

Supported cameras:
* [Seek Thermal compact](http://www.thermal.com/products/compact/)
* [Seek Thermal compact pro](http://www.thermal.com/products/compactpro)

## Credits

The code is based on ideas from the following repo's:
* https://github.com/BjornVT/Masterproef.git
* https://github.com/zougloub/libseek

The added value of this library is the support for the compact pro.

## Compilation

The library depends on libusb-1.0 and opencv (tested with opencv 3.2).
To compile just type 'make' in the root directory.
To get debug data build with 'make DEBUG=1'.

## Running code samples

You need to add a udev rule to be able to run the program as non root user:

Udev rule:

```
SUBSYSTEM=="usb", ATTRS{idVendor}=="289D", ATTRS{idProduct}=="XXXX", MODE="0666", GROUP="users"
```

Replace 'XXXX' with:
* 0010: Seek Thermal Compact
* 0011: Seek Thermal Compact Pro

or manually chmod the device file after plugging the usb cable:

```
sudo chmod 666 /dev/bus/usb/00x/00x
```

with '00x' the usb bus found with the lsusb command

Run the examples
