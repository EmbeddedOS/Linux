/* vinput.c - Virtual input device.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <asm/errno.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/input.h>
#include <linux/slab.h>

#define DEVICE_NAME "larva-dev"

static int _open(struct inode * inode, struct file *f);
static int _release(struct inode * inode, struct file *f);
static ssize_t _read(struct file *f, char __user *p, size_t size, loff_t *offset);
static ssize_t _write(struct file *f, const char __user *p, size_t size, loff_t *offset);

static int _major; /* major number assigned to our device driver */


static struct attribute *_vinput_cls_attrs[] = {
    NULL
};

/* This macro make a attribute_group object with name name##_group.
 * This macro is declared in file:
 * `/lib/modules/(uname -r)/build/include/linux/sysfs.h`
 */
ATTRIBUTE_GROUPS(_vinput_cls);

static struct class _cls = {
    .name = "vinput",
    .owner = THIS_MODULE,
    .class_groups = _vinput_cls_group /* Using our attribute_group object. */
};

static struct file_operations _fops = {
    .read = _read,
    .write = _write,
    .open = _open,
    .release = _release,
};

static int __init _vinput_init(void)
{
    int res = 0;
    pr_info("%s(): invoked.\n", __FUNCTION__);

    /* 1. Registering the device with the kernel. */

    /* Create and register a cdev.
     * @major: major device number or 0 for dynamic allocation.
     * @name: name of this range of devices.
     * @fops: file operations associated with this devices.
     */
    _major = register_chrdev(0, DEVICE_NAME, &_fops);
    if (_major < 0)
    { // Registration failed.
        pr_alert("Registering char device failed with %d\n", _major);
        res = _major;
        goto out;
    }

    res = class_register(&_cls);
    if (res < 0)
    {
        pr_err("%s(): Failed to register virtual input class.\n", __FUNCTION__);
        goto out2;
    }

out2:
    class_unregister(&_cls);
    unregister_chrdev(_major, DEVICE_NAME);
out:
    return res;
}

static void __exit _vinput_exit(void)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);
    class_unregister(&_cls);
    unregister_chrdev(_major, DEVICE_NAME);
}

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

module_init(_vinput_init);
module_exit(_vinput_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Larva");
MODULE_DESCRIPTION("Virtual input device.");
