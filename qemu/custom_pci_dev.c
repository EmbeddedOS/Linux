#include "qemu/osdep.h"
#include "qemu/units.h"
#include "hw/pci/pci.h"
#include "hw/hw.h"
#include "hw/pci/msi.h"
#include "qemu/timer.h"
#include "qom/object.h"
#include "qemu/main-loop.h"
#include "qemu/module.h"
#include "qapi/visitor.h"

#define TYPE_PCI_CUSTOM_DEVICE  "c_pci_dev"
#define DEVICE_ID               0xABCD;
#define DEVICE_REVISION         0x10;
#define REG_OP1                 0x10
#define REG_OP2                 0x14
#define REG_OPCODE              0x18
#define REG_RESULT              0x20
#define REG_ERROR               0x24
#define OPCODE_ADD              0x00
#define OPCODE_MUL              0x01
#define OPCODE_DIV              0x02
#define OPCODE_SUB              0x03

#define BIG_BAR_SIZE            4096

#define DMA_REG_CMD             0x00
#define DMA_REG_SRC             0x04
#define DMA_REG_DST             0x08
#define DMA_REG_LEN             0x0C

/**
 * our CMD register:
 * Bit 0 is run DMA or not.
 * Bit 1 are DMA direction: to device or from device.
 */
#define DMA_CMD_RUN                 1
#define DMA_DIRECTION_TO_DEVICE     0
#define DMA_DIRECTION_FROM_DEVICE   1

#define DMA_GET_DIR(cmd) ((cmd & 0b10) >> 1)


typedef struct _pci_device_object _pci_device_object;

/**
 * @brief Construct a new declare instance checker object.
 * @InstanceType: instance struct name.
 * @OBJ_NAME: the object name is uppercase with underscore separtors.
 * @TYPENAME: type name.
 * 
 * Direct usage of this macro should be avoided, and the complete 
 * OBJECT_DECLARE_TYPE macro is recommended instead.
 * 
 * This macro will provide the instance type cast functions for a QOM type.
 */
DECLARE_INSTANCE_CHECKER(_pci_device_object, C_PCI_DEV, TYPE_PCI_CUSTOM_DEVICE);

/**
 * @brief This struct defining/descring the state of our pci device.
 * 
 */
struct _pci_device_object {
    PCIDevice _pci_dev;

    /* Test read/write large memory and also this region is used by 
     * DMA controller like a device memory region. */
    MemoryRegion _big_mem_region;
    uint8_t _big_mem_bar[BIG_BAR_SIZE];

    /* Test operators and interrupt. */
    MemoryRegion _mmio;
    uint32_t _operand_1;
    uint32_t _operand_2;
    uint32_t _opcode;
    uint32_t _result;
    uint32_t _error;

    /* Test DMA, to receive DMA command. */
    MemoryRegion _dma;
    struct dma_state {
        dma_addr_t _cmd;
        dma_addr_t _src;
        dma_addr_t _dst;
        dma_addr_t _len;
    } _dma_state;
};

static uint64_t _pci_dev_mmio_read(void *opaque, hwaddr addr, unsigned size)
{
    _pci_device_object *_pci_dev = (_pci_device_object *)opaque;
    uint64_t res = ~0ULL;
    printf("_PCI_DEV: _pci_dev_mmio_read() addr 0x%lx\n", addr);
    printf("_PCI_DEV: _pci_dev_mmio_read() size 0x%x\n", size);

    switch (addr)
    {
    case 0x00:
        printf("BAR\n");
        break;
    case REG_OP1:
        res = _pci_dev->_operand_1;
        break;
    case REG_OP2:
        res = _pci_dev->_operand_2;
        break;
    case REG_OPCODE:
        res = _pci_dev->_opcode;
        break;
    case REG_RESULT:
        switch (_pci_dev->_opcode) {
            case OPCODE_ADD:
                _pci_dev->_result = _pci_dev->_operand_1 + _pci_dev->_operand_2;
                break;
            case OPCODE_SUB:
                _pci_dev->_result = _pci_dev->_operand_1 - _pci_dev->_operand_2;
                break;
            case OPCODE_DIV:
                _pci_dev->_result = _pci_dev->_operand_1 / _pci_dev->_operand_2;
                break;
            case OPCODE_MUL:
                _pci_dev->_result = _pci_dev->_operand_1 * _pci_dev->_operand_2;
                break;
            default:
                _pci_dev->_result = 0x00;
                _pci_dev->_error = 0x01;
                break;
        }
        res = _pci_dev->_result;

        /* We fire interrupt when result ready. */
        pci_set_irq(&_pci_dev->_pci_dev, 1);
        break;
    case REG_ERROR:
        res = _pci_dev->_error;
        break;
    default:

        break;
    }

    return res;
}

static void _pci_dev_mmio_write(void *opaque, hwaddr addr, uint64_t val,
                unsigned size)
{
    _pci_device_object *_pci_dev = (_pci_device_object *)opaque;
    printf("_PCI_DEV: _pci_dev_mmio_write() addr 0x%lx\n", addr);
    printf("_PCI_DEV: _pci_dev_mmio_write() val 0x%lx\n", val);

    switch (addr)
    {
    case REG_OP1:
        _pci_dev->_operand_1 = val;
        break;
    case REG_OP2:
        _pci_dev->_operand_2 = val;
        break;
    case REG_OPCODE:
        _pci_dev->_opcode = val;
        break;
    }
}

static uint64_t _pci_dev_big_mem_mmio_read(void *opaque, hwaddr addr, unsigned size)
{
    _pci_device_object *_pci_dev = (_pci_device_object *)opaque;
    printf("_PCI_DEV: _pci_dev_big_mem_mmio_read() addr 0x%lx\n", addr);
    printf("_PCI_DEV: _pci_dev_big_mem_mmio_read() size 0x%x\n", size);

    if (size == 1) {
        return _pci_dev->_big_mem_bar[addr];
    } else if (size == 2) {
        uint16_t *ptr = (uint16_t *) &_pci_dev->_big_mem_bar[addr];
        return *ptr;
    } else if (size == 4) {
        uint32_t *ptr = (uint32_t *) &_pci_dev->_big_mem_bar[addr];
        return *ptr;
    } else if (size == 8) {
        uint64_t *ptr = (uint64_t *) &_pci_dev->_big_mem_bar[addr];
        return *ptr;
    }

    return 0xffffffffffffffL;
}

static void _pci_dev_big_mem_mmio_write(void *opaque, hwaddr addr, uint64_t val,
                unsigned size)
{
    _pci_device_object *_pci_dev = (_pci_device_object *)opaque;
    printf("_PCI_DEV: _pci_dev_big_mem_mmio_write() addr 0x%lx\n", addr);
    printf("_PCI_DEV: _pci_dev_big_mem_mmio_write() val 0x%lx\n", val);

    if (size == 1) {
        _pci_dev->_big_mem_bar[addr] = (uint8_t)val;
    } else if (size == 2) {
        uint16_t *ptr = (uint16_t *) &_pci_dev->_big_mem_bar[addr];
        *ptr = (uint16_t) val;
    } else if (size == 4) {
        uint32_t *ptr = (uint32_t *) &_pci_dev->_big_mem_bar[addr];
        *ptr = (uint32_t) val;
    } else if (size == 8) {
        uint64_t *ptr = (uint64_t *) &_pci_dev->_big_mem_bar[addr];
        *ptr = (uint64_t) val;
    }
}

static uint64_t _pci_dev_dma_mmio_read(void *opaque, hwaddr addr, unsigned size)
{
    _pci_device_object *_pci_dev = (_pci_device_object *)opaque;
    printf("_PCI_DEV: _pci_dev_dma_mmio_read() addr 0x%lx\n", addr);
    printf("_PCI_DEV: _pci_dev_dma_mmio_read() size 0x%x\n", size);

    return 0xffffffffffffffL;
}

static void fire_dma(_pci_device_object *_pci_dev)
{
        printf("pci_dma_*: src: %lx, dst: %lx, len: %ld, cmd: %lx\n",
               _pci_dev->_dma_state._src,
               _pci_dev->_dma_state._dst,
               _pci_dev->_dma_state._len,
               _pci_dev->_dma_state._cmd);

    if (DMA_GET_DIR(_pci_dev->_dma_state._cmd) == DMA_DIRECTION_TO_DEVICE)
    {
        if ((_pci_dev->_dma_state._dst + _pci_dev->_dma_state._len) > BIG_BAR_SIZE)
        {
            printf("Buffer overflow!\n");
            return;
        }

        /* Read from address and store in buffer. This function start transfer
         * from physical memory to device memory. */
        pci_dma_read(&_pci_dev->_pci_dev,                                // Device.
                     _pci_dev->_dma_state._src,                          // Physical Memory Address.
                     _pci_dev->_big_mem_bar + _pci_dev->_dma_state._dst, // Device Memory Buffer Address.
                     _pci_dev->_dma_state._len);                         // Length.

    } else if (DMA_GET_DIR(_pci_dev->_dma_state._cmd) == DMA_DIRECTION_FROM_DEVICE)
    {
        if ((_pci_dev->_dma_state._src + _pci_dev->_dma_state._len) > BIG_BAR_SIZE)
        {
            printf("Buffer overflow!\n");
            return;
        }

        /* Write data in buffer to address. This function start transfer from 
         * device memory to physical memory (defined by dest). */
        pci_dma_write(&_pci_dev->_pci_dev,                                // Device.
                      _pci_dev->_dma_state._dst,                          // Physical Address.
                      _pci_dev->_big_mem_bar + _pci_dev->_dma_state._src, // Device Memory Buffer Address.
                      _pci_dev->_dma_state._len);                         // Length.
    }
}

static void _pci_dev_dma_mmio_write(void *opaque, hwaddr addr, uint64_t val,
                unsigned size)
{
    _pci_device_object *_pci_dev = (_pci_device_object *)opaque;
    printf("_PCI_DEV: _pci_dev_dma_mmio_write() addr 0x%lx\n", addr);
    printf("_PCI_DEV: _pci_dev_dma_mmio_write() val 0x%lx\n", val);

    switch (addr)
    {
    case DMA_REG_CMD:
        _pci_dev->_dma_state._cmd = val;
        if (val & DMA_CMD_RUN)
        {
            printf("_PCI_DEV: fire dma!");
            fire_dma(_pci_dev);
        }
        break;
    case DMA_REG_SRC:
        _pci_dev->_dma_state._src = val;
        break;
    case DMA_REG_DST:
        _pci_dev->_dma_state._dst = val;
        break;
    case DMA_REG_LEN:
        _pci_dev->_dma_state._len = val;
        break;
    default:
        break;
    }
}

/**
 * @brief Memory region callbacks.
 * 
 * @read: Read from the memory region. @addr is relative to @mr; @size is in
 * bytes.
 * @write: Write to the memory region. 
 * @endianness: device endian: native, big or little endian.
 * @valid: Guest-visible constraints:
 *      @min_access_size, max_access_size: specify bounds on access sizes beyond
 *      which a machine check is thrown.
 * @impl: Internal implementation constraints.
 */
static const MemoryRegionOps _pci_dev_mmio_ops = {
    .read = _pci_dev_mmio_read,
    .write = _pci_dev_mmio_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    .valid = {
        .min_access_size = 4,
        .max_access_size = 8,
    },
    .impl = {
        .min_access_size = 4,
        .max_access_size = 8,
    },
};

static const MemoryRegionOps _pci_dev_big_mem_mmio_ops = {
    .read = _pci_dev_big_mem_mmio_read,
    .write = _pci_dev_big_mem_mmio_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    .valid = {
        .min_access_size = 1,
        .max_access_size = 8,
    },
    .impl = {
        .min_access_size = 1,
        .max_access_size = 8,
    },
};

static const MemoryRegionOps _pci_dev_dma_mmio_ops = {
    .read = _pci_dev_dma_mmio_read,
    .write = _pci_dev_dma_mmio_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    .valid = {
        .min_access_size = 4,
        .max_access_size = 8,
    },
    .impl = {
        .min_access_size = 4,
        .max_access_size = 8,
    },
};

static void _pci_dev_realize(PCIDevice *dev, Error **errp)
{
    _pci_device_object *_pci_dev = C_PCI_DEV(dev);
    uint8_t *pci_conf = dev->config;

    pci_config_set_interrupt_pin(pci_conf, 1);

    if (msi_init(dev, 0, 1, true, false, errp)) {
        return;
    }

    memset(_pci_dev->_big_mem_bar, 0, BIG_BAR_SIZE);

    _pci_dev->_operand_1 = 0x02;
    _pci_dev->_operand_2 = 0x04;
    _pci_dev->_opcode = 0xAA;
    _pci_dev->_result = 0xBB;
    _pci_dev->_error = 0x00;

    /**
     * @brief Initialize an I/O memory region. Accesses into the region will
     * cause the callbacks in @ops to be called. If @size is nonzero, subregions
     * will be clipped to @size.
     * 
     * @mr: The #MemoryRegion to be initialized.
     * @owner: The object that tracks the region's reference count.
     * @ops: A structure contain read and write callbacks to be used when I/O is
     *      performed on the region.
     * @opaque: Passed to the read and write callbacks of the @ops structure.
     * @name: Used for debugging; not visible to the user or ABI.
     * @size: Size of the region (in bytes).
     * 
     */
    memory_region_init_io(&_pci_dev->_mmio,
                            OBJECT(_pci_dev),
                            &_pci_dev_mmio_ops,
                            _pci_dev,
                            "_pci_dev-mmio",
                            0x40); // We only need 64 bytes for registers.

    /**
     * @brief This function attach newly allocated `MemoryRegions` to the PCI
     * bus address space.
     * The device specifications has several BARs (Base Address Register) (link:
     * https://wiki.osdev.org/PCI#Base_Address_Registers):
     * - BAR0, code flash.
     * - BAR1, data flash.
     * - BAR2, EEPROM.
     * - BAR3, various configuration registers.
     * 
     * The BARs are exposed through our device PCI config space, but there value
     * might be changed by an OS driver at runtime. As they refer to the
     * location of memory mapped device registers, there should exist a QEMU
     * internal `moulinette` to inform the related emulated devices of their
     * possible relocation.
     */
    pci_register_bar(dev, 0, PCI_BASE_ADDRESS_SPACE_MEMORY, &_pci_dev->_mmio);

    /* Big memory, for mapping big region. */
    memory_region_init_io(&_pci_dev->_big_mem_region,
                          OBJECT(_pci_dev),
                          &_pci_dev_big_mem_mmio_ops,
                          _pci_dev,
                          "_pci_dev-mmio",
                          BIG_BAR_SIZE);

    pci_register_bar(dev,
                     1,
                     PCI_BASE_ADDRESS_SPACE_MEMORY,
                     &_pci_dev->_big_mem_region);

    /* DMA controller. */
    memory_region_init_io(&_pci_dev->_dma,
                          OBJECT(_pci_dev),
                          &_pci_dev_dma_mmio_ops,
                          _pci_dev,
                          "_pci_dev-mmio",
                          0x20);

    pci_register_bar(dev,
                     2,
                     PCI_BASE_ADDRESS_SPACE_MEMORY,
                     &_pci_dev->_dma);
}

static void _pci_dev_exit(PCIDevice *pdev)
{
    return;
}

static void _pci_dev_instance_init(Object *obj)
{
    return;
}

static void _pci_dev_class_init(ObjectClass *class, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(class);

    /**
     * @realize: Call back function when the device is `realization`.
     */
    PCIDeviceClass *k = PCI_DEVICE_CLASS(class);
    k->realize = _pci_dev_realize;
    k->exit = _pci_dev_exit;
    k->vendor_id = PCI_VENDOR_ID_QEMU;
    k->device_id = DEVICE_ID;
    k->revision = DEVICE_REVISION;
    k->class_id = PCI_CLASS_OTHERS;

    /* Set device categories is MISC. */
    set_bit(DEVICE_CATEGORY_MISC, dc->categories);
}

static void pci_custom_device_register_types(void)
{
    static InterfaceInfo interfaces[] = {
        { INTERFACE_CONVENTIONAL_PCI_DEVICE },
        { },
    };

    /**
     * TypeInfo:
     * @name: The name of the type.
     * @parent: The name of the parent type.
     * @instance_size: The size of the object (derivative of #Object). If
     *      @instance_size is 0, then the size of the object will be the size of
     *      the parent object.
     * @instance_init: This function is called to initialize an object. The
     *      parent class will have already been initialized so the type is only
     *      responsible for initializing its own members.
     * @class_init: This func is called after all parent class initialization
     *      has occurred to allow a class to set its default virtual method
     *      pointers. This is also the function to use to override virtual
     *      methods from a parent class.
     * @interfaces: The list of interfaces associated with this type. This
     *      should point to a static array that's terminated with a zero filled
     *      element.
     */
    static const TypeInfo _pci_device_info = {
        .name          = TYPE_PCI_CUSTOM_DEVICE,
        .parent        = TYPE_PCI_DEVICE,
        .instance_size = sizeof(_pci_device_object),
        .instance_init = _pci_dev_instance_init,
        .class_init    = _pci_dev_class_init,
        .interfaces = interfaces,
    };

    // Registers the new type.
    type_register_static(&_pci_device_info);
}

/* Init QEMU module via this macro. Our register type function will be called
 * automatically. */
type_init(pci_custom_device_register_types);
