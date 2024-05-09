#pragma once
#include <linux/printk.h>

#define info(fmt, arg...) pr_info("%s(): " fmt"\n", __FUNCTION__, ##arg)
#define error(fmt, arg...) pr_err("%s(): " fmt"\n", __FUNCTION__, ##arg)
#define debug(fmt, arg...) pr_debug("%s(): " fmt"\n", __FUNCTION__, ##arg)

