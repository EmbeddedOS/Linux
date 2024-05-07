# Linux DMA in device driver

- [link](https://www.youtube.com/watch?v=yJg-DkyH5CM&t=4s)

## 1. Agenda

- Memory allocation.
- Kernel configuration.
- Cache Control.
- DMA engine.
- DMA engine slave API.
- DMA Kernel Driver Example.

- Prerequisites:
  - 1. Knowledge of the Linux Kernel in general such as building and configuring the kernel.
  - 2. Basic device driver experience in Linux.
  - 3. Experience with the C programming language.

## 2. Introduction

- The goal of this session is to help users understanding the Linux kernel DMA framework and how it can be used in a device driver.
- DMA in Linux is designed to be used from a kernel space driver.
- User space DMA is possible and is a more advanced topic that is not covered in this presentation.
- The primary components of DMA include the DMA device control together with memory allocation and cache control.

## 3. Memory ALlocation For DMA

- Linux provides memory allocation functions in the kernel.
- The `vmalloc()` function allocates cached memory which is virtually contiguous but not physical contiguous.
  - Not as useful for DMA without an I/O MMU.
  - `Zynq` does not have an I/O MMU.
- The `kmalloc()` function allocates cached memory which is physical contiguous.
  - It is limited in the size of a single allocation.
  - Testing showed 4MB to be the limit, but it might vary with kernels.

```text

               Virtual                        Physical
                Memory                         Memory
                Pages                          Pages
  _ _ _ _ _  ___________                     ___________
            | Address 0 |                   | Address 0 |
    8KB     |           |==================>|           |
 Contiguous |___________|                   |___________|
   Memory   |  Address  |                   |  Address  |
            |   0x1000  |==================>|   0x1000  |
  _ _ _ _ _ |___________|                   |___________|
            |  Address  |                   |  Address  |
            |   0x2000  |                   |   0x2000  |
            |___________|                   |___________|
            |  Address  |                   |  Address  |
            |   0x3000  |       //=========>|   0x3000  |
  _ _ _ _ _ |___________|       ||          |___________|
            |  Address  |       ||          |  Address  |
  8KB NON   |   0x4000  |=======//          |   0x4000  |
 Contiguous |___________|                   |___________|
   Memory   |  Address  |                   |  Address  |
            |   0x5000  |=======\\          |   0x5000  |
  _ _ _ _ _ |___________|        ||         |___________|
            |  Address  |        ||         |  Address  |
            |   0x6000  |        \\========>|   0x6000  |
            |___________|                   |___________|
```

- The `dma_alloc_coherent()` function allocates **non-cached** physically contiguously memory.
  - The name `coherent` can be a confusing name (for me anyway).
  - The CPU and the I/O device see the same memory contents without any cache operations.
  - Accesses to the memory by the CPU are the same as a cache miss when the cache is used.
  - The CPU does not have to invalidate or flush the cache which can be time consuming.
  - This function is the intended function for DMA memory allocation.
  - There is another function, `dma_alloc_noncoherent()` but it's not really implemented so don't use it.

## 4. Boot Time Memory Setup

- Memory can be reserved such that the kernel does not use it.
  - `MEM=512M` on the kernel command line causes it to use only 512MB of memory.
  - The device tree memory can also be changed.
- This is the oldest method allowing large amounts of memory to be allocated for DMA.
- Drivers use `io_remap()` to map the physical memory address into virtual address space.
- There are multiple versions `io_remap()` which allow cached and non-cached.
- These functions don't allocate any memory, they only map the memory into the address space in the page tables.
- The Linux `io_remap()` function causes the memory to be setup as **Device Memory** in the MMU which should be slower than Normal Memory.

## 5. Device Memory vs Normal Memory (Cortex A9)

- Device Memory:
  - Each page of memory in Linux is setup with memory attributes based on its specific purpose.
  - The number and size of accesses are preserved, accesses are atomic, and will not be interrupted part way through.
  - Both read and write accesses can have side-effects on the system.
  - Accesses are never cached.
  - Speculative accesses are never be performed.
  - Accesses cannot be unaligned.
  - The order of accesses arriving at Device Memory is guaranteed to correspond to the program order of instructions which access device memory.
  - A write to Device memory is permitted to complete before it reaches peripheral or memory component accessed by the write.

- Normal memory:
  - The processor can repeat read and some write accesses.
  - The processor can pre-fetch or speculatively access additional memory locations, with no side-effects (if permitted by MMU access permission settings).
  - The processor does perform speculative writes.
  - Unaligned accesses can be performed.
  - Multiple access can be merged by processor hardware into a smaller number of accesses of a larger size.

## 6. Contiguous Memory Allocator (CMA)

- This is a newer feature of the kernel that some people may not know about.
- There had been a lot of demand for larger memory buffers needed for many applications including multimedia.
- CMA came into the kernel at version 3.5.
- Is only accessible in the DMA framework via `dma_alloc_coherent()`.
- Allows very large amounts of physical contiguous memory to be allocated.
- Defaults to small amounts.
  - Can be increased on the kernel command line (CMA=) which doesn't require a kernel rebuild.
  - Can be increased in the kernel configuration.

## 7. DMA Cache Control

- Linux provides DMA functions for cache control of DMA buffers.
- Cache control is based on the direction of DMA transfer, from memory to a device, from device to memory, or bidirectional.
- DMA controllers in the PL are cache coherent in `Zynq` with ACP port.
- For transfers from memory to a device, the memory must be flushed from the cache to memory before a DMA transfer is started.
- For transfers from a device to memory, the cache must be invalidated after the transfer and before the CPU access memory.
- `dma_map_single()` is provided to transfer ownership of a buffer from the CPU to the DMA hardware.
  - It can cause a cache flush for the buffer in the memory to device direction.
- `dma_unmap_single()` is provided to transfer ownership of a buffer from the DMA hardware back to the CPU.

## 8. Linux Kernel Details For DMA

- A **descriptor** is used to describe a DMA transaction such that a single data structure can be passed in an API.
- A **completion** is a lightweight mechanism which allows one thread to tell another thread that a task is done.
- A **tasklet** implements deferrable functionality and replaces older bottom half mechanisms for drivers.
  - A function can be scheduled to run at a later time with a **tasklet**.
- A **cookie** is an piece of opaque data which is returned from a function, then passed to yet a different function communicating information which only those functions understand.
  - A DMA cookie is returned from `dmaengine_submit()` and is passed to `dma_async_is_tx_complete()` to check for completion of a specific DMA transaction.
  - DMA cookies may also contain a status of a DMA transaction.

## 9. Linux DMA Engine

- A driver, `dmaengine.c`, along with Xilinx DMA drivers, is located in `driver/dma` of the kernel.
- Documentation: `Documentation/dmaengine.txt`
- The Xilinx Kernel has the DMA engine driver turned on by default.
  - The Xilinx DMA core drivers are only visible in the kernel configuration when it is enabled.
- The DMA test for the AXI DMA cores in the Xilinx kernel uses the DMA engine slave API.

```text
        Application
|--------------------------|
|          Kernel          |
|  ______________________  |
| |Device Specific Driver| |
| |______________________| |
| |  DMA Engine Driver   | |
| |______________________| |
| | Xilinx DMA Driver(s) | |
| |______________________| |
|__________________________|
```

## 10. Linux DMA Engine Slave API

- The DMA Engine driver works as a layer on top of the Xilinx DMA drivers using the slave DMA API.
  - It appears that `slave` may refer to the fact that the software initiates the DMA transactions to the DMA controller hardware rather than a hardware device with integrated DMA initiating a transaction.
- **Drivers which use the DMA Engine Driver are referred to as a client**.
- The API designed to handle complex DMA with scatter gather.

- The slave DMA usage consists of following these steps:
  - 1. Allocate a DMA slave channel.
  - 2. Set slave and controller specific parameters.
  - 3. Get a descriptor for transaction.
  - 4. Submit the transaction to queue it in the DMA engine.
  - 5. Issue pending requests (start the transaction).
  - 6. Wait for it to complete.

- Client drivers typically need a channel from a particular DMA controller only.
  - In some cases a specific channel is desired.
  - For AXI DMA, the 1st channel is the transmit channel and the 2nd channel is the receive channel.
- The function `dma_request_channel()` is used to request a channel.
  - A channel allocated is exclusive to the caller.
- The function `dma_release_channel()` is used to release a channel.
- The `dmaengine_prep_slave_single()` function gets a descriptor for a DMA transaction.
  - This is really converting a single buffer without a descriptor to use a descriptor.

- The `dmaengine_submit()` function submits the descriptor to the DMA engine to be put into the pending queue.
  - The returned **cookie** can be used to check the progress.
- The `dma_async_issue_pending()` function is used to start the DMA transaction that was previously put in the pending queue.
  - If channel is idle then the first transaction in queue is started and subsequent transactions are queued up.
  - On completion of each DMA operation, the next in queue is started and a `tasklet` triggered. The `tasklet` will then call the client driver completion callback routine fir notification, if set.

## 11. Allocating a Channel Example

```C
// Set up the capabilities for the channel that will be requested.
dma_cap_mask_t mask;
dma_cap_zero(mask);
dma_cap_set(DMA_SLAVE | DMA_PRIVATE, mask);

// Request the DMA channel from DMA engine.
chan = dma_request_channel(mask, NULL, NULL);

// Application do some thing with the channel.

// Release the channel after the application is done with it.
dma_release_channel(chan);
```

## 12. Starting A DMA Transfer Example

```C
// 1. Allocate a 1KB buffer of cached contiguous memory.
char *dma_buffer = kmalloc(1024, GFP_KERNEL);

// 2. Cause the buffer to be ready to use by the DMA including any cache operations required.
dma_map_single(device, dma_buffer, 1024, DMA_TO_DEVICE).

// 3. Create a descriptor for the DMA transaction.
enum dma_ctrl_flags flags = DMA_CRTL_ACK | DMA_PREP_INTERRUPT;
chan_descriptor = dmaengine_prep_slave_single(chan, buf, 1024,
                                              DMA_MEM_TO_DEV, flags);

// 4. Setup the callback function for the descriptor.
completion cmp;
chan_descriptor->callback = callback_function_when_the_transfer_completes;
chan_descriptor->callback_param = cmp;

// 5. Submit the transaction to the DMA engine: Queue the descriptor in the DMA engine.
dma_cookie_t cookie = dmaengine_submit(chan_descriptor);
```

### 12.1. Linux Asynchronous Transfer API

- The `async_tx` API provides methods for describing a chain of asynchronous bulk memory transfers/transforms with support for inter-transactional dependencies.
- It is implemented as a `dmaengine` client that smooths over details of different hardware offload engine implementations.
- Code that is written to the API can optimize for asynchronous operation and the API will fit the chain of operations to the available offload resources.

- The `dma_async_issue_pending()` function starts the DMA transaction:
  - The DMA engine calls the callback function that was supplied with the submit function when the transfer is complete.
- The `dma_async_is_tx_complete()` function checks to see if the DMA transaction completed.

## 13. Waiting For DMA Completion Example

```C

// A callback function was connected to the descriptor when it was submitted (queued).
// This function will called by the DMA Engine when the transfer complete.
void transfer_complete(void *completion)
{
    complete(completion);
}

// 1. Assume that a DMA transfer was previously submitted to the DMA engine.
unsigned long timeout = msecs_to_jiffies(3000);
enum dma_status = status;
struct completion cmp;

// 2. Initialize the completion so the DMA engine can indicate when it's done.
init_completion(&cmp);

// 3. Cause the DMA engine to start on any pending (queued) work.
dma_async_issue_pending(chan);

// 4. Wait for the DMA transfer to complete, it block until the completion completed or timeout.
timeout = wait_for_completion_timeout(&cmp, timeout);
```

### 14. Processing the transfer status example

```C
// Wait for the transfer to complete.
timeout = wait_for_completion_timeout(&cmp, timeout);

// Get the status of the DMA transfer using the cookie which was result of submitting it to the DMA Engine.
status = dma_async_is_tx_complete(chan, cookie, NULL, NULL);

// The transfer could have timed out or completed, with an error or OK.
if (timeout == 0)
{
    // Handle timeout.
} else if (status != DMA_COMPLETE)
{
    if (status == DMA_ERROR)
    {
        // Handle Error.
    }
}
```

## 15. Requesting a specific DMA channel

- The `dma_request_channel()` function provides parameters to allow a specific channel to be requested when there are multiple channels.

```C
struct dma_chan *dma_request_channel(dma_cap_mask_t mask,
                    dma_filter_fn filter_fn, void *filter_param);
```

- `dma_filter_fn` is defined as:
  - `typedef bool (*dma_filter_fn)(struct dma_chan *chan, void *filter_param);`.
  - The `filter_fn` routine will be called once for each free channel which has a capability matching those specified in the mask input.
  - `filter_fn` expect return `true` when desired DMA channel is found.

- The DMA channel unique ID is defined by the DMA driver using the DMA engine.

## 16. Requesting a specific DMA channel Example

```C
u32 device_id; // Assume that we get this from device tree.
u32 match;

// A filter function determine if the channel matches the desired channel.
static bool filter(struct dma_chan *chan, void *filter_param)
{
    if (*((int *) chan->private) == *(int *)filter_param)
    {
        return true;
    }

    return false;
}

direction = DMA_MEM_TO_DEV;
match = (direction & 0xFF) | device_id;
chan = dma_request_channel(mask, filter, &match);
```
