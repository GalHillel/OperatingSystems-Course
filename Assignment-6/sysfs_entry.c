#include <linux/sysfs.h>
#include <linux/device.h>
#include "char_device.h"

static ssize_t registered_bytes_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct char_device_data *data = dev_get_drvdata(dev);
    return snprintf(buf, PAGE_SIZE, "%ld\n", data->registered_bytes);
}

static DEVICE_ATTR_RO(registered_bytes);

static struct attribute *sysfs_entries[] = {
    &dev_attr_registered_bytes.attr,
    NULL,
};

static struct attribute_group attr_group = {
    .attrs = sysfs_entries,
};

static struct class_attribute *class_attrs[] = {
    NULL,
};

static struct class dev_class = {
    .name = DEVICE_NAME,
    .dev_groups = &attr_group,
    .class_attrs = class_attrs,
};

static int __init sysfs_entry_init(void)
{
    int result = class_register(&dev_class);
    if (result < 0)
    {
        printk(KERN_ALERT "Failed to register sysfs class\n");
        return result;
    }

    printk(KERN_INFO "Sysfs entry initialized\n");
    return 0;
}

static void __exit sysfs_entry_exit(void)
{
    class_unregister(&dev_class);
    printk(KERN_INFO "Sysfs entry exited\n");
}

module_init(sysfs_entry_init);
module_exit(sysfs_entry_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gal");
MODULE_DESCRIPTION("Sysfs Entry for Encrypted Memory");
