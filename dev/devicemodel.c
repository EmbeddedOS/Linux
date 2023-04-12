/* devicemodel.c - template for device model.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>

struct larva_platform_data {
    unsigned int size;
    char *serial;
    int perm;
};

struct larva_device_config {
    // We define some config to match compatible device.
    int config_1;
    int config_2;
};

enum larvadev_identifier {
    LARVADEVA1X = 0,
    LARVADEVB1X = 1,
    LARVADEVC1X = 2,
    LARVADEVD1X = 3
};

static int _probe(struct platform_device *dev);
static int _remove(struct platform_device *dev);
static int _suspend(struct device *dev);
static int _resume(struct device *dev);

struct larva_device_config _larvadev_config[] = {
    [LARVADEVA1X] = {.config_1 = 10, .config_2 = 11},
    [LARVADEVB1X] = {.config_1 = 12, .config_2 = 13},
    [LARVADEVC1X] = {.config_1 = 14, .config_2 = 15},
    [LARVADEVD1X] = {.config_1 = 16, .config_2 = 17},
};

struct of_device_id _larvadev_dt_match[] = 
{
	{.compatible = "larvadev-A1x", .data = (void*)LARVADEVA1X},
	{.compatible = "larvadev-B1x", .data = (void*)LARVADEVB1X},
	{.compatible = "larvadev-C1x", .data = (void*)LARVADEVC1X},
	{.compatible = "larvadev-D1x", .data = (void*)LARVADEVD1X},
	{} //Null termination.
};

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
        .pm = &_dev_ops,
        .of_match_table = of_match_ptr(_larvadev_dt_match),
    },

    .probe = _probe,
    .remove = _remove
};

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
    /* 1. First we probe the device to see it matches the driver or not. */
    

    /* 2. If it is compatible, we will a device file for it. */
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