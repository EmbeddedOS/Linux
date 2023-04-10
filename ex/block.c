/* block.c - 
 */

#include <linux/types.h>
#include <linux/printk.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/version.h>
#include <linux/uaccess.h>
#include <linux/wait.h> /* For putting processes to sleep and waking them up. */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
#define HAVE_PROC_OPS
#endif

#define PROCFS_NAME "larva-dev"
#define OK 0

static int _open(struct inode * inode, struct file *f);
static int _release(struct inode * inode, struct file *f);
static ssize_t _read(struct file *f, char __user *p, size_t size, loff_t *offset);
static ssize_t _write(struct file *f, const char __user *p, size_t size, loff_t *offset);

static struct proc_dir_entry *_proc_file;

static DECLARE_WAIT_QUEUE_HEAD(waitq);

#ifdef HAVE_PROC_OPS
static struct proc_ops _fops = {
    .proc_read = _read,
    .proc_write = _write,
    .proc_open = _open,
    .proc_release = _release,
};
#else
static struct file_operations _fops = {
    .read = _read,
    .write = _write,
    .open = _open,
    .release = _release,
};
#endif

static int __init _block_init(void)
{
    int res = OK;
    pr_info("%s(): invoked.\n", __FUNCTION__);

    _proc_file = proc_create(PROCFS_NAME, 0644, NULL, &_fops);
    if (_proc_file == NULL)
    {
        pr_alert("ERROR: failed to create /proc/%s file.\n", PROCFS_NAME);
        res = -ENOMEM;
        goto out;
    }

    /* Set the `/proc` file size.*/
    proc_set_size(_proc_file, 100);

    /* Set the `/proc` file owner as root.*/
    proc_set_user(_proc_file, GLOBAL_ROOT_UID, GLOBAL_ROOT_GID);

    pr_info("/proc/%s created.\n", PROCFS_NAME);

out:
    return OK;
}

static void __exit _block_exit(void)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);
    proc_remove(_proc_file);
}

static int _open(struct inode * inode, struct file *f)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);

    try_module_get(THIS_MODULE);

    return OK;
}

static int _release(struct inode * inode, struct file *f)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);

    module_put(THIS_MODULE);

    return OK;
}

static ssize_t _read(struct file *f, char __user *p, size_t size, loff_t *offset)
{
    size_t res = 0;


    pr_info("%s(): invoked.\n", __FUNCTION__);

    char msg[20] = "I am Larva!\n";
    int msg_len = sizeof(msg);

    if (*offset >= msg_len || copy_to_user(p, msg, msg_len))
    {
        *offset = 0;
    } else
    {
        *offset += msg_len;
        res = msg_len;
        pr_info("read: %s", f->f_path.dentry->d_name.name);
    }

    return res;
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

    pr_info("User written: %s.\n", msg);

    *offset += size;

    return size;
}

module_init(_block_init);
module_exit(_block_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Larva");
MODULE_DESCRIPTION("Kernel module that create a file in `/proc`.");