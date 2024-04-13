# Memory Mapping

- Understand address space mapping mechanisms.
- Learn about the most important structures related to memory management.
- [Link](https://linux-kernel-labs.github.io/refs/pull/160/merge/labs/memory_mapping.html)

## 1. Overview

- In the Linux Kernel it is possible to map a kernel address space to a user address space. This eliminates the overhead of copying user space information into the kernel space and vice versa. This can be done through a device driver and the user space device interface (`/dev`).

- This feature can be used by implementing the `mmap()` operation in the device driver's `struct file_operations` and using the `mmap()` system call in user space.

- The basic unit for virtual memory management is a **page**, which size is usually 4K, but it can be up to 64K on some platforms. Whenever we work with virtual memory we work with two type of addresses:
  - 1. virtual address.
  - 2. physical address.
- All CPU access (including from kernel space) uses virtual addresses that are translated by the MMU into physical addresses with the help of page tables.

- A physical page of memory is identified by the **Page Frame Number** (PFN). The PFN can be easily computed from the physical address by dividing it with the size of the page (or by shifting the physical address with **PAGE_SHIFT** bits to the right).
