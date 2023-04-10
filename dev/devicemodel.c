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

static int __init _device_model_init(void)
{
    int res = 0;
    pr_info("%s(): invoked.\n", __FUNCTION__);

    return res;
}

static void __exit _device_model_exit(void)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);
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