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

### 4.3. The `__init` and `__exit` Macros

- The `__init` macro causes the init function be discarded and its memory freed once the init function finishes for built-in drivers, but not loadable modules. If u think about when the init function is invoked, this makes perfect sense.

- There is also an `__initdata` which works similarly to `__init` but for init variables rather than functions.

- The `__exit` macro causes the omission of the function when the module is built into the kernel, and like `__init`, has no effect for loadable modules. Again, if u consider when the cleanup function runs, this makes complete sense; built-in drivers do not need a cleanup function, while loadable modules do.

- These macros are defined in [include/linux/init.h](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/include/linux/init.h) ans serve to free up kernel memory. When u boot your kernel and see something like `Freeing unused kernel memory: 236k freed`, this is precisely what the kernel is freeing.

### 4.4. Licensing and Module Documentation

- Honestly, who loads or even cares about proprietary modules? If u do then u might have seen something like this: `sudo insmod xxxxxx.ko`

```text
loading out-of-tree module taints kernel.
module license 'unspecified' taints kernel.
```

- U can use a few macros to indicate the license for your module. Some examples are `GPL`, `GPL v2`, `Dual MIT/GPL`. They are defined within [include/linux/module.h](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/include/linux/module.h).
- To reference what license you’re using a macro is available called `MODULE_LICENSE`.

### 4.5. Passing Command Line Arguments to a Module

- Modules can take command line arguments, but not with the `argc/argv` u might be used to.

- To allow arguments to be passed to your module, declare the variable that will take the values of the command line arguments as global and then use the `module_param()` macro, (defined in [include/linux/moduleparam.h](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/include/linux/moduleparam.h)) to set the mechanism up.

- At runtime, `insmod` will fill the variables with any command line arguments that are given, like `insmod mymodule.ko myvariable=5`.

- The variable declarations and macros should be placed at the beginning of the module for clarity.

- The `module_param()` macro takes **3** arguments:
  - The name of the variable.
  - Its type.
  - Permissions for the corresponding file in **sysfs**.

- Integer types can be signed as usual or unsigned. If u would like to use arrays of integers or strings see `module_param_array()` and `module_param_string()`.

```C
int myint = 3;
module_param(myint, int, 0);
```

- Arrays are supported too, but things are a bit different now than they were in the olden days. To keep track of the number of parameters u need to pass a pointer to a count variable as third parameter. At your option, u could also ignore the count and pass NULL instead. We show both possibilities here:

```C
int myintarray[2];
module_param_array(myintarray, int, NULL, 0); /* not interested in count */

short myshortarray[4];
int count;
module_param_array(myshortarray, short, &count, 0); /* put count into "count",→ variable */
```

- A good use for this is to have the module variable's default value set, like a port or IO address. If the variables contain the default value, then perform auto detection. Otherwise, keep the current value. This will be made clear later on.

- Lastly, there is a macro function, `MODULE_PARM_DESC()`, that is used to document arguments that the module can take. It takes two parameters:
  - a variable name and
  - a free form string describing that variable.

```bash
sudo rmmod examples/module_param.ko
sudo insmod examples/module_param.ko my_int_array={4,5,6,7} my_str="HelloLarva"
```

### 3.6. Modules Spanning Multiple Files

- Sometimes it make sense to divide a kernel module between several source files.

- Here is an example of such a kernel module.

```C
/*
 * start.c - Illustration of multi filed modules
 */
#include <linux/kernel.h> /* We are doing kernel work */
#include <linux/module.h> /* Specifically, a module */

int init_module(void)
{
    pr_info("Hello, world - this is the kernel speaking\n");
    return 0;
}

 MODULE_LICENSE("GPL");
```

- The next file:

```C
/*
 * stop.c - Illustration of multi filed modules
 */

#include <linux/kernel.h> /* We are doing kernel work */
#include <linux/module.h> /* Specifically, a module */

void cleanup_module(void)
{
    pr_info("Short is the life of a kernel module\n");
}

 MODULE_LICENSE("GPL");
```

- And finally, the makefile:

```Makefile
obj-m += startstop.o
startstop-objs := start.o stop.o

PWD := $(CURDIR)

all:
    make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
    make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```

## 5. Preliminaries

### 5.1. How module begin and end

- A program usually begins with a `main()` function, executes a bunch of instructions and terminates upon completion of these instructions. Kernel modules work a bit different. A module always begin with either the `init_module` or the function u specify with `module_init` call. This is the entry function for modules; it tells the kernel what functionality the module provides and sets up the kernel to run the module's functions when they are needed. Once it does this, entry function returns and the modules does nothing until the kernel wants to something with the code that the module provides.

- All modules end by calling either `cleanup_module` or the function u specify with the `module_exit` call. This is the exit function for modules; it undoes whatever entry function did. It un-registers the functionality that the entry function registered.

- Every module must have an entry function and an exit function. Since there is more than one way to specify entry and exit functions, I will try my best to use the terms `entry function` and `exit function`, but if I slip and simply refer to them as `init_module` and `cleanup_module`, I think u will know what I mean.

### 5.2. Functions available to modules

- Programmers use functions they do not define all the time. A prime example of this is `printf()`. U use these library functions which are provided by the standard C library. The definitions for these functions do not actually enter your program until linking stage, which insures that the code (for printf() for example) is available, and fixes the call instruction to point to the code.

- Kernel modules are different here, too. In the hello world example, u might have noticed that we used a function, `pr_info()` but did not include a standard I/O library. This is because modules are object files whose symbols get resolved upon running `insmod` or `modprobe`. The definition for the symbols comes from the kernel itself; the only external functions u can uses are the ones provided by the kernel . If U are curious about what symbols have been exported by your kernel, take a look at `/proc/kallsyms`.

- One point to keep in mind is the difference between library functions and system calls. Library functions are higher level, run completely in user space and provide a more convenient interface for the programmer to the functions that do the real work -- system calls. **System calls** run in kernel mode on the user's behalf and are provided by the kernel itself. The library function `printf()` may look like a very general printing function, but all it really does is format the data into strings and write the string data using the low level system call `write()`, which then sends the data to standard output.

- Would u like to see what system calls are made by `printf()`? It is easy! Compile the following program:

```C
#include <stdio.h>

int main()
{
  printf("hello");
  return 0;
}
```

- With `gcc -Wall -o hello hello.c`. Run the executable with `strace ./hello`. Are u impressed? Every line u see corresponds to a system call.
  - [strace](https://strace.io/) is a handy program that gives u details about what system calls a program is making, including which call is made, what its arguments are and what it returns.
  - It is an invaluable tool for figuring out things like what files a program is trying to access. Towards the end, u will see a line which looks like `write(1, "hello", 5hello)`. There it is.
  - The face behind the `printf()` mask. U may not be familiar with `write`, since most people use library functions for the file I/O (like `fopen()`, `fputs()`, `fclose()`).

  - U can even write modules to **replace** the kernel's system calls, which we will do shortly. Crackers often make use of this sort of thing for back-doors or trojans, but u can write your own modules to do more benign things, like have the kernel write Tee hee; that tickles! every time someone tries to delete a file on your system.

### 5.3. User space and kernel space

- A kernel is all about access to resources, whether the resource in question happens to be a video card, a hard drive or even memory.

- Programs often compete for the same resource. As I just saved this document, `updatedb` started updating the locate database. My vim session and `updatedb` are both using the hard drive concurrently. The kernel needs to keep things orderly, and not give users access to resources whenever they feel like it. To this end, a CPU can run in different modes. Each mode gives a different level of freedom to do what u want on the system.

- The Intel 80386 architecture had 4 of these modes, which were called **rings**. Unix uses only two ring:
  - The **highest ring**: ring 0, also known as `supervisor mode` where everything is allowed to happen.
  - The **lowest ring**: which is called `user mode`.

- Recall the discussion about library functions vs system calls. Typically, u use a library function in user mode. The library function calls one or more system calls, and these system calls execute on the library function's behalf, but do so in supervisor mode since they are part of the kernel itself. Once the system call completes its task, it returns and execution gets transferred back to user mode.

### 5.4. Name space

- When u write a small C program, u use variables which are convenient and make sense to the reader. If, on the other hand, u are writing routines which will be part of a bigger problem, any global variables u have are part of a community of other people's global variables, some of the variable manes can clash. When a program has lots of global variables which aren't meaningful enough to be distinguished, u get namespace pollution. In large projects, effort must be made to remember reserved names, and to find ways to develop a scheme for naming unique variable names and symbols.

- When writing kernel code, even the smallest module will be linked against the entire kernel, so this is definitely an issue. The best way to deal with this is to declare everything as static and to use a well-defined prefix for your symbols. By convention, *all kernel prefixes are lowercase*. If u do not want to declare everything as static, another option is to declare a symbol table and register it with the kernel.

- The file `/proc/kallsyms` holds all the symbols that the kernel knows about and which are therefore accessible to your modules since they share the kernel's code space.

### 5.5. Code space

- Memory management is a very complicated subject.
- If u have not thought about what a segfault really means, u may be surprised to hear that `pointers do not actually point to memory locations`. Not real ones, anyway.
  - When a process is created, the kernel sets aside a portion of real physical memory and hands it to process things to use for its executing code, variables, stack, heap, and other things which a computer scientist would know about.

  - This memory begins with `0x00000000` and externs up to whatever it needs to be.

  - Since the memory space for any two processes **do not overlaps**, every process that can access a memory address, say `0xbffff978`, would be accessing a different location in real physical memory! The processes would be accessing an index named `0xbffff978` which points to some kind of offset into the region of memory set aside for that particular process. For the most part, a process like our `Hello, world` program can't access the space of another process, although there are ways which we will talk about later.

  - The kernel has its own space of memory as well. Since a module is code which can be dynamically inserted and removed in the kernel (as opposed to a semi-autonomous object), it shares the kernel's code-space rather having its own. Therefore, `if your module segfaults, the kernel segfaults`. And if u start writing over data because of an off-by-one error, then you are **trampling** on kernel data (or code). This is even worse than it sounds, so try your best to careful.

### 5.6. Device Drivers

- One class of module is the device driver, which provides functionality for hardware like a serial port.
  - On Unix, each piece of HW is represented by a file located in `/dev/` named a device file which provides the communicate with the HW.
  - The device driver provides the communication on behalf of a user program.
    - So the `es1370.ko` sound card device driver might connect the `/dev/sound` device file to the `Ensoniq IS1370` sound card.
    - A user program like `mp3blaster` can use `/dev/sound` without ever knowing what kind of sound card is installed.
  - For example: `ls -l /dev/tty[0-15]`

  ```text
  crw--w---- 1 root tty 4, 0 Thg 3  20 07:56 /dev/tty0
  crw--w---- 1 gdm  tty 4, 1 Thg 3  20 07:56 /dev/tty1
  crw--w---- 1 root tty 4, 5 Thg 3  20 07:56 /dev/tty5
  ```

  - Notice the column of numbers separated by a comma. The first number is called the device’s `major number`. The second number is the `minor number`.
    - The major number tells u which driver is used to access the HW.
    - Each driver is assigned a unique major number; all device files with the same major number are controller by the same driver.
    - On the above example, all the major numbers are `4`, because they are all controlled by the same driver.

  - The `minor` number is used by the driver to distinguish between the various it controls. Returning to the example above, although all three devices are handled by the same driver they have unique minor numbers because the driver sees them as being different pieces of HW.

- Device are divided into two types:
  - character devices, and
  - block devices.

- The difference is that block devices have a buffer for requests, so they can choose the best order in which to respond to the requests.
  - This is **important** in the case of storage devices, where it is faster to read or write sectors which are close to each other, rather than those which are further apart.
  - Another difference is that block devices can only accept input and return output in **blocks** (whose size can vary according to the device), whereas character devices are allowed to use as many or as they like.

- Most devices in the world are character, because they don’t need this type of buffering, and they don’t operate with a fixed block size.

- You can tell whether a device file is for a block device or a character device by looking at the first character in the output of `ls -l`. If it is `b` then it is a block device, and if it is `c` then it is a character device.

```text
# Block device.
brw-rw----  1 root  disk      8,   0 Thg 3  20 07:56 sda
brw-rw----  1 root  disk      8,   1 Thg 3  20 07:56 sda1

# Character device.
crw--w---- 1 root tty 4, 0 Thg 3  20 07:56 /dev/tty0
crw--w---- 1 gdm  tty 4, 1 Thg 3  20 07:56 /dev/tty1
```

- When the system was installed, all of those device files were created by the `mknod` command. To create a new char device named `coffee` with major/minor
number `12` and `2`, simply do `mknod /dev/coffee c 12 2`. You do not have
to put your device files into `/dev`, but it is done by convention.

- When a device file is accessed, the kernel uses the `major number` of the file to determine which driver should be used to handle the access. This means that kernel doesn't really should be used to handle the access.
  - The driver itself is the only thing that cares about the `minor number`. It uses the minor number to distinguish between different pieces of hardware.

## 6. Character Device Drivers

### 6.1. The `file_operations` Structure

- The `file_operations` structure is defined in [include/linux/fs.h](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/include/linux/fs.h), and holds pointers to functions defined by the driver that perform various operations on the device.

- *Each field of the structure corresponds to the address of some function defined by the driver to handle a request operation.*

- For example, every character driver needs to define a function that reads from the device. The `file_operations` structure holds the address of the module's function that performs that operation.

```C
struct file_operations {
  struct module *owner;
  loff_t (*llseek) (struct file *, loff_t, int);
  ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
  ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
  ssize_t (*read_iter) (struct kiocb *, struct iov_iter *);
  ssize_t (*write_iter) (struct kiocb *, struct iov_iter *);
  int (*iopoll)(struct kiocb *kiocb, bool spin);
  int (*iterate) (struct file *, struct dir_context *);
  int (*iterate_shared) (struct file *, struct dir_context *);
  __poll_t (*poll) (struct file *, struct poll_table_struct *);
  long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
  long (*compat_ioctl) (struct file *, unsigned int, unsigned long);
  int (*mmap) (struct file *, struct vm_area_struct *);
  unsigned long mmap_supported_flags;
  int (*open) (struct inode *, struct file *);
  int (*flush) (struct file *, fl_owner_t id);
  int (*release) (struct inode *, struct file *);
  int (*fsync) (struct file *, loff_t, loff_t, int datasync);
  int (*fasync) (int, struct file *, int);
  int (*lock) (struct file *, int, struct file_lock *);
  ssize_t (*sendpage) (struct file *, struct page *, int, size_t, loff_t *, int);
  unsigned long (*get_unmapped_area)(struct file *, unsigned long, unsigned long, unsigned long, unsigned long);
  int (*check_flags)(int);
  int (*flock) (struct file *, int, struct file_lock *);
  ssize_t (*splice_write)(struct pipe_inode_info *, struct file *, loff_t *, size_t, unsigned int);
  ssize_t (*splice_read)(struct file *, loff_t *, struct pipe_inode_info *, size_t, unsigned int);
  int (*setlease)(struct file *, long, struct file_lock **, void **);
  long (*fallocate)(struct file *file, int mode, loff_t offset, loff_t len);
  void (*show_fdinfo)(struct seq_file *m, struct file *f);
  ssize_t (*copy_file_range)(struct file *, loff_t, struct file *, loff_t, size_t, unsigned int);
  loff_t (*remap_file_range)(struct file *file_in, loff_t pos_in, struct file *file_out, loff_t pos_out, loff_t len, unsigned int remap_flags);
  int (*fadvise)(struct file *, loff_t, loff_t, int);
} __randomize_layout;
```

- Some operations are not implemented by a driver. For example, a driver that handles a video card will not need to read from a directory structure.
  - The corresponding entries in the `file_operations` structure should be set to `NULL`.

- Assigning handler to the structure:

```C
struct file_operations fops = {
  .read = device_read,
  .write = device_write,
  .open = device_open,
  .release = device_release
};
```

### 6.2. The file structure

- Each device is represented in the kernel by a file structure, which is defined in [include/linux/fs](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/include/linux/fs.h). Be aware that a file is a kernel level structure and never appears in a user space program.
  - It is not the same thing as a `FILE`, which is defined by glibc and would never appear in a kernel space function.
  - Also, its name is a bit misleading, it represents an abstract open **file**, not a file on a disk, which is represented by a structure named `inode`.

- An instance of structure file is commonly named `filp`. U will also see it referred to as a struct file object. Resist the temptation.

### 6.3. Registering A Device

- Char devices are accessed through device files, usually located in `/dev`. This is by convention. When writing a driver, it is OK, to put the device file in your current directory. Just make sure you place it in `/dev` for a production driver.
  - The major number tells u which driver handles which device file.
  - The minor number is used only by the driver itself to differentiate which device it is operation on, just in case the driver handles more than one device.

- Adding a driver to your system means registering it with the kernel. This is synonymous with assigning it a major number during the module's initialization. U do this by using the `register_chrdev` function, defined by [include/linux/fs](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/include/linux/fs.h).

```C
int register_chrdev(unsigned int major, const char *name, struct file_operations *fops);
```

- Where `unsigned int major` is the major number u want to request,
  - `const char *name` is the name of the device as it will appear in `/proc/devices` and
  - `struct file_operations *fops` is a pointer to the `file_operations` table for your driver.

- A negative return value means the registration failed.
- Note: we didn't pass the minor number to `register_chrdev` because the kernel doesn't care about the minor number; only our driver uses it.

- Now the question is how do u get a major number without hijacking one that is already in use?
  - The easiest way would be to look through [Documentation/admin-guide/devices.txt](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/Documentation/admin-guide/devices.txt) and pick an unused one. That is a bad way of doing things because u will NEVER be sure if the number u picked will be assigned later.
  - The answer is that u can ask the kernel to assign u a *dynamic major number*.
    - if u pass a major of `0` to `register_chrdev`, the return value will be the dynamically allocated major number.
    - The downside:
      - U can not make a device file in advance, since u do not know what the major number will be.
    - There are a couple of way to do this.
      - First, the driver itself can print the newly assigned number and we can make the device file by hand.
      - Second, the newly registered device will have an entry in `/proc/devices`, and we can either make the device file by hand or write a shell script to rad the file in and make the device file.
      - The third method is that we can have our driver make the device file using the `device_create` function after a successful registration and `device_destroy` during the call to `cleanup_module()`.

  - However, `register_chrdev()` would occupy a range of minor numbers associated with the given major. The recommended way to reduce waste for char device registration is using `cdev` interface.

- The newer interface completes the char device registration in two distinct steps.
  - First, we should register a range of device numbers, which can completed with `register_chrdev_region` or `alloc_chrdev_region`.

    ```C
    int register_chrdev_region(dev_t from, unsigned count, const char *name);
    int alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count, const char *name);
    ```

    - The choice between two different functions depends on whether u know the major numbers for your device. Using `register_chrdev_region` if u know the device major number and `alloc_chrdev_region` if u would like to allocate a dynamicly-allocated major number.

  - Second, we should initlialize the data structure `struct cdev` for our char device and associate it with the device numbers. To initilialize the `struct cdev`, we can achieve by the simalar sequence of the following codes.

    ```C
    struct cdev *my_dev = cdev_alloc();
    my_cdev->ops = &my_fops;
    ```

    - However the common usage pattern will embed the struct cdev within a device-specific structure of your own. In this case, we will need `cdev_init` for the initlization.

      ```C
      void cdev_init(struct cdev *cdev, const struct file_operations *fops)
      ```

- Once we finish the initialization, we can add the char device to the system by using the `cdve_add`.

```C
int cdev_add(struct cdev *p, dev_t dev, unsigned count);
```

### 6.4. Unregistering A device

- We can not know allow the kernel module to `rmmod`ed whenever root feels like it.

- If the device file is opened by a process and then we remove the kernel module, using the file would cause a call to the memory location where the appropriate function (read/write) used to be.
  - If we are lucky, no other code was loaded there and we will get an ugly error message.
  - If we are unlucky, another kernel module was loaded into the same location, which means a jump into the middle of another function within the kernel. The result of this would be impossible to predict, but they can not be very positive.

- Normally, when u do not want to allow something, u return an error code (a negative number) from function which is supposed to do it. With `cleanup_module` that is *impossible* because it is **void** function.

- However, there is a **counter** which keeps track of how many processes are using your module.
  - You can see what its value is by looking at the 3rd field with the command `cat /proc/modules` or `sudo lsmod`. If this number isn't zero, `rmmod` will **fail**.
  - Note that u do not have to check the counter within `cleanup_module` because the check will be performed for u by the system call `sys_delete_module`, defined in [include/linux/syscalls.h](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/include/linux/syscalls.h).
  - U **should not** use this counter directly, but there are functions defined in [include/linux/module.h](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/include/linux/module.h) which let u increase, decrease and display this counter:
    - `try_module_get(THIS_MODULE)`: Increment the reference count of current module.
    - `module_put(THIS_MODULE)`: Decrement the reference count of current module.
    - `module_refcount(THIS_MODULE)`: Return the value of reference count of current module.

- It is importanc to keep the counter accurate; if u ever do lose track of the correct usage count, u will **never be able to** unload the module; it's now reboot time.

### 6.5. `char_device.c`

- In multiple-threaded environment, without any protection, concurrent access to the same memory may lead to the *race condition*, and will not preserve the performance.
  - In the kernel module, this problem may happen due to multiple instances accessing the shared resources. Therefore, a solution is to enforce the exclusive access. We use atomic **Compare-And-Swap (CAS)** to maintain the states, `DEV_NOT_USED` and `DEV_IS_OPEN`, to determine whether the file is currently opended by someone or not.
  - `CAS` compares the contents of a memory location with the expected value and, only if they are the same, modifies the contents of that memory location to the desired value.

## 7.  The `/proc` File System

- In Linux, there is an additional mechanism for the kernel and kernel modules to send information to processes -- the `/proc` file system. Originally designed to allow easy access to information about processes (hence the name), it is now used by every bit of the kernel which has something interesting to report, such as `/proc/modules` which provides the list of modules and `/proc/meminfo` which gathers memory usage statistics.

- The method to use proc file system is very similar to the one used with device drivers -- **a structure** is created with all the information needed for the `/proc` file, including pointers to *any handler functions*.

- Then `init_module` registers the structure with the kernel and `cleanup_module` unregisters it.

- Normal file systems are located on a disk, rather than just in memory (which is where /proc is), and in that case the index-node (**inode** for short) number is a pointer to a disk location where the file's inode is located.
  - The inode contains information about the file, for example the file's permissions, together with a pointer to the disk location or locations where the file's data can be found.

### 7.1. The `proc_ops` structure

- The `proc_ops` structure is defined in [include/linux/proc_fs.h](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/include/linux/proc_fs.h) in Linux v5.6+. In order kernels, it used `file_operations` for custom hook in `/proc` fs.
  - but it contains some members that are unnecessary in VFS, and every time `VFS` expands `file_operations` set, `/proc` code comes bloated. On the other hand, not only the space, but also some operations were saved by this structure to improve its performance.

```C
struct proc_ops {
  unsigned int proc_flags;
    int (*proc_open)(struct inode *, struct file *);
    ssize_t (*proc_read)(struct file *, char __user *, size_t, loff_t *);
    ssize_t (*proc_read_iter)(struct kiocb *, struct iov_iter *);
    ssize_t (*proc_write)(struct file *, const char __user *, size_t, loff_t *);
  /* mandatory unless nonseekable_open() or equivalent is used */
  loff_t (*proc_lseek)(struct file *, loff_t, int);
  int (*proc_release)(struct inode *, struct file *);
  __poll_t (*proc_poll)(struct file *, struct poll_table_struct *);
  long (*proc_ioctl)(struct file *, unsigned int, unsigned long);
#ifdef CONFIG_COMPAT
  long (*proc_compat_ioctl)(struct file *, unsigned int, unsigned long);
#endif
  int (*proc_mmap)(struct file *, struct vm_area_struct *);
  unsigned long (*proc_get_unmapped_area)(struct file *, unsigned long, unsigned long, unsigned long, unsigned long);
} __randomize_layout;
```

### 7.2. Read and Write a `/proc` file

- The reason for copy_from_user or get_user is that Linux memory (On Intel architecture, it may be different under some other processors) is segmented. This means that a pointer, by itself, does not reference a unique location in memory, ony a location in memory segment it is to be able to use it. *There is one memory segment for the kernel and one for each of the processes.*

- The only memory segment accessible to a process is its own, so when writing regular programs to run as processes, there is no need to worry segments. When u write a kernel module, normally u want to access the kernel memory segment, which is handled automatically by the system. However, when the content of a memory buffer needs to be passed between the currently running process and the kernel, the kernel function receives a pointer to the memory buffer which is in the process segment.
  - The `put_user` and `get_user` macros allow u to access that memory. These functions handle only one character, u can handle several characters with `copy_to_user` and `copy_from_user`.

### 7.3. Manage `/proc` file with standard filesystem

- A `/proc` file can read and write with `/proc` interface. But it is also possible to manage `/proc` with `inodes`. The main concern is to use advanced functions, like permissions.

- In Linux, there is a standard mechaism for file system registration. Since every file has to have its own functions to handle inode and file operations, there is a special structure to hold pointers to all those functions, **struct inode_operations**, which include a pointer to struct proc_ops.

- The difference between file and inode operations is that file operations deal with the file itself whereas inode operations deal with ways of referencing the file, such as creating links to it.

- In `/proc`, whenever we register a new file, we are allowed to specify which `struct inode_operations` which includes a pointer to a `struct proc_ops` which includes pointers to our `_read()` and `_write()` function.

- Another interesting point is the `module_permission` function. This function is called whenever a process tries to do something with `/proc` file, and it can decide whether to allow access or not.
  - Right now it is based on the operation and the `uid` of the current user, but it could be based on anything we like, such as what other processes are doing with the same file, the time of day, or the last inupt we received.

- It is importance to note that the *standard roles of read and write are reversed* in the kernel.
  - Read functions are used for output, whereas write functions are used for input. The reason for that is that read and write refer to the user’s point of view — if a process reads something from the kernel, then the kernel needs to output it, and if a process writes something to the kernel, then the kernel receives it as input.

### 7.4. Manage `/proc` file with `seq_file`

- Writing a `/proc` file may be quite `complex`. So to help people writing `/proc` file, there is an API named `seq_file` that helps formating a `/proc` file for output. It is based on sequence, which is composed of 3 functions: `start()`, `next()`, `stop()`. The `seq_file` API starts a sequence when a user read the `/proc` file.

- A sequence begins with the call of the function start(). If the return is a non `NULL` value, the function `next()` is called. This function is an iterator, the goal is to go through all the data.

- Each time `next()` is called, the function `show()` is also called. It writes data value in the buffer read by the user. The function `next()` is called until it returns `NULL`.

- The sequence ends when `next()` return `NULL`, then the function `stop()` is called.

- BE CAREFUL: when a sequence is finished, another one starts. That means that at the end of fuction `stop()`, the function `start()` is called again. This loop **ONLY FINISHES** when the function `start()` return `NULL`.

- The `seq_file` provides basic functions for `proc_ops`, such as `seq_read()`, `seq_lseek()`, and some others. But nothing to write in the `/proc` file.

## 8. sysfs: Interacting with your module

- `sysfs` allows u to interact with the running kernel from userspace by reading or setting variables inside of modules. This can be useful for `debugging` purposes, or just as an interface for applications or scripts. U can find `sysfs` directories and files under the `/sys` directory on your system.

```bash
ls -l /sys
```

- Attributes can be exported for kobjects in the form of regular files in the filesystem. Sysfs forwards file I/O operations to methods defined for the attributes, providing a means to read and write kernel attributes.

- An attribute definition in simply:

```C
struct attribute {
  char* name;
  struct module *owner;
  umode_t mode;
}

int sysfs_create_file(struct kobject * kobj, const struct attribute * attr);
void sysfs_remove_file(struct kobject * kobj, const struct attribute * attr);
```

- For example, the driver model defines `struct device_attribute` like:

```C
struct device_attribute {
  struct attribute attr;
  ssize_t (*show)(struct device *dev, struct device_attribute *attr, char *buf);
  ssize_t (*store)(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
};

int device_create_file(struct device *, const struct device_attribute *);
void device_remove_file(struct device *, const struct device_attribute *);
```

- To read or write attributes, `show()` or `store()` method must be specified when declaring the attribute. For the common cases [include/linux/sysfs.h](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/include/linux/sysfs.h) provides convenience macros (`__ATTR`, `__ATTR_RO`, `__ATTR_WO`, etc) to make defining attributes easier as well as making code more concise and readable.

- Install module: `insmod sysfs.ko`

- Get the current value: `cat /sys/kernel/larva_property/_property`
- Set new value: `echo 1 > /sys/kernel/larva_property/_property`

- Since Linux v2.6.0, the `kobject` structure made its appearance. It was initially meant as a simple way of unifying kernel which manages reference counted object. After a bit of mission creep, it is now the glue that holds much of the device model and its sysfs interface together. For more information: [driver-model](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/Documentation/driver-api/driver-model/driver.rst), [kobject](https://lwn.net/Articles/51437/).

## 9. Talk to device files

- Device files are supposed to represent physcal devices. Most physical devices are used for output as well as input, so there has to be some mechanism for device drivers in the kernel to get the output to send to the device from processes. This is done by opening the device file for output and writing to it, just like writing to a file.

- This is not always enough. Imagine u had a serial port connected to a modem (even if u have an internal modem, it is still implemented from the CPU's perspective as a serial port connected to a modem, so u don't have to tax your imagination too hard).
  - The nature thing to do would be use the device file to write things to the modem and read things from the modem.
  - However, this leaves open question of `what to do when u need talk to the serial port itself?`, for example, to configure the rate at which data is sent and received.

- The answer in UNIX is to use a **special function** called `ioctl` (Input/Output Control).
  - Every device can have its own `ioctl` commands, which can be read ioctl's (to send information from a process to the kernel), write ioctl's (to return information to a process), both or neither.
  - **NOTICE** here the roles of read and write are reversed again, so int ioctl's read is to send info to the kernel and write is to receive info from kernel.

- The `ioctl` function is called with three arguments:
  - file descriptor of the device file.
  - ioctl number, and
  - a parameter: which is of type long so u can use a cast to use it to pass anything.

- U will not be able to pass a structure this way, but u will be able to pass a pointer to the structure.
