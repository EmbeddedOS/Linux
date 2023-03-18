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

- We can use `-p` flag to print out the environment variable values from the Makefile.

```bash
make -p | grep PWD
PWD = /home/ubuntu/temp
OLDPWD = /home/ubuntu
```

- You can find info on it with the command: `modinfo helloworld.ko`

- Insert module with the command: `sudo insmod helloworld.ko`

- Check module is inserted or not: `sudo lsmod | grep helloworld`. Example output:

```text
Module                  Size  Used by
helloworld             16384  0
```

- U should now see your loaded module. It can be removed again with: `sudo rmmod helloworld`

- To see what just happened in the logs:

```bash
sudo journalctl --since "1 hour ago" | grep kernel
```

- Output:

```text
Thg 3 18 16:21:16 larva kernel: helloworld: loading out-of-tree module taints kernel.
Thg 3 18 16:21:16 larva kernel: helloworld: module verification failed: signature and/or required key missing - tainting kernel
Thg 3 18 16:21:16 larva kernel: Hello world.
Thg 3 18 16:25:42 larva kernel: Goodbye world.
```

- Or easier to see the logs with `dmesg` command. Output:

```text
[18678.798108] helloworld: loading out-of-tree module taints kernel.
[18678.798159] helloworld: module verification failed: signature and/or required key missing - tainting kernel
[18678.799213] Hello world.
[18944.415643] Goodbye world.
```

- You now know the basics of *creating*, *compiling*, *installing* and *removing*
modules. Now for more of a description of how this module works.

...

- Kernel modules must have at least two functions:
  - a `start` (initialization) function called `init_module()` which is called when the module is **insmod**ed into the kernel,
  - and an `end` (cleanup) function called `cleanup_module()` which is called just before it is removed from the kernel.

- Actually, things have changed start with kernel `2.3.13`. U can now use whatever name u like for the `start` and `end` functions of a module.

- In fact, the new method is preferred method. However, many people still `init_module()` and `cleanup_module()` for their start and end functions.

- Typically, `init_module()` either **registers a handler** for something with the kernel, or it **replaces** one of the kernel functions with its own code (usually code to do something and then call the origin function). The `cleanup_module()` function is supposed to **undo** whatever `init_module()` did, so the module can be unloaded safely.

- Lastly, every kernel module needs to include `<linux/module.h>`. We needed to include `<linux/printk.h>` only for the macro expansion for the `pr_alert()` log level, which u will learn about in the next sections.
  - A point about coding style. Another thing which may not be immediately obvious to anyone getting started with kernel programming is that indentation within your code should be using **tabs** and **not spaces**. It is one of the `coding conventions` of the kernel.
  - Introducing print macros. In the beginning there was `printk`, ussually followed by a priority such as `KERN_INFO` or `KERN_DEBUG`. More recently this can also be expressed in abbreviated form using a set of print macros, such as `pr_info` and `pr_debug`.
    - They can be found within [include/linux/printk.h](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/include/linux/printk.h). Take time to read through the available priority macros.

  - **About compiling**. Kernel modules need to compiled a bit differently from regular user space apps.
    - Former kernel versions required us to care much about these settings, which are usually stored in Makefile. Although hierarchically organized, many redundant settings accumulated in sublevel Makefile and made them large and rather difficult to maintain.
    - Fortunately, there is a new way of doing these things, called `kbuild`, and the build process for external loadable modules is now fully integrated into the standard kernel build mechanism.
      - To learn more on how to compile modules which are not part of the official kernel, see [Documentation/kbuild/modules.rst](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/Documentation/kbuild/modules.rst).

    - Additional details about Makefile for kernel modules are available in [Documentation/kbuild/makefiles.rst](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/Documentation/kbuild/makefiles.rst). Be sure to read this and the related files before starting to hack Makefiles. It will probably save u lots of work.

### 4.2. Hello and Goodbye

- In early kernel versions u had to use the `init_module` and `cleanup_module` functions. But these days u can name those anything u want by using the `module_init` and `module_exit` macros. These macros are defined in [include/linux/module.h](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/include/linux/module.h).

- For example:

```C
/*
* helloworld.c - Demonstrating the module_init() and module_exit() macros.
* This is preferred over using init_module() and cleanup_module().
*/

#include <linux/init.h> /* Needed for the macros */
#include <linux/module.h> /* Needed by all modules */
#include <linux/printk.h> /* Needed for pr_info() */

static int __init helloworld_init(void)
{
    pr_info("Hello, world.\n");
    return 0;
}


static void __exit helloworld_exit(void)
{
    pr_info("Goodbye, world.\n");
}

module_init(helloworld_init);
module_exit(helloworld_exit);

MODULE_LICENSE("GPL");
```
