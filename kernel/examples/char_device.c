/* char_device.c: Create a read-only char device that says 
 * how many times u have read from the dev file.
 */
#include <linux/types.h>
#include <linux/printk.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h> /* For get_user and put_user. */
#include <linux/kernel.h> /* For sprintf(). */
#include <linux/atomic.h>

#include <asm/errno.h>

struct inode;
struct file;
struct file_operations;


#define OK 0
#define DEVICE_NAME "larva-dev" /* Dev name as it appears in /proc/devices */
#define DEV_NOT_USED 0
#define DEV_IS_OPEN 1

static atomic_t _already_open = ATOMIC_INIT(DEV_NOT_USED); /* Is device open? Used to prevent multiple access to device. */
static int _major; /* major number assigned to our device driver */
static struct class *_cls;
static int _open_counter = 0;
char _msg[100];



static int _open(struct inode * inode, struct file *f);
static int _release(struct inode * inode, struct file *f);
static ssize_t _read(struct file *f, char __user *p, size_t size, loff_t *offset);
static ssize_t _write(struct file *f, const char __user *p, size_t size, loff_t *offset);

static struct file_operations _fops = {
    .read = _read,
    .write = _write,
    .open = _open,
    .release = _release,
};


static int __init _module_init(void)
{
    int res = OK;
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

    /* Create a struct class structure.
     * @owner: pointer to the module that is to `own` this struct class.
     * @name: pointer to a string for the name of this class.
     */
    _cls = class_create(THIS_MODULE, DEVICE_NAME);
    
    /* Creates a device and registers it with sysfs.
     * @class: pointer to the struct class that this device should be registered to.
     * @parent: pointer to the parent struct device of this new device, if any.
     * @devt: the dev_t for the char device to be added.
     * @drvdata: the data to be added to the device for callbacks.
     * @fmt: string for the device's name.
     * @...: variable arguments.
     */
    device_create(_cls, NULL, MKDEV(_major, 0), NULL, DEVICE_NAME);

    pr_info("Device created on /dev/%s.\n", DEVICE_NAME);

out:
    return res;
}

static void __exit _module_exit(void)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);
    device_destroy(_cls, MKDEV(_major, 0));
    class_destroy(_cls);
    unregister_chrdev(_major, DEVICE_NAME);
    pr_info("Unregister the device on /dev/%s.\n", DEVICE_NAME);

}

static int _open(struct inode * inode, struct file *f)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);

    /* Atomic compare and exchange.
     * @uaddr: The address of the atomic_t to be modified.
     * @oldval: The expected value of the atomic_t.
     * @newval: The new value to try and assign the atomic_t.
     * 
     * Return the old value of addr->val.
     */
    if (atomic_cmpxchg(&_already_open, DEV_NOT_USED, DEV_IS_OPEN))
    {
        return -EBUSY;
    }

    _open_counter++;

    /* Increment the reference count of current module. */
    try_module_get(THIS_MODULE);

    return OK;
}

static int _release(struct inode * inode, struct file *f)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);

    /* Atomic set.
     * @addr: Address of the variable to set.
     * @newval:	New value for the atomic_t.
     * Return the new value of addr->val.
     */
    atomic_set(&_already_open, DEV_NOT_USED);

    /* Decrement the reference count of current module. */
    module_put(THIS_MODULE);
    
    return OK;
}

static ssize_t _read(struct file *f, char __user *p, size_t size, loff_t *offset)
{
    int bytes_read = 0;
    const char* msg = NULL;

    pr_info("%s(): invoked.\n", __FUNCTION__);

    sprintf(_msg, "Device is open %d times.\n", _open_counter);
    msg = _msg;

    if (!*(msg + *offset))
    { // Is end of file.
        *offset = 0;
        return OK;
    }

    msg += *offset;

    while (size && *msg)
    {

        /* put_user - Write a simple value into user space.
         * @x: Value to copy to user space.
         * @ptr: Destination address, in user space.
         * 
         * This macro copies a single simple value from kernel space to user
         * space. It supports simple types like char and int, but not larger
         * data types like structures or arrays.
         * 
         * @ptr must have pointer-to-simple-variable type, and
         * @x must be assignable to the result of dereferencing @ptr.
         * 
         * Return: zero on success, or -EFAULT on error.
         */

        put_user(*(msg++), p++);
        size--;
        bytes_read++;
    }

    *offset += bytes_read;

    return bytes_read;
}

static ssize_t _write(struct file *f, const char __user *p, size_t size, loff_t *offset)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);
    return OK;
}

module_init(_module_init);
module_exit(_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Larva");
MODULE_DESCRIPTION("Read-only character device.");