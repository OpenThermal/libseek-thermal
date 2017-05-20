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

## Build

Dependencies:
* libopencv-dev (>= 2.4)
* libusb-1.0-0-dev
* libboost-program-options-dev

```
make
```

To get debug verbosity, build with

```
make DEBUG=1
```

Install shared library, headers and binaries to the default location

```
make install
ldconfig       # update linker runtime bindings
```

Install to a specific location

```
make install PREFIX=/my/install/prefix
```

## Getting USB access

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

## Running example binaries

```
./bin/test      # Thermal Compact
./bin/test_pro  # Thermal Compact Pro
```

Or if you installed the library you can run from any location:

```
seek_test      # Thermal Compact
seek_test_pro  # Thermal Compact Pro
```

To get better image quality, you can optionally apply an additional flat-field calibration.
This will cancel out the 'white glow' in the corners and additional flat-field noise.
The disadvantage is that this calibration is temperature sensitive and should only be applied
when the camera has warmed up.

Procedure for the Seek Thermal compact:

```
seek_create_flat_field -c seek seek_ffc.png
seek_test ffc.png
```

Procedure for the Seek Thermal compact Pro:

```
seek_create_flat_field -c seekpro seek_ffc.png
seek_test_pro ffc.png
```
