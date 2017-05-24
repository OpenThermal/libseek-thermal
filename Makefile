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

export CXXFLAGS LDFLAGS LIBSEEK LDLIBS CXX CC

all: bin

lib:
	make -C lib/

bin: lib
	make -C bin/

install: bin $(LIBSEEK).pc
	install -d $(PREFIX)/lib
	install -d $(PREFIX)/lib/pkgconfig
	install -d $(PREFIX)/include/seek
	install -d $(PREFIX)/bin
	install libseek.pc $(PREFIX)/lib/pkgconfig
	install lib/$(LIBSEEK).so $(PREFIX)/lib
	install lib/*.h $(PREFIX)/include/seek
	install bin/test $(PREFIX)/bin/seek_test
	install bin/test_pro $(PREFIX)/bin/seek_test_pro
	install bin/create_flat_field $(PREFIX)/bin/seek_create_flat_field

$(LIBSEEK).pc:
	echo 'prefix=$(PREFIX)' > $(LIBSEEK).pc
	cat $(LIBSEEK).pc.make >> $(LIBSEEK).pc

clean:
	make -C lib/ clean
	make -C bin/ clean
	rm -rf $(LIBSEEK).pc

.PHONY: all lib bin install clean
