# Linux Kernel Module Programming Guide

## 1. Introduction

### 1.3. What is A Kernel Module?

- So u want to write a kernel module. U know C, u have written a few normal programs to run as processes, and now you want to get to where the real action is, to where a single wild pointer can wipe out your file system and a core dump means a reboot.

- What exactly is a kernel module? Modules are pieces of code that can be loaded and unloaded into the kernel upon demand. They extend the function-ability of the kernel without the need to reboot system.

- For example, one type of module is the device driver, which allows the kernel to access hardware connected to the system.

- Without modules, we would have to build monolithic kernels and add new functionality directly into the kernel image. Besides having larger kernels, this has the disadvantage of requiring us to rebuild and reboot the kernel every time we want functionality.

### 1.4. Kernel module package

- Linux distribution provide the commands `modprobe`, `insmod` and `depmod` within a package.

- On Ubuntu/Debian:

```bash
sudo apt-get install build-essential kmod
```

### 1.5. What modules are in my Kernel?

- To discover what modules are already loaded within your current kernel use the command `lsmod`.

```bash
sudo lsmod
```

- Modules are stored within the file `/proc/modules`, so u can also see them with:

```bash
sudo cat /proc/modules
```

### 1.6. Do u need to download and compile the kernel?

- For the purposes of following this guide u don't necessarily to do that. However, it would be wise to run the examples within a test distribution running on a virtual machine in order to avoid any possibility of messing up your system.

### 1.7. Before we begin

- Before we delve into code, there are a few issues we need to cover. Everyone's system is different and everyone has their own groove.
  - **Modversioning**. A module compiled for one kernel will not load if u boot a different kernel unless u enable `CONFIG_MODVERSIONS` in the kernel. Most stock distribution kernels come with it turned on. If u are having trouble loading the modules because of versioning errors, compile a kernel with modversioning turned off.
  - **Using X Window System**. It is highly recommended that u extract, compile and load all the examples this guide discusses from a console. U should not be working on this stuff in X Window System.
    - Modules can not print to the screen like `printf()` can, but they can log information and warnings, which ends up being printed on your screen, but only on a console. If you `insmod` a module from an `xterm`, the information and warnings will be logged, but only to your `systemd journal`. You will not see it unless you look through your `journalctl`.

  - **SecureBoot**. Many contemporary computers are pre-configured with **UEFI SecureBoot enabled**. It is a security standard that can make sure the device boots using only software that is trusted by original equipment manufacturer. The default Linux Kernel from some distributions have also enabled the **SecureBoot**. For such distributions, the kernel module has to be signed with the security key or u would get the `ERROR: could not insert module` when u insert your module. And then u can check further with `dmesg` andd see the following text: `Lockdown: insmod: unsigned module loading is restricted; see man kernel lockdown.7`
    - If u got this message, the simplest way is to disable the UEFI SecureBoot from the PC/laptop boot menu to have your modules to be inserted.

    - Of course you can go through complicated steps to generate keys, install keys to your system, and finally sign your module to make it work. However, this is not suitable for beginners. You could read and follow the steps in [SecureBoot](https://wiki.debian.org/SecureBoot) if you are interested.

## 2. Headers

- Before u can build anything u will need to install the header files for the kernel. On Ubuntu/Debian:

```bash
sudo apt-get update
apt-cache search linux-headers-`uname -r`
```

- This will tell you what kernel header files are available. Then for example:

```bash
sudo apt-get install kmod linux-headers-5.19.0-35-generic
```

## 4. Simplest module

- We will start with a simplest programs that demonstrate different aspects of the basics of writing a kernel module.

```C
/*
* hello_world.c - The simplest kernel module.
*/
#include <linux/module.h> /* Needed by all modules */
#include <linux/printk.h> /* Needed for pr_info() */

int init_module(void)
{
    pr_info("Hello world.\n");

    /* A non 0 return means init_module failed; module can't be loaded. */
    return 0;
}

void cleanup_module(void)
{
    pr_info("Goodbye world.\n");
}

MODULE_LICENSE("GPL");
```

- Now u will need a `Makefile`.

```Makefile
obj-m += hello-world.o
PWD := $(CURDIR)
all:
    make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
    make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```

- In `Makefile`, `$(CURDIR)` can set to absolute pathname of the current working directory (after all `-C` options are processed, if any). See more about `CURDIR` in [GNU make manual](https://www.gnu.org/software/make/manual/make.html)

- And finally, just run `make` directly:

```bash
make
```
