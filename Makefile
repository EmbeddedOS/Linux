.PHONY: all net clean

net:
	make -C ./net/

all:
	make -C ./ex/
	make -C ./net/

clean:
	make clean -C ./ex/
	make clean -C ./net/