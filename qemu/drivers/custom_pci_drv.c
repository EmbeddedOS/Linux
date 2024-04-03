#include <linux/init.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/delay.h>

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

static struct pci_device_id dev_ids[] = {
    {PCI_DEVICE(DEVICE_VENDOR_ID, DEVICE_DEVICE_ID)},
    {}
};

/* Use this macro for matching device. */
MODULE_DEVICE_TABLE(pci, dev_ids);

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


exit:
    return res;
}

static void _remove(struct pci_dev *dev)
{
    pr_info("%s()\n", __FUNCTION__);
}

static struct pci_driver _driver = {
    .name = TYPE_PCI_CUSTOM_DEVICE,
    .probe = _probe,
    .remove = _remove,
    .id_table = dev_ids
};

module_pci_driver(_driver);
MODULE_LICENSE("GPL");