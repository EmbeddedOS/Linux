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

- For efficiency reasons, the virtual address space is divided into user space and kernel space. For the same reason, **the kernel space contains a memory mapped zone, called `lowmem`**, which is contiguously mapped in physical memory, starting from the lowest possible physical address (usually 0). The virtual address where `lowmem` is mapped is defined by `PAGE_OFFSET`.

- On a 32 bit system, not all available memory can be mapped in `lowmem` and because of that there is a separate zone in kernel space called `highmem` which can be used to arbitrarily map physical memory.
- Memory allocated by `kmalloc()` resides in `lowmem` and it is physically contiguous. Memory allocated by `vmalloc()` is not contiguous and does not reside in `lowmem` (it has a dedicated zone in `highmem`).

```text
 _________3G______________________________3GB+896MB_________________
|User space|            LowMem               |      HighMem         |
|__________|___Contiguous_PHysical_Mapping___|__Arbitrary_mappings__|
           /                                /\      \
          /                                /  \      \
         /                                /   |      |
        /                                /    |      |
       /                                /     |Map   |
      /                                /      |some  |
     /                                /       |where |
    /                                /        |      |
   /                                /         |      |
  /                                /          |      |
 /                                /           |      |
/________________________________/____________|______|_______________
|                                |     |                |            |
|________________________________|_____|________________|____________|
OGB                             896GB
```

## 2. Structures used for memory mapping

- Some of the basic structures used by the Linux Memory Management sub-system. Some of the basic structures are: `struct page`, `struct vm_area_struct`, `struct mm_struct`.

- 1. `struct page`:
  - `struct page` is used to embed information about all physical pages in the system. The kernel has a `struct page` structure for all pages in the system.
  - There are many functions interact with this structure:
    - `virt_to_page()` returns the page associated with a virtual address.
    - `pfn_to_page()` returns the page associated with a page frame number.
    - `page_to_pfn()` returns the page frame number associated with a `struct page`.
    - `page_address()` returns the virtual address a `struct page;` this functions can be called only for pages from `lowmem`.
    - `kmap()` creates a mapping in kernel for an arbitrary physical page (can be from `highmem`) and returns a virtual address that can be used to directly reference the page.
- 2. `struct vm_area_struct`:
  - `struct vm_area_struct` holds information about **a contiguous virtual memory area**. The memory areas of a process can be viewed by inspecting the **maps** attribute of the process via `procfs`:

    ```text
    root@qemux86:~# cat /proc/1/maps
    #address          perms offset  device inode     pathname
    08048000-08050000 r-xp 00000000 fe:00 761        /sbin/init.sysvinit
    08050000-08051000 r--p 00007000 fe:00 761        /sbin/init.sysvinit
    08051000-08052000 rw-p 00008000 fe:00 761        /sbin/init.sysvinit
    092e1000-09302000 rw-p 00000000 00:00 0          [heap]
    4480c000-4482e000 r-xp 00000000 fe:00 576        /lib/ld-2.25.so
    4482e000-4482f000 r--p 00021000 fe:00 576        /lib/ld-2.25.so
    4482f000-44830000 rw-p 00022000 fe:00 576        /lib/ld-2.25.so
    44832000-449a9000 r-xp 00000000 fe:00 581        /lib/libc-2.25.so
    449a9000-449ab000 r--p 00176000 fe:00 581        /lib/libc-2.25.so
    449ab000-449ac000 rw-p 00178000 fe:00 581        /lib/libc-2.25.so
    449ac000-449af000 rw-p 00000000 00:00 0
    b7761000-b7763000 rw-p 00000000 00:00 0
    b7763000-b7766000 r--p 00000000 00:00 0          [vvar]
    b7766000-b7767000 r-xp 00000000 00:00 0          [vdso]
    bfa15000-bfa36000 rw-p 00000000 00:00 0          [stack]
    ```

  - **A memory area is characterized by a start address, a stop address, length and permissions**.
  - **A `structure vm_area_struct` is created at each `mmap()` call issued from user space**. A driver that supports the `mmap()` operation must complete and initialize the associated `struct vm_area_struct`. The most important fields of this structure are:
    - `vm_start, vm_end` - the beginning and the end of the memory area, respectively (these fields also appear in `/proc/<pid>/maps`);
    - `vm_file` - the pointer to the associated file structure (if any);
    - `vm_pgoff` - the offset of the area within file.
    - ``

