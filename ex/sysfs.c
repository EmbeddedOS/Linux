/*
 * sysfs.c - Kernel module that create a variable accessible via sysfs.
 */

#include <linux/types.h>
#include <linux/printk.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>

#define OK 0
#define PROPERTY_NAME "larva_property"

static struct kobject *_module;

static int _property = 0;

static ssize_t _show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t _store(struct kobject *kobj, struct kobj_attribute *attr, char *buf, size_t count);

/* Defining the attribute.
 */
static struct kobj_attribute _attribute = __ATTR(_property, 0660, _show, (void *)_store);

static int __init _module_init(void)
{
    int res = OK;
    pr_info("%s(): invoked.\n", __FUNCTION__);

    /* Create a struct kobject dynamically and register it with sysfs.
     * @name: the name for the kobject.
     * @parent: the parent kobject of this kobject, if any.
     *
     * This function create a kobject structure dynamically and registers it
     * with sysfs. When u are finished with this structure, call `kobject_put()`
     * and the structure will be dynamically freed when it is no longer being used.
     * 
     * We are using kernel_kobj as parent property, so, this property will exist in
     * `/sys/kernel/` directory.
     * 
     * If  the kobject was not able to be create, NULL will be returned.
     */
    _module = kobject_create_and_add(PROPERTY_NAME, kernel_kobj);
    if (_module == NULL)
    {
        res = -ENOMEM;
        goto out;
    }

    /* Binding the attribute to the kobject.
     */
    res = sysfs_create_file(_module, &_attribute.attr);
    if (res != 0)
    {
        pr_err("Failed to create the property file.\n");
    }

out:
    return res;
}

static void __exit _module_exit(void)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);
    kobject_put(_module);
}

/* This callback is called when reading the property.
 * bash: `cat /sys/kernel/larva_property/_property`
 */
static ssize_t _show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", _property);
}

/* This callback is called when writing to the property.
 * bash: `echo 1 > /sys/kernel/larva_property/_property`
 */
static ssize_t _store(struct kobject *kobj, struct kobj_attribute *attr, char *buf, size_t count)
{
    sscanf(buf, "%du", &_property);
    return count;
}


module_init(_module_init);
module_exit(_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Larva");
MODULE_DESCRIPTION("Kernel module that create a variable accessible via sysfs.");