.PHONY: all net clean examples

all:
	make -C ./ex/
	make -C ./net/

net:
	make -C ./net/

examples:
	make -C ./ex/

clean:
	make clean -C ./ex/
	make clean -C ./net/