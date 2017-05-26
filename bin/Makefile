LIBDIR =../lib
CXXFLAGS += -I$(LIBDIR)
LDLIBS += -lboost_program_options

all: seek_test_pro seek_test seek_viewer seek_create_flat_field

seek_test_pro: seek_test_pro.o $(LIBDIR)/$(LIBSEEK).a
seek_test: seek_test.o $(LIBDIR)/$(LIBSEEK).a
seek_viewer: seek_viewer.o $(LIBDIR)/$(LIBSEEK).a
seek_create_flat_field: seek_create_flat_field.o $(LIBDIR)/$(LIBSEEK).a

install:
	install -d $(PREFIX)/bin
	install seek_test_pro $(PREFIX)/bin/seek_test_pro
	install seek_test $(PREFIX)/bin/seek_test
	install seek_viewer $(PREFIX)/bin/seek_viewer
	install seek_create_flat_field $(PREFIX)/bin/seek_create_flat_field

clean:
	rm -rf seek_test_pro seek_test seek_viewer seek_create_flat_field *.o

.PHONY: install clean all

