LIBDIR =../lib
CXXFLAGS += -I$(LIBDIR)
LDLIBS += -lboost_program_options

all: test_pro test create_flat_field seek_viewer

test_pro: test_pro.o $(LIBDIR)/$(LIBSEEK).a
test: test.o $(LIBDIR)/$(LIBSEEK).a
create_flat_field: create_flat_field.o $(LIBDIR)/$(LIBSEEK).a
seek_viewer: seek_viewer.o $(LIBDIR)/$(LIBSEEK).a

clean:
	rm -rf test_pro test create_flat_field seek_viewer *.o

.PHONY: clean all

