# Net link sockets - IPC between user space and kernel space

## Computer architecture

    Application 1 <----IPC----> Application 2
        ||
    Net link sockets,
       IOCTL,
    device files,
     system calls
        ||
        \/
      Kernel
        ||
    Device driver
        ||
        \/
    Hardware (CPU, Memory, Devices, etc.)

- Note: All device drivers could be LKM, but all LKMs are not device driver.

## Socket as a unified interface

- Net-link sockets are especially created to facilitate clean bidirectional communication between user space and kernel space.

- Other techniques can also be used for US <---> KS communication, but they were not invented for this purpose.
  - E.g: ioctl, device files, system calls.
    - If you want to make new system call to communication between US and KS, you have to rebuild the kernel.
    - U just don't write system call unless u have a very fairly good reason because system calls are actually general purpose calls, and those should be used by every other application that is running on the Linux platform just to meet the requirement of one application.

    - Device files have been invented, especially to write device drivers.

- A socket based technique was developed to build the unified interface using which user space application (USA) can interact with various kernel subsystems.

- We usually create a socket using `socket` system call:

```C
/*
 * These 3 arguments determine:
 * 1. Socket address family.
 * 2. Communication type: data-gram based or stream based.
 * 3. Protocol used for communication.
 */

int fd = socket(AF, socket_type, protocol_id);
```

- Thus, socket interface is unified - depending on arguments passed, we set up communication properties - whom to communicate, what to communicate, how to communicate.
