CXX=g++
CC=g++
LDLIBS+=$(shell pkg-config --libs opencv) -lusb-1.0 -lboost_program_options -lboost_regex
CXXFLAGS+=-I/usr/include/opencv -I/usr/include/libusb-1.0
CXXFLAGS+=-Wall -std=c++11
LDFLAGS+=-lstdc++
LIBSEEK=libseek.a

ifeq ($(DEBUG), 1)
    CMNFLAGS = -fno-omit-frame-pointer -fsanitize=address
    CXXFLAGS += -DSEEK_DEBUG -g $(CMNFLAGS)
    LDFLAGS  += $(CMNFLAGS)
else
    CXXFLAGS += -O2
endif

export CXXFLAGS LDFLAGS LIBSEEK LDLIBS CXX CC

all: bin

lib:
	make -C lib/

bin: lib
	make -C bin/

clean:
	make -C lib/ clean
	make -C bin/ clean

.PHONY: all lib bin clean
