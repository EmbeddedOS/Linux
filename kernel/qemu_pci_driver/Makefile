obj-m += c_pci_qemu_driver.o

PWD := $(CURDIR)

all:
	make -C ../linux-6.6.22 M=$(PWD) modules
clean:
	make -C ../linux-6.6.22 M=$(PWD) clean
