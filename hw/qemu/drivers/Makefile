obj-m += custom_pci_drv.o

PWD := $(CURDIR)

all:
	make -C ../linux-6.6.22 M=$(PWD) modules
clean:
	make -C ../linux-6.6.22 M=$(PWD) clean
