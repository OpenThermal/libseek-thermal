include ../variables.mk

SRC = $(wildcard *.cpp)
OBJ = $(LIBSRC:.cpp=.o)
LIBDIR =../lib
CXXFLAGS += -I$(LIBDIR)

test: $(OBJ) $(LIBDIR)/$(LIBSEEK)

clean:
	rm -rf $(OBJ) test

.PHONY: clean

