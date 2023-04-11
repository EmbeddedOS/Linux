/* devicemodel.c - template for device model.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>

struct private_data {
    int number;
    void* p;
};

static int _probe(struct platform_device *dev);
static int _remove(struct platform_device *dev);
static int _suspend(struct device *dev);
static int _resume(struct device *dev);

/* Device power management operations - device PM callbacks.
 * @prepare:
 * @complete:
 * @suspend:
 * @resume:
 * @freeze:
 * @thaw:
 * @poweroff
 * @restore:
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
        .pm = &_dev_ops
    }

    .probe = _probe,
    .remove = _remove
}

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

static int _probe(struct platform_device *dev)
{
    return 0;
}

static int _remove(struct platform_device *dev)
{
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


module_init(_device_model_init);
module_exit(_device_model_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Larva");
MODULE_DESCRIPTION("Linux Device Model.");