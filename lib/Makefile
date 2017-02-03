include ../variables.mk

LIBSRC=$(wildcard *.cpp)
LIBOBJ=$(LIBSRC:.cpp=.o)

$(LIBSEEK): $(LIBOBJ)
	ar rcs $@ $(LIBOBJ)

clean:
	rm -rf $(LIBOBJ) $(LIBSEEK)

.PHONY: clean
