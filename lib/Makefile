LIBSRC=$(wildcard *.cpp)
LIBOBJ=$(LIBSRC:.cpp=.o)
CXXFLAGS += -fPIC

all: $(LIBSEEK).so $(LIBSEEK).a

$(LIBSEEK).so: $(LIBOBJ)
	$(CXX) -shared -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(LIBSEEK).a: $(LIBOBJ)
	ar rcs $@ $(LIBOBJ)

clean:
	rm -rf $(LIBOBJ) $(LIBSEEK).so

.PHONY: clean
