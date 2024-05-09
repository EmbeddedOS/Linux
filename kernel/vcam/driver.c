#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>

#include "log.h"
#include "driver.h"
#include "device.h"

static long vcam_ioctl(struct file *file,
                       unsigned int cmd,
                       unsigned long param);

typedef struct
{
    dev_t _dev_number;
    int _major;
    struct class *_class;
    struct device *_device;
    struct cdev _cdev;
    spinlock_t _device_access_lock;
    vcam_device_t **_devices;
    unsigned short _device_count;
} vcam_driver_t;

static vcam_driver_t *_drv = NULL;

static struct file_operations _fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = vcam_ioctl,
};

static vcam_driver_t *allocate_vcam_driver(void)
{
    vcam_driver_t *driver = (vcam_driver_t *)kmalloc(sizeof(vcam_driver_t), GFP_KERNEL);
    if (!driver)
    {
        goto exit;
    }

    memset(driver, 0, sizeof(vcam_driver_t));

    if (g_param_device_nodes)
    {
        driver->_devices = (vcam_device_t **)kmalloc(sizeof(vcam_device_t *) * g_param_device_nodes, GFP_KERNEL);
        if (!driver->_devices)
        {
            goto vcam_device_allocate_failure;
        }

        memset(driver->_devices, 0, sizeof(vcam_device_t *) * g_param_device_nodes);
    }

    return driver;

vcam_device_allocate_failure:
    kfree(driver);
    driver = NULL;
exit:
    return driver;
}

static void clean_vcam_driver(vcam_driver_t *driver)
{
    int i;

    if (driver)
    {
        for (i = 0; i < driver->_device_count; i++)
        {
            clean_vcam_device(driver->_devices[i]);
        }
        kfree(driver->_devices);

        device_destroy(driver->_class, driver->_dev_number);
        class_destroy(driver->_class);
        cdev_del(&driver->_cdev);
        unregister_chrdev_region(driver->_dev_number, 1);
        kfree(driver);
    }
}

int __init vcam_driver_init(const char *device_name)
{
    int res = 0;

    info("Initializing %s.", device_name);

    /* 1. Allocate driver memory. */
    _drv = allocate_vcam_driver();
    if (_drv == NULL)
    {
        res = -ENOMEM;
        error("Failed to allocate vcam driver.");
        goto allocate_failure;
    }

    /* 2. Create device class. */
    _drv->_class = class_create(THIS_MODULE, device_name);
    if (!_drv->_class)
    {
        error("Failed to create device class.");
        res = -ENODEV;
        goto class_creation_failure;
    }

    /* 3. Register file operations. */
    cdev_init(&_drv->_cdev, &_fops);

    /* 4. Allocate device number. */
    res = alloc_chrdev_region(&_drv->_dev_number, 0, 1, device_name);
    if (res < 0)
    {
        error("Failed to allocate device number.");
        goto alloc_chrdev_failure;
    }

    /* 5. Register device with its number. */
    res = cdev_add(&_drv->_cdev, _drv->_dev_number, 1);
    if (res < 0)
    {
        error("Device registration failure.");
        goto register_device_failure;
    }

    _drv->_device = device_create(_drv->_class, NULL, _drv->_dev_number,
                                  NULL, device_name, MINOR(_drv->_dev_number));
    if (!_drv->_device)
    {
        error("Failed to create device.");
        res = -ENODEV;
        goto device_creation_failure;
    }

    return 0;

device_creation_failure:
    cdev_del(&_drv->_cdev);
register_device_failure:
    unregister_chrdev_region(_drv->_dev_number, 1);
alloc_chrdev_failure:
    class_destroy(_drv->_class);
class_creation_failure:
    clean_vcam_driver(_drv);
allocate_failure:
    return res;
}

void vcam_driver_exit()
{
    info("Cleaning up virtual camera driver.");
    clean_vcam_driver(_drv);
}

static long vcam_ioctl(struct file *file,
                       unsigned int cmd,
                       unsigned long param)
{
    return 0;
}
