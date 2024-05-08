/* ioctl.c: Create an input/output character device.
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

/* Define ioctl numbers.
 * @_IO    an ioctl with no parameters.
 * @_IOW   an ioctl with write parameters (copy_from_user).
 * @_IOR   an ioctl with read parameters (copy_to_user).
 * @_IOWR  an ioctl with both write and read parameters.
 * 
 * 'Write' and 'read' are from the user's point of view, just like the
 * system calls 'write' and 'read'.
 * 
 * The first argument to _IO, _IOW, _IOR, or _IOWR is an identifying letter
 * or number from the table in ref below. Because of the large number of drivers,
 * many drivers share a partial letter with other drivers.
 *
 * If you are writing a driver for a new device and need a letter, pick an
 * unused block with enough room for expansion: 32 to 256 ioctl commands.
 *
 * Ref: https://www.kernel.org/doc/Documentation/ioctl/ioctl-number.txt
 */
#define LAVA_MAGIC_NUMBER '\x66'
#define IOCTL_GET_NUM _IOR(LAVA_MAGIC_NUMBER, 1, int)
#define IOCTL_SET_NUM _IOW(LAVA_MAGIC_NUMBER, 2, int)

#define OK 0
#define DEVICE_NAME "larva-ioctl" /* Dev name as it appears in /proc/devices */
#define DEV_NOT_USED 0
#define DEV_IS_OPEN 1

static int _major; /* major number assigned to our device driver */
static struct class *_cls;

struct private_data
{
    int data;
    rwlock_t lock;
};


static int _open(struct inode * inode, struct file *f);
static int _release(struct inode * inode, struct file *f);
static ssize_t _read(struct file *f, char __user *p, size_t size, loff_t *offset);
static ssize_t _write(struct file *f, const char __user *p, size_t size, loff_t *offset);
static long _unlocked_ioctl(struct file *f, unsigned int number, unsigned long val);

static struct file_operations _fops = {
    .owner = THIS_MODULE,
    .read = _read,
    .write = _write,
    .open = _open,
    .release = _release,
    .unlocked_ioctl = _unlocked_ioctl
};


static int __init _module_init(void)
{
    int res = OK;
    pr_info("%s(): invoked.\n", __FUNCTION__);

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
    pr_info("%s(%p, %p): invoked.\n", __FUNCTION__, inode, f);

    /* The GFP flags control the allocators behavior.
     * Most of the time GFP_KERNEL is what you need.
     * Memory for the kernel data structures, DMAable memory,
     * inode cache, all these and many other allocations
     * types can use GFP_KERNEL.
     */
    struct private_data *data = kmalloc(sizeof(struct private_data), GFP_KERNEL);

    if (data == NULL)
    {
        return -ENOMEM;
    }

    rwlock_init(&data->lock);
    data->data = 231199;

    f->private_data = data;
    return OK;
}

static int _release(struct inode * inode, struct file *f)
{
    pr_info("%s(%p, %p): invoked.\n", __FUNCTION__, inode, f);

    if (f->private_data != NULL)
    {
        kfree(f->private_data);
        f->private_data = NULL;
    }

    return OK;
}

static ssize_t _read(struct file *f, char __user *p, size_t size, loff_t *offset)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);

    int bytes_read = 0;
    struct private_data* data = (struct private_data*) f->private_data;

    int val = 0;

    read_lock(&data->lock);
    val = data->data;
    read_unlock(&data->lock);

    char msg[20];
    sprintf(msg, "%d", val);
    int msg_len = sizeof(msg);

    if (*offset >= msg_len || copy_to_user(p, msg, msg_len))
    {
        *offset = 0;
    } else
    {
        *offset += msg_len;
        bytes_read = msg_len;
    }

    return bytes_read;
}

static ssize_t _write(struct file *f, const char __user *p, size_t size, loff_t *offset)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);

    char msg[size+1];
    
    if (copy_from_user(msg, p, size))
    {
        return -EFAULT;
    }

    msg[size] = '\0';

    int usr_data = 0;

    sscanf(msg, "%d", &usr_data);

    struct private_data* data = (struct private_data*) f->private_data;

    write_lock(&data->lock);
    data->data = usr_data;
    write_unlock(&data->lock);


    *offset += size;

    return size;
}

static long _unlocked_ioctl(struct file *f, unsigned int number, unsigned long val)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);
    int res = OK;

    struct private_data* data = (struct private_data*) f->private_data;

    switch (number)
    {
    case IOCTL_GET_NUM:
        pr_info("%s(): run command: %d.\n", __FUNCTION__, IOCTL_GET_NUM);

        int private_val = 0;

        read_lock(&data->lock);
        private_val = data->data;
        read_unlock(&data->lock);

        if (copy_to_user((int __user *)val, &private_val, sizeof(private_val)))
        {
            res = -EFAULT;
            goto out;
        }

        break;
    case IOCTL_SET_NUM:
        pr_info("%s(): run command: %d.\n", __FUNCTION__, IOCTL_SET_NUM);

        int usr_data = 0;
        
        if (copy_from_user(&usr_data, (int __user *)val, sizeof(usr_data)))
        {
            res =  -EFAULT;
            goto out;
        }

        write_lock(&data->lock);
        data->data = usr_data;
        write_unlock(&data->lock);

        break;

    default:
        res = -ENOTTY;
    }

out:
    return OK;
}



module_init(_module_init);
module_exit(_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Larva");
MODULE_DESCRIPTION("Read-only character device.");