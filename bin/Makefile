include ../variables.mk

LIBDIR =../lib
CXXFLAGS += -I$(LIBDIR)

all: test_pro test

test_pro: test_pro.o $(LIBDIR)/$(LIBSEEK)
test: test.o $(LIBDIR)/$(LIBSEEK)

clean:
	rm -rf test_pro test *.o

.PHONY: clean all

