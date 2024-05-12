#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "driver.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Larva");
MODULE_DESCRIPTION("Virtual V4L2 Camera Driver");

#define DEVICE_NAME "vcam"

unsigned char g_param_enable_scaling = 0;
unsigned short g_param_device_nodes = 0;

module_param(g_param_enable_scaling, byte, 0);
module_param(g_param_device_nodes, ushort, 0);

MODULE_PARM_DESC(g_param_enable_scaling, "Enable image scaling by default.\n");
MODULE_PARM_DESC(g_param_device_nodes, "Maximum number of device nodes.\n");

static int __init vcam_init(void)
{
    return vcam_driver_init(DEVICE_NAME);
}

static void __exit vcam_exit(void)
{
    vcam_driver_exit();
}

module_init(vcam_init);
module_exit(vcam_exit);