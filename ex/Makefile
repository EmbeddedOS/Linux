obj-m += helloworld.o
obj-m += module_param.o
obj-m += char_device.o
obj-m += proc.o
PWD := $(CURDIR)

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
