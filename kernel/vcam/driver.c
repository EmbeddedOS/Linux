#include <linux/cdev.h>

#include "log.h"
#include "driver.h"
#include "device.h"

typedef struct
{
    dev_t _dev_number;
    int _major;
    struct class *_class;
    struct cdev _cdev;
    spinlock_t _device_access_lock;

    vcam_device_t **_devices;
    unsigned short _device_count;
} vcam_driver_t;

static vcam_driver_t *_drv = NULL;

static vcam_driver_t *allocate_vcam_driver(void)
{
    vcam_driver_t *driver = (vcam_driver_t *)kmalloc(sizeof(vcam_driver_t), GFP_KERNEL);
    if (!driver)
    {
        goto exit;
    }

exit:
    return driver;
}

static void free_vcam_driver(void)
{
    if (_drv)
    {
    }
}

int __init vcam_driver_init(const char *device_name)
{
    int res = 0;

    info("Initializing %s.", device_name);
    _drv = allocate_vcam_driver();
    if (_drv == NULL)
    {
        res = -ENOMEM;
        goto exit;
    }

exit:
    return res;
}

void vcam_driver_exit()
{
    info("Cleaning up virtual camera driver.");
}