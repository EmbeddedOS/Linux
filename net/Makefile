obj-m += net.o
net-objs += calls.o netlink.o

PWD := $(CURDIR)

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

usr:
	gcc usr.c -o usr.o -lpthread