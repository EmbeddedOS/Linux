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
