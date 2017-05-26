CXX = g++
CC = $(CXX)
LDLIBS += $(shell pkg-config --libs opencv)
LDLIBS += $(shell pkg-config --libs libusb-1.0)
CXXFLAGS += $(shell pkg-config --cflags opencv)
CXXFLAGS += $(shell pkg-config --cflags libusb-1.0)
CXXFLAGS += -Wall -std=c++11
LDFLAGS += -lstdc++
LIBSEEK = libseek
PREFIX ?= /usr/local

ifeq ($(DEBUG), 1)
    CMNFLAGS = -fno-omit-frame-pointer -fsanitize=address
    CXXFLAGS += -DSEEK_DEBUG -g $(CMNFLAGS)
    LDFLAGS  += $(CMNFLAGS)
else
    CXXFLAGS += -O2
endif

export CXXFLAGS LDFLAGS LIBSEEK LDLIBS CXX CC PREFIX

all: bin

lib:
	make -C lib/

bin: lib
	make -C bin/

install: bin $(LIBSEEK).pc
	make -C bin/ install
	make -C lib/ install
	install -d $(PREFIX)/lib/pkgconfig
	install libseek.pc $(PREFIX)/lib/pkgconfig

$(LIBSEEK).pc:
	echo 'prefix=$(PREFIX)' > $(LIBSEEK).pc
	cat $(LIBSEEK).pc.make >> $(LIBSEEK).pc

clean:
	make -C lib/ clean
	make -C bin/ clean
	rm -rf $(LIBSEEK).pc

.PHONY: all lib bin install clean
