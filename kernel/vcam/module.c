#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Larva");
MODULE_DESCRIPTION("Virtual V4L2 Camera Driver");

unsigned char g_enable_scaling = 0;

module_param(g_enable_scaling, byte, 0);
MODULE_PARM_DESC(g_enable_scaling, "Enable image scaling by default.\n");

static int __init vcam_init(void)
{
    return 0;
}

static void __exit vcam_exit(void)
{

}

module_init(vcam_init);
module_exit(vcam_exit);