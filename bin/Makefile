LIBDIR =../lib
CXXFLAGS += -I$(LIBDIR)
LDLIBS += -lboost_program_options

all: test_pro test create_flat_field

test_pro: test_pro.o $(LIBDIR)/$(LIBSEEK)
test: test.o $(LIBDIR)/$(LIBSEEK)
create_flat_field: create_flat_field.o $(LIBDIR)/$(LIBSEEK)

clean:
	rm -rf test_pro test create_flat_field *.o

.PHONY: clean all

