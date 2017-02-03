all: bin

lib:
	make -C lib/

bin: lib
	make -C bin/

clean:
	make -C lib/ clean
	make -C bin/ clean

.PHONY: all lib bin clean
