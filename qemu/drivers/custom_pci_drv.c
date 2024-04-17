#include <linux/init.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/cdev.h>

#define TYPE_PCI_CUSTOM_DEVICE  "c_pci_dev"
#define DEVICE_VENDOR_ID        0x1234
#define DEVICE_DEVICE_ID        0xABCD

#define REG_OP1                 0x10
#define REG_OP2                 0x14
#define REG_OPCODE              0x18
#define REG_RESULT              0x20
#define REG_ERROR               0x24
#define OPCODE_ADD              0x00
#define OPCODE_MUL              0x01
#define OPCODE_DIV              0x02
#define OPCODE_SUB              0x03

#define DEVICE_NAME TYPE_PCI_CUSTOM_DEVICE

#define __pr_info(fmt, arg...) pr_info("%s():" fmt, __FUNCTION__, ##arg)
#define __pr_err(fmt, arg...) pr_err("%s():" fmt, __FUNCTION__, ##arg)


struct c_pci_dev {
    struct pci_dev *_dev;
    struct class *_cls;
    int _major;
} _dev;

static struct pci_device_id dev_ids[] = {
    {PCI_DEVICE(DEVICE_VENDOR_ID, DEVICE_DEVICE_ID)},
    {}
};
MODULE_DEVICE_TABLE(pci, dev_ids);

static int _mmap(struct file *file, struct vm_area_struct *vma)
{
    int res = 0;

    /* VM Area offset will point to first page of PCI DMA (physical addr).
     * pci_resource_start() return start address od PCI BAR.
     * We shift `PAGE_SHIFT` bits the address to right to get the page number.
     **/
    vma->vm_pgoff = pci_resource_start(_dev._dev, 0) >> PAGE_SHIFT;

    /* We map user VMA to the BAR. */
    res = io_remap_pfn_range(vma,
                             vma->vm_start,
                             vma->vm_pgoff,
                             vma->vm_end - vma->vm_start,
                             vma->vm_page_prot);
    if (res) {
        __pr_err("Failed to map PCI BAR 0 to user VMA: %d", res);
        res = -res;
        goto exit;
    }

exit:
    return res;
}

static int _open(struct inode *inode, struct file *f);
static int _release(struct inode *inode, struct file *f);
static ssize_t _read(struct file *f, char __user *p, size_t size, loff_t *offset);
static ssize_t _write(struct file *f, const char __user *p, size_t size, loff_t *offset);

static struct file_operations f_ops = {
    // .read = _read,
    // .write = _write,
    // .open = _open,
    // .release = _release,
    .mmap = _mmap,
};

static int _probe(struct pci_dev *dev, const struct pci_device_id *id)
{
    int res = 0;
    void __iomem *bar_0_ptr = NULL;
    void __iomem *bar_1_ptr = NULL;
    void __iomem *bar_2_ptr = NULL;

    /* 1. Enable PCI device. */
    res = pcim_enable_device(dev);
    if (res < 0) {
        pr_err("%s(): Failed to enable PCI device.\n", __FUNCTION__);
        goto exit;
    }

    /* Map device's memory regions. */
    bar_0_ptr = pcim_iomap(dev, 0, pci_resource_len(dev, 0));
    if (bar_0_ptr == NULL)
    {
        pr_err("%s(): Failed to map mem region 0.\n", __FUNCTION__);
        res = -ENODEV;
        goto exit;
    }

    pr_info("%s(): Region 0 length: %d \n", __FUNCTION__, pci_resource_len(dev, 0));

    bar_1_ptr = pcim_iomap(dev, 1, pci_resource_len(dev, 1));
    if (bar_1_ptr == NULL)
    {
        pr_err("%s(): Failed to map mem region 1.\n", __FUNCTION__);
        res = -ENODEV;
        goto exit;
    }

    pr_info("%s(): Region 1 length: %d \n", __FUNCTION__, pci_resource_len(dev, 1));

    bar_2_ptr = pcim_iomap(dev, 2, pci_resource_len(dev, 2));
    if (bar_2_ptr == NULL)
    {
        pr_err("%s(): Failed to map mem region 2.\n", __FUNCTION__);
        res = -ENODEV;
        goto exit;
    }

    pr_info("%s(): Region 2 length: %d \n", __FUNCTION__, pci_resource_len(dev, 2));

    /* 3. Test math operators. */
    iowrite32((u32)1, bar_0_ptr + REG_OP1);
    iowrite32((u32)2, bar_0_ptr + REG_OP2);
    iowrite32((u32)OPCODE_ADD, bar_0_ptr + REG_OPCODE);

    mdelay(1);

    pr_info("%s(): Read result from BAR1: %d\n",
            __FUNCTION__,
            ioread32(bar_0_ptr + REG_RESULT));

    /* 4. Expose our driver. */
    _dev._dev = dev;
    _dev._major = register_chrdev(0, DEVICE_NAME, &f_ops);
    if (_dev._major < 0)
    { // Registration failed.
        pr_alert("Registering char device failed with %d\n",  _dev._major);
        res = _dev._major;
        goto exit;
    }

    /* Create a struct class structure.
     * @owner: pointer to the module that is to `own` this struct class.
     * @name: pointer to a string for the name of this class.
     */
     _dev._cls = class_create(DEVICE_NAME);
    if (IS_ERR_OR_NULL(_dev._cls)) {
        res = PTR_ERR(_dev._cls);
        pr_err("%s(): Failed to create class: %d\n", __FUNCTION__, res);
        goto release_dev;
    }

    /* Creates a device and registers it with sysfs.
     * @class: pointer to the struct class that this device should be registered to.
     * @parent: pointer to the parent struct device of this new device, if any.
     * @devt: the dev_t for the char device to be added.
     * @drvdata: the data to be added to the device for callbacks.
     * @fmt: string for the device's name.
     * @...: variable arguments.
     */
    device_create(_dev._cls, NULL, MKDEV(_dev._major, 0), NULL, DEVICE_NAME);

    pr_info("Device created on /dev/%s.\n", DEVICE_NAME);

    return 0;

release_dev:
    unregister_chrdev(_dev._major, DEVICE_NAME);
exit:
    return res;
}

static void _remove(struct pci_dev *dev)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);
    device_destroy(_dev._cls, MKDEV(_dev._major, 0));
    class_destroy(_dev._cls);
    unregister_chrdev(_dev._major, DEVICE_NAME);
}

static struct pci_driver _driver = {
    .name = TYPE_PCI_CUSTOM_DEVICE,
    .probe = _probe,
    .remove = _remove,
    .id_table = dev_ids
};

static int _open(struct inode * inode, struct file *f)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);
    return 0;
}

static int _release(struct inode * inode, struct file *f)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);
    return 0;
}

static ssize_t _read(struct file *f, char __user *p, size_t size, loff_t *offset)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);
    return 0;
}

static ssize_t _write(struct file *f, const char __user *p, size_t size, loff_t *offset)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);
    return 0;
}

module_pci_driver(_driver);
MODULE_LICENSE("GPL");