# Linux Device Driver

## 1. An introduction to Device Drivers

### 1.1. The Role of the Device Driver

- Most programming problems can indeed be split into two parts:
  - 1. What capabilities are to be provided - **The mechanism**.
  - 2. How those capabilities can be used - **The policy**.

- When writing drivers, a programmer should pay particular attention to this fundamental concept: `Write kernel code to access the hardware, but don't force particular policies on the user, since different users have different needs.`
  - The driver should deal with making the hardware available, leaving all the issues about *HOW* to use the hardware to the applications.

- You can also look at your driver from a different perspective: `It is a software layer that lies between the applications and the actual device.` This privileged role of the driver allows the driver programmer to choose exactly how the device should appear: Different drivers can offer different capabilities, even for the same device.

### 1.2. Splitting the Kernel

- In a Unix system, several concurrent *processes* attend to different tasks. Each process asks for system resources, be it computing power, memory, network connectivity, or some other resource. The *Kernel* is the big chunk of executable code in charge of handling all such requests. The kernel's role can be split into the following parts:
  - 1. **Process management**: The kernel is in charge of creating and destroying processes and handling their connection to the outside world (input and output, IPC). In addition, the scheduler, which controls how processes share the CPU, is part of process management.
  - 2. **Memory management**: The computer's memory is major resource, and the policy used to deal with it is a critical one for system performance. The kernel builds up a virtual addressing space for any and all processes on top of the limited available resources.
  - 3. **File systems**: Unix is heavily based on the filesystem concept; almost everything in UNIX can be treated as a **FILE**. The kernel builds a structured filesystem on top of unstructured hardware, and the resulting file abstraction is heavily used throughout the whole system.
  - 4. **Device control**: Almost every system operation eventually maps to a physical device. With the exception of the processor, memory, and a very few other entities, any and all device control operations are performed by code that is specific to the device being addressed. That code is called a **device driver**.
  - 5. **Networking**: Networking must be managed by the OS, because most network operations are not specific to a process: incoming packets are asynchronous events. The packets must be collected, identified, and dispatched before a process takes care of them. The system is in charged of delivering data packets across program and network interfaces, and it must control the execution of programs according to their network activity. Additionally, all the routing and address resolution issues are implemented within the kernel.

### 1.3. Loadable module

- One of the good features of Linux is the ability to extend at runtime the set of features offered by the kernel.
- Each piece of code that can be added to the kernel at runtime is called a **module**.
- Each module is made up of object code that can be dynamically linked to the running kernel by the `insmod` program and can be unlinked by the `rmmod` program.

### 1.4. Classes of Devices and Modules

- Three fundamental device types:
  - 1. `character module`.
  - 2. `block module`.
  - 3. `network module`.

- 1. **Character devices**:
  - A character (char) device is one that can be accessed as a stream of bytes.
  - A character driver is in charge of implementing this behavior. Such a driver usually implements at least the `open()`, `close()`, `read()` and `write()` system calls.
  - For example: `devconsole` and `devtty`.
- 2. **Block devices**:
  - Like char devices, block devices are accessed by filesystem nodes in the `/dev` directory. A block device is a device (e.g., a disk) that can host a filesystem.
- 3. **Network interfaces**:
  - Any network transaction is made through an interface, that is, a device that is able to exchange data with other hosts.
  - Usually, an *interface* is a hardware device, but it might also be a pure software device, like the `loopback interface`.
  - A network interface is in charge of sending and receiving data packets, driven by the network subsystem of the kernel.
  - Many network connections (especially those using TCP) are stream-oriented, but network devices are, usually, designed around the transmission and receipt of packets. A network driver knows nothing about individual connections; it only handles packets.

- Not being a stream-oriented device, a network interface isn't easily mapped to a node in the filesystem, as `devtty1` is. The Unix way to provide access to interfaces is still by assigning a unique name to them (such as `eth0`), but that name doesn't have a corresponding entry in the filesystem. Instead of `read` and `write`, the kernel calls functions related to packet transmission.

## 2. Building and running modules

### 2.1. Kernel module versus Applications

- While most small and medium-sized applications perform a single task from beginning to end, every *kernel module just registers itself in order to serve future requests*, and its initialization function terminates immediately.
  - In other words, the task of the module's initialization function is to prepare for later invocation of the module's functions; `Here I am, and this is what I can do`.
  - The module's exit function gets invoked just before the module is unloaded. It should tell the kernel, `I'm not there anymore; don't ask me to do anything else`.

- This kind of approach to programming is similar to **event-driven programming**.

- In applications, you can call functions it doesn't define: the linking stage resolves external references using the appropriate library of functions.
- But in kernel, is linked only to the kernel, no libraries to link.
- Most of the relevant headers live in `include/linux` and `include/asm`.

#### 2.1.1. User space and kernel space

- All current processors have at least two protection levels, and some, like the x86 family, have more levels; when several levels exist, the highest and lowest levels are used.

- Unix transfer execution from user space to kernel space whenever an application issues a system call or is suspended by a hardware interrupt.
  - **Kernel code executing a system call is working in the context of process** -- it operates on behalf of the calling process and is able to access data in the process's address space.
  - Code that handles interrupts, on the other hand, is asynchronous with respect to processes and is not related to any particular process.

- **The role of a module is to extend kernel functionality**; modularized code runs in kernel space. Usually a driver performs both the tasks outlined previously: some functions in the module are executed as part of system calls, and some are in charge of interrupt handling.

#### 2.1.2. Concurrency in the kernel

- Even the simplest kernel modules must be written with the idea that many things can be happening at once.

- There are a few sources of concurrency in kernel programming.
  - 1. Naturally, Linux system run multiple processes, more than one of which can be trying to use your driver at the same time.
  - 2. Most devices are capable of interrupting the processor; interrupt handlers run asynchronously and can be invoked at the same time that your driver is trying to do something else.
  - 3. Several software abstractions (such as kernel timer) run asynchronously as well.
  - 4. Moreover, of course, Linux can run on symmetric multiprocessor (SMP) systems, with the result that your driver could be executing concurrently on more than one CPU.
  - 5. Finally, in 2.6, kernel code has been made **preemptible**; that change causes even uni-processor systems to have many of the same concurrency issues as multiprocessor systems.

- As a result, Linux kernel code, including driver code, must be **REENTRANT** -- it must be capable of running in more than one context at the same time.
- Data structures must be carefully designed to keep multiple threads of execution separate, and he code must take care to access shared data in ways that prevent corruption of the data.

- *A common mistake made by driver programmers is to assume that concurrency is not a problem as long as a particular segment of code does not go to sleep*. If you do not write your code with concurrency in mind, it will be subject to catastrophic failures that can be exceedingly difficult to debug.

#### 2.1.3. Current process

- Although kernel modules, don't execute sequentially as applications do, *most actions* performed by the kernel are done on behalf of a specific process.
- Kernel code can refer to the current process by accessing the global item `current`, defined in `asm/current.h`, which yields a pointer to `struct task_struct`.
- For example for x86 (`arch/x86/include/asm/current.h`):

    ```C
    struct task_struct;

    struct pcpu_hot {
        union {
            struct {
                struct task_struct    *current_task;
                int            preempt_count;
                int            cpu_number;
    #ifdef CONFIG_CALL_DEPTH_TRACKING
                u64            call_depth;
    #endif
                unsigned long        top_of_stack;
                void            *hardirq_stack_ptr;
                u16            softirq_pending;
    #ifdef CONFIG_X86_64
                bool            hardirq_stack_inuse;
    #else
                void            *softirq_stack_ptr;
    #endif
            };
            u8    pad[64];
        };
    };
    static_assert(sizeof(struct pcpu_hot) == 64);

    DECLARE_PER_CPU_ALIGNED(struct pcpu_hot, pcpu_hot);

    static __always_inline struct task_struct *get_current(void)
    {
        return this_cpu_read_stable(pcpu_hot.current_task);
    }

    #define current get_current()
    ```

- The `current` pointer refers to the process that is currently executing.
- Actually, `current` is not truly a global variable. The need to support SMP systems forced the kernel developers to develop a mechanism that finds the current process on the relevant CPU.
- This mechanism must also be fast, since references to `current` happen frequently. The result is an architecture-dependent mechanism that, usually, hides a pointer to the `task_struct` structure on the kernel stack.

#### 2.1.4. A Few Other Details

- 1. Application are laid out in virtual memory with a very large stack area. The kernel, instead, has a very small stack; it can be as small as a single, 4096 byte page. Your functions must share that stack with the entire kernel-space call chain. Thus, **It is never a good idea to declare large automatically variables**; If you need larger structures, you should allocate them dynamically at call time.

- 2. Often, as you look at the kernel API, you will encounter function names starting with a double underscore `__`. Functions so marked are generally a low-level component of the interface and should be used with caution.
  - Essentially, the double underscore says to the programmer: `If you call this function, be sure you know what you are doing.`

- 3. Kernel code cannot do floating point arithmetic. Enabling floating point would require that the kernel save and restore the floating point processor's state on each entry to, and exit from, kernel space--at least, on some architectures. `Given that there really is no need for floating point in kernel code`.

### 2.2. Compiling and Loading

- The kernel build system is a complex beast, and we just look at a tiny piece of it.

#### 2.2.1. Loading and Unloading Modules

- `insmod` loads the module code and data into the kernel, which, in turn, performs a function similar to that of `ld`, in that it links any unresolved symbol in the module to the symbol table of the kernel.

- 1. The function `sys_init_module` allocates kernel memory to hold a module (this memory is allocated with `vmalloc`).
- 2. Copies the module text into that memory region.
- 3. Resolves kernel references in the module via the kernel symbol table.
- 4. And calls the module's initialization function to get everything going.

##### 2.2.1.1. `modprobe`

- This utility, like insmod, load a module into the kernel. It differs in that it will look at the module to be loaded to see whether it references any symbols that are not currently defined in the kernel. If any such references are found, `modprobe` looks for other modules in the current module search path hat define the relevant symbols.
- When the `modprobe` finds those module (which are needed by module being loaded), it loads them into the kernel as well.

- If you use `insmod` in this case, `unresolved symbols` message left.

#### 2.2.2. Version dependency

- You can use some macros to to something version dependency:

- 1. `LINUX_VERSION_CODE`: binary representation of the kernel version.
- 2. `UTS_RELEASE`: a string describing the version of this kernel tree.
- 3. `KERNEL_VERSION(major,minor,release)`: an integer version code from individual numbers that build up a version number.

### 2.3. The Kernel Symbol Table

- The table contains the addresses of global kernel items -- functions and variables -- that are needed to implement modularized drivers.
- When a module is loaded, any symbol exported by the module becomes part of the kernel symbol table.

- New modules can use symbols exported by your module, and can *stack* new modules on top of other modules. For example:
  - 1. Each input USB device module stacks on the `usbcore` and `input` modules.

- **Module stacking** is useful in complex project. If a new abstraction is implemented in the form of a device driver, it might offer a plug for hardware specific implementations. For example:

```text
     __________
    |  __lp__  |
     ||      || Port sharing and device registration.
     ||     _\/______________
     ||    |_  _parport___  _|
     ||      ||           || Low-level device operations.
     ||      ||          _\/____________
     ||      ||         |__partport_pc__|
     ||      ||                ||
     \/______\/________________\/_________
    |           Kernel API                |
    |(Message print, driver registration, |
    |port allocation, etc.)               |
    |_____________________________________|
```

- When using stacked modules, it is helpful to be aware of the `modprobe` utility.
- Using stacking to split modules into multiple layers can help reduce development time by simplifying each layer.

- If your module needs to export symbols for other modules to use, the following macros should be used:

    ```C
    EXPORT_SYMBOL(name);
    EXPORT_SYMBOL_GPL(name);
    ```

### 2.3. Initialization and shutdown

- Initialization functions should be declared `static`, since they are not meant to be visible outside the specific file.
- `init` token is a hint to the kernel that the given function is used only at initialization time. The module loader drops the initialization function after the module is loaded, making its memory available for other uses.

#### 2.3.1. The cleanup function

- The `exit` modifier marks the code as being for module unload only (by causing the compiler to place it in a special ELF section). If your module is built directly into the kernel, or if your kernel is configured to disallow the unloading of modules, functions marked `exit` are simply discarded.

#### 2.3.2. Error Handling During Initialization

- One thing you must always bear in mind when registering facilities with the kernel is that the registration could fail. Even the simplest action often requires memory allocation, and the required memory may not be available. So **module code must always check return values**, and be sure that the requested operations have actually succeeded.

```C
int init my_init_function(void)
{
    int err;
    /* registration takes a pointer and a name */
    err = register_this(ptr1, "skull");
    if (err) goto fail_this;
    err = register_that(ptr2, "skull");
    if (err) goto fail_that;
    err = register_those(ptr3, "skull");
    if (err) goto fail_those;

    return 0; /* success */
    fail_those: unregister_that(ptr2, "skull");
    fail_that: unregister_this(ptr1, "skull");
    fail_this: return err; /* propagate the error */
}
``

- Or your initialization is more complex, you can define the clean function like this:

```C
struct something *item1;
struct somethingelse *item2;
int stuff_ok;

void my_cleanup(void)
{
    if (item1)
        release_thing(item1);
    if (item2)
        release_thing2(item2);
    if (stuff_ok)
        unregister_stuff( );
    return;
}

int init my_init(void)
{
    int err = -ENOMEM;
    item1 = allocate_thing(arguments);
    item2 = allocate_thing2(arguments2);
    if (!item2 || !item2)
        goto fail;
    err = register_stuff(item1, item2);
    if (!err)
        stuff_ok = 1;
    else
        goto fail;
    return 0; /* success */
    fail:
        my_cleanup( );
        return err;
}
```

### 2.4. Doing it in User space

- There are some arguments in favor of user-space programming, and sometimes writing a so-called `user-space device driver` is a wise alternative to kernel hacking.

- The advantages of user-space device driver:
  - 1. Full C library can be linked in.
  - 2. Conventional debugger on the driver code.
  - 3. If a user-space driver hangs, you can simply kill it.
  - 4. User memory is swappable, unlike kernel memory.
  - 5. A well-designed driver program can still, like kernel-space drivers, allow concurrent access to a device.

- For example, USB drivers can be written for user space; see the `libusb` project.

- But the user-space approach to device driving has a number of drawbacks. Some important are:
  - 1. Interrupts are not available in user space. There are workarounds for this limitation of some platforms.
  - 2. Direct access to memory is possible only by `mmap` `devmem`, and only a privileged user can do.
  - 3. Access to I/O ports is available only after calling `ioperm` or `iopl`.
  - 4. Response time is slower, because a context switch is required to transfer information or actions between client and the hardware.
  - 5. If the driver has been swapped to disk, response time is unacceptable long.
  - 6. Some important devices can't be handled in user space: network interfaces and block devices.

## 3. Char Drivers

- `scull`: Simple Character Utility for Loading Localities. `scull` is a char driver that acts on a memory area as though it were a device.
  - The advantage of scull is that it isn't hardware dependent. `scull` jut acts on some memory, allocated from the kernel.

### 3.1. The design of `scull`

- The first step of driver writing is defining the capabilities (the mechanism) the driver will offer to user programs.
- To make `scull` useful as a template for writing real drivers for real devices, we'll show you how to implement several device abstractions on top of the computer memory, each with a different personality.

- The `scull` source implements the following devices. Each kind of device implemented by the module is referred to as a *type*.
  - `scull0` to `scull3`:
    - Four devices, each consisting of a memory area that is both global and persistent.
      - **Global** means if device is opened multiple times, the data contained within the device is shared by all the file descriptors.
      - **Persistent** means that if the device is closed and reopened, data isn't lost.
    - This device can be fun to work with, because it can be accessed and tested using conventional commands: `cp`, `cat`, shell IO redirection.
  - `scullpipe0` to `scullpipe3`:
    - Four FIFO devices, which act like pipes. One process reads and another writes.
  - `scullsingle`
  - `scullpriv`
  - `sculluid`
  - `scullwuid`

- Each of the `scull` devices demonstrates different features of a driver and presents different difficulties.

### 3.2. Major and Minor

- Char devices are accessed through name in the filesystem. Those names are called special files or device files or simply nodes of the file system tree; they are conventionally located in the `/dev` directory.

- For example `ls -l`:

```text
crw-rw-rw-1 root root 1, 3 Apr 11 2002 null
crw------- 1 root root 10, 1 Apr 11 2002 psaux
crw------- 1 root root 4, 1 Oct 28 03:04 tty1
crw-rw-rw-1 root tty 4, 64 Apr 11 2002 ttys0
crw-rw---- 1 root uucp 4, 65 Apr 11 2002 ttyS1
crw--w---- 1 vcsa tty 7, 1 Apr 11 2002 vcs1
crw--w---- 1 vcsa tty 7, 129 Apr 11 2002 vcsa1
crw-rw-rw-1 root root 1, 5 Apr 11 2002 zero
```

- `c` means character device file. `b` block device file.
- You can see 1, 10, 4, 7 are **MAJOR** numbers and 3, 1, 64, 65, 129, 5 are **MINOR** numbers.

- Traditionally, the major number identifies the driver associated with the device.
  - For example, `devnull` and `devzero` are both managed by driver 1, whereas `virtual console` and `serial terminals` are managed by driver 4;

- The minor number is used by the kernel to determine exactly which device is being referred to. Kernel itself knows almost nothing about minor numbers beyond the fact that they refer to devices implemented by your driver.

#### 3.2.1. The Internal Representation of Device Numbers

- Within the kernel, the `dev_t` type is used to hold device numbers -- both the major and minor parts.
- To obtain the major or minor parts of a `dev_t`, use:

```C
MAJOR(dev_t dev);
MINOR(dev_t dev);
```

- Make `dev_t` struct from numbers:

```C
MKDEV(int major, int minor);
```

#### 3.2.2. Allocating and Freeing Device Numbers

- One of the first things your driver will need to do when setting up a char device is to obtain one or more device numbers to work with. The necessary function:

    ```C
    int register_chrdev_region(dev_t first, unsigned int count, char *name);
    ```

  - `first` is the beginning device number of the range you would like to allocate.
  - `count` is the total number of contiguous device numbers you are requesting.
  - `name` name of the device.

- The `register_chrdev_region` works well if you know ahead of time exactly which device numbers you want.

- Often, however, you will not know which major numbers your device will use; there is a constant effort within the Kernel community to move over to the use of dynamically-allocated device numbers. The kernel will happily allocate a major number for you on the fly, but you must request this allocation by using a different function:

    ```C
    int alloc_chrdev_region(dev_t *dev, unsigned int firstminor, unsigned int count, char *name);
    ```

  - `dev` is output parameter that will, on successful completion, hold the first number in your allocated range.

- You should free them when they are no longer in use:

```C
void unregister_chrdev_region(dev_t first, unsigned int count);
```

### 3.2.3. Dynamic Allocation of Major Numbers

- Some major device numbers are statically assigned to the most common devices. A list of those devices can be found in `Documentation/devices.txt`.

- So as a driver writer, you have a choice: you can simply pick a number that appears to unused, you can allocate major numbers in a dynamic manner.

- We recommend use dynamic allocation to obtain your major device number. In other words, your driver should almost certainly be using `alloc_chrdev_region` rather than `register_chrdev_region`.

- Read `procdevices` file to see major numbers of current drivers:

```text
Character devices:
    1 mem
    2 pty
    3 ttyp
    4 ttyS
    6 lp
    7 vcs
    10 misc
    13 input
    14 sound
    21 sg
    180 usb
Block devices:
    2 fd
    8 sd
    11 sr
    65 sd
    66 sd
```

- Script to create 4 scull devices [scull_load](https://github.com/starpos/scull/blob/master/scull/scull_load):

```bash
#!binsh
module="scull"
device="scull"
mode="664"

# invoke insmod with all arguments we got
# and use a pathname, as newer modutils don't look in . by default
sbininsmod ./$module.ko $* || exit 1

# remove stale nodes
rm -f dev${device}[0-3]
major=$(awk "\\$2= =\"$module\" {print \\$1}" procdevices)

mknod dev${device}0 c $major 0
mknod dev${device}1 c $major 1
mknod dev${device}2 c $major 2
mknod dev${device}3 c $major 3

# give appropriate group/permissions, and change the group.
# Not all distributions have staff, some have "wheel" instead.
group="staff"
grep -q '^staff:' etcgroup || group="wheel"

chgrp $group dev${device}[0-3]
chmod $mode dev${device}[0-3]
```

- `mknod` was originally used to create the character and block devices that populate `/dev/`.
- The script can be adapted for another driver by redefining the variables and adjusting the `mknod` lines.

- A [scull_unload](https://github.com/starpos/scull/blob/master/scull/scull_unload) script is also available to clean up the `/dev` directory and remove module.

- The best way to assign major numbers, is by defaulting to dynamic allocation while leave yourself the option of specifying the major number at load time, or even compile time.
  - The `scull` work in this way. For example, if you init scull_major with `0` we use the dynamic allocation:

    ```C
    if (scull_major) {
        dev = MKDEV(scull_major, scull_minor);
        result = register_chrdev_region(dev, scull_nr_devs, "scull");
    } else {
        result = alloc_chrdev_region(&dev, scull_minor, scull_nr_devs,
        "scull");
        scull_major = MAJOR(dev);
    }
    if (result < 0) {
        printk(KERN_WARNING "scull: can't get major %d\n", scull_major);
        return result;
    }
    ```

### 3.2.4. Some Important Data structures

- Most if the fundamental driver operations involve three important kernel data structures:
  - 1. `file_operations`.
  - 2. `file`.
  - 3. `inode`.

#### 3.2.4.1. File Operations

- 1. `struct module *owner`: A pointer to the module that owns the structure. It's used to prevent the module being unloaded while its operations are in use.
- 2. `loff_t (*llseek)(struct file *, loff_t, int);`: The `llseek` method is used to change the current read/write position in a file and the new position i returned as 