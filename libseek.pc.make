
Name: libseek-thermal
Description: Seek thermal imager library
Version: 0.1
Cflags: -std=c++11 -I${prefix}/include/seek
Libs: -L${prefix}/lib -lseek
Requires: opencv >= 2.4
Requires.private: libusb-1.0 >= 1.0
