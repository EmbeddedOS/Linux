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

### 2.1.1. User space and kernel space

- All current processors have at least two protection levels, and some, like the x86 family, have more levels; when several levels exist, the highest and lowest levels are used.

- Unix transfer execution from user space to kernel space whenever an application issues a system call or is suspended by a hardware interrupt.
  - **Kernel code executing a system call is working in the context of process** -- it operates on behalf of the calling process and is able to access data in the process's address space.
  - Code that handles interrupts, on the other hand, is asynchronous with respect to processes and is not related to any particular process.

- **The role of a module is to extend kernel functionality**; modularized code runs in kernel space. Usually a driver performs both the tasks outlined previously: some functions in the module are executed as part of system calls, and some are in charge of interrupt handling.

### 2.1.2. Concurrency in the kernel

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

### 2.1.3. Current process
