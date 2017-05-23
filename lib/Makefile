LIBSRC= SeekCam.cpp SeekDeviceFile.cpp SeekThermalPro.cpp SeekThermal.cpp
LIBOBJ=$(LIBSRC:.cpp=.o)
CXXFLAGS += -fPIC

all: $(LIBSEEK).so $(LIBSEEK).a

$(LIBSEEK).so: $(LIBOBJ)
	$(CXX) -shared -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(LIBSEEK).a: $(LIBOBJ)
	ar rcs $@ $(LIBOBJ)

clean:
	rm -rf $(LIBOBJ) $(LIBSEEK).so $(LIBSEEK).a

.PHONY: clean
