CXX=g++
CC=g++
LDLIBS+=$(shell pkg-config --libs opencv) -lusb-1.0
CMNFLAGS=-fno-omit-frame-pointer -fsanitize=address
CXXFLAGS+=-I/usr/include/opencv -I/usr/include/libusb-1.0
CXXFLAGS+=-Wall -g -std=c++11 $(CMNFLAGS)
LDFLAGS+=-lstdc++ $(CMNFLAGS)
LIBSEEK=libseek.a
