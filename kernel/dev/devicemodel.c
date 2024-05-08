/* devicemodel.c - template for device model.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>

#define DEVICE_NAME "larva-dev" /* Dev name as it appears in /dev/ directory */

struct larva_platform_data {
    unsigned int size;
    const char *serial;
    int perm;
};


static int _open(struct inode * inode, struct file *f);
static int _release(struct inode * inode, struct file *f);
static ssize_t _read(struct file *f, char __user *p, size_t size, loff_t *offset);
static ssize_t _write(struct file *f, const char __user *p, size_t size, loff_t *offset);
static int _probe(struct platform_device *plf_dev);
static int _remove(struct platform_device *plf_dev);
static int _suspend(struct device *dev);
static int _resume(struct device *dev);


struct of_device_id _larvadev_dt_match[] = 
{
	{.compatible = "larvadev-A1x"},
	{.compatible = "larvadev-B1x"},
	{.compatible = "larvadev-C1x"},
	{.compatible = "larvadev-D1x"},
	{} //Null termination.
};

/* Device power management operations - device PM callbacks.
 * @prepare: This principal role of this callback is to prevent new children
 *      of the device from being registered after it has returned. If @prepare()
 *      detects a situation it cannot handle (e.g. registration of a child already
 *      in progress), it may return -EAGAIN, so that the pm core can execute
 *      it once again.
 * @complete: Undo the changes made by @prepare(). This method is executed for all
 *      kinds of resume transitions, following one of the resume callbacks: @resume(),
 *      @thaw(), @restore().
 * @suspend: Executed before putting the system into a sleep state in which the contents
 *      of main memory are preserved.
 * @resume: Executed after waking the system up from a sleep state in which contents
 *      of main memory were preserved.
 * @freeze: Hibernation-specific, executed before creating a hibernation image.
 * @thaw: Hibernation-specific, executed after creating a hibernation image OR if the
 *      creation of an image has failed.
 * @poweroff: Hibernation-specific, executed after saving a hibernation image.
 * @restore: Hibernation-specific, executed after restoring the contents of main
 *      memory from a hibernation image, analogous to @resume()
 */
static const struct dev_pm_ops _dev_ops = {
    .suspend = _suspend,
    .resume = _resume,
    .poweroff = _suspend,
    .freeze = _suspend,
    .thaw = _resume,
    .restore = _resume
};

static struct platform_driver _driver = {
    .driver = {
        .name = "larva driver",
        .pm = &_dev_ops, /* Power management operations of the device which matched this driver. */
        .of_match_table = of_match_ptr(_larvadev_dt_match), /* The open firmware table.
                                                               This struct used for matching a device. */
    },

    .probe = _probe, /* Called to query the existence of a specific device.
                        whether this driver can work with it, and bind the driver
                        to a specific. */
    .remove = _remove /* Called when the device is removed from the system to unbind
                         a device from this driver. */
};

static struct file_operations _fops = {
    .read = _read,
    .write = _write,
    .open = _open,
    .release = _release,
};

static int _major; /* major number assigned to our device driver. */
static struct class *_cls;

static int __init _device_model_init(void)
{
    int res = 0;
    pr_info("%s(): invoked.\n", __FUNCTION__);

    res = platform_driver_register(&_driver);
    if (res != 0)
    {
        pr_err("%s(): Failed to register platform driver.\n", __FUNCTION__);
    }

    return res;
}

static void __exit _device_model_exit(void)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);
    platform_driver_unregister(&_driver);
}

static int _probe(struct platform_device *plf_dev)
{
    int res = 0;

    /* 1. First we probe the device to see it matches the driver or not. */
   	struct device *dev = &plf_dev->dev;
    struct device_node *dev_node = dev->of_node;

    /* devm_kzalloc - resource managed kzalloc.
     * @dev: Device to allocate memory for.
     * @size: Allocation size.
     * @gfp: Allocation GFP flags.
     * 
     * Managed kzalloc. Memory allocated with this function is automatically 
     * freed on driver detach.
     */
    struct larva_platform_data* device_data = devm_kzalloc(dev,
                                                            sizeof(struct larva_platform_data),
                                                            GFP_KERNEL); 

    if (device_data == NULL)
    {
        res = -ENOMEM;
        goto out;
    }

    /* 1.1. Get device properties that defined in device tree file. */
    if (of_property_read_string(dev_node, "org,device-serial-num", &device_data->serial))
    {
        pr_alert(" Missing serial number property.\n");
        res = -EINVAL;
        goto out;
    }

    if (of_property_read_u32(dev_node, "org,size", &device_data->size))
    {
        pr_alert(" Missing size property.\n");
        res = -EINVAL;
        goto out;
    }

    if (of_property_read_u32(dev_node, "org,perm", &device_data->perm))
    {
        pr_alert(" Missing permission property.\n");
        res = -EINVAL;
        goto out;
    }

    /* After get properties successfully, we can read them, compare them with some config,
     * or do some thing to validate the device.
     */


    /* save the device private data pointer in platform device structure. */
    dev_set_drvdata(dev,(void*)device_data);

	dev_info(dev, "Device serial number = %s. \n", device_data->serial);
	dev_info(dev, "Device permission = %d. \n", device_data->perm);
	dev_info(dev, "Device size = %d. \n", device_data->size);

    /* 2. If it is compatible, we will make a new device file for it. */
    _major = register_chrdev(0, DEVICE_NAME, &_fops);
    if (_major < 0)
    { // Registration failed.
        pr_alert("Registering char device failed with %d\n", _major);
        res = _major;
        goto out;
    }

    _cls = class_create(THIS_MODULE, DEVICE_NAME);

    device_create(_cls, NULL, MKDEV(_major, 0), NULL, DEVICE_NAME);

    pr_info("Device created on /dev/%s.\n", DEVICE_NAME);

out:
    return res;
}

static int _remove(struct platform_device *plf_dev)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);

    device_destroy(_cls, MKDEV(_major, 0));
    class_destroy(_cls);
    unregister_chrdev(_major, DEVICE_NAME);
    pr_info("Unregister the device on /dev/%s.\n", DEVICE_NAME);

    return 0;
}

static int _suspend(struct device *dev)
{
    return 0;
}

static int _resume(struct device *dev)
{
    return 0;
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

module_init(_device_model_init);
module_exit(_device_model_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Larva");
MODULE_DESCRIPTION("Linux Device Model.");