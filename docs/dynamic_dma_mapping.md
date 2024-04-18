# Dynamic DMA mapping

## 1. CPU and DMA addresses

- There are several kinds of addresses involved in the DMA API, and it's important to understand the differences.
- The kernel normally uses virtual addresses. Any address returned by `kmalloc()`, `vmalloc()`, and similar interfaces is a virtual address and can be stored in a `void *`.
- The virtual memory system (TLB, page tables, etc.) translates virtual addresses to CPU physical addresses, which are stored as `phys_addr_t` or `resource_size_t`. **The kernel manages device resources like registers as physical address**. These are the addresses in `/proc/iomem`. The physical address is not directly useful to a driver; it must use `ioremap()` to map the space and produce a virtual address.

- **I/O devices use a third kind of address: a `bus address`**. If a device has registers at an MMIO address, or if it performs DMA to read or write system memory, the addresses used by the device are bus addresses. In some systems, bus addresses are identical to CPU physical addresses, but in general they are not. `IOMMUs` and host bridges can produce arbitrary mappings between physical and bus addresses.

- From a device's point of view, DMA uses the bus address space, but it may be restricted to a subset of that space. For example, even if a system supports 64-bit addresses for main memory and PCI BARs, it may use an IOMMU so devices only need to use 32-bit DMA addresses.

- Here’s a picture and some examples:

```text
             CPU                  CPU                  Bus
           Virtual              Physical             Address
           Address              Address               Space
            Space                Space

          +-------+             +------+             +------+
          |       |             |MMIO  |   Offset    |      |
          |       |  Virtual    |Space |   applied   |      |
        C +-------+ --------> B +------+ ----------> +------+ A
          |       |  mapping    |      |   by host   |      |
+-----+   |       |             |      |   bridge    |      |   +--------+
|     |   |       |             +------+             |      |   |        |
| CPU |   |       |             | RAM  |             |      |   | Device |
|     |   |       |             |      |             |      |   |        |
+-----+   +-------+             +------+             +------+   +--------+
          |       |  Virtual    |Buffer|   Mapping   |      |
        X +-------+ --------> Y +------+ <---------- +------+ Z
          |       |  mapping    | RAM  |   by IOMMU
          |       |             |      |
          |       |             |      |
          +-------+             +------+
```

- During the enumeration process, the kernel learns about I/O devices and their MMIO space and the host bridges that connect them to the system.
- For example, if a PCI device has a BAR, the kernel reads the bus address `(A)` from the BAR and converts it to a CPU physical address `(B)`.
  - The address `(B)` is stored in a struct resource and usually exposed via `/proc/iomem`. When a driver claims a device, it typically uses `ioremap()` to map physical address `(B)` at a virtual address `(C)`. It can then use. e.g., `ioread32(C)`, to access the device registers at bus address `(A)`.

- If the device supports (DMA), the driver sets up a buffer using `kmalloc()` or a similar interface, which returns a virtual address `(X)`. The virtual memory system maps `(X)` to a physical address `(Y)` in system RAM. The driver can use virtual address `(X)` to access the buffer, but the device itself cannot because DMA doesn't go through the CPU virtual memory system.

- In some simple systems, the device can do DMA directly to physical address `(Y)`. But in many others, there is `IOMMU` hardware that translate DMA address to physical addresses, e.g. it translates `(Z)` to `(Y)`. This is part of reason for the DMA API: The driver can give a virtual address `(X)` to and interface like `dma_map_single()`, which sets up any required IOMMU mapping and returns the DMA address `(Z)`. The driver then tells the device to DMA to `(Z)`, and the IOMMU maps it to the buffer at address `(Y)` in system RAM.

- So that Linux can use the dynamic DMA mapping, it needs some help from the drivers, namely, it has to take into account that **DMA addresses should be mapped only for the time they are actually used and unmapped after the DMA transfer**.

## What memory is DMA’able?

- The first piece of information you must know is what kernel memory can be used with the DMA mapping facilities. There has been an written set of rules regarding this, and this text is an attempt to finally write them down.
- ...
