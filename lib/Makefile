LIBSRC=$(wildcard *.cpp)
LIBOBJ=$(LIBSRC:.cpp=.o)
CXXFLAGS += -fPIC

all: $(LIBSEEK).so $(LIBSEEK).a

$(LIBSEEK).so: $(LIBOBJ)
	$(CXX) -shared -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(LIBSEEK).a: $(LIBOBJ)
	ar rcs $@ $(LIBOBJ)

install:
	install -d $(PREFIX)/lib
	install -d $(PREFIX)/include/seek
	install $(LIBSEEK).so $(PREFIX)/lib
	install *.h $(PREFIX)/include/seek

clean:
	rm -rf $(LIBOBJ) $(LIBSEEK).so $(LIBSEEK).a

.PHONY: all install clean
