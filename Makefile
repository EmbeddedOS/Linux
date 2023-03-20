net:
	make -C ./net/

all:
	make -C ./ex/

clean:
	make clean -C ./ex/
	make clean -C ./net/