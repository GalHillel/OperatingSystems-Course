#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/ioctl.h>
#include "ioctl_commands.h"
#include "encryption.h"
#include "char_device.h"

#define DEVICE_NAME "encrypted_memory"

static dev_t dev_num;
static struct class *dev_class;
static struct cdev cdev;
static char *memory_buffer;
static int key;

static int device_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
    return 0;
}

static ssize_t device_read(struct file *file, char __user *buffer, size_t length, loff_t *offset)
{
    int bytes_to_read = length;

    if (bytes_to_read > PAGE_SIZE)
        bytes_to_read = PAGE_SIZE;

    if (copy_to_user(buffer, memory_buffer + *offset, bytes_to_read))
        return -EFAULT;

    *offset += bytes_to_read;
    return bytes_to_read;
}

static ssize_t device_write(struct file *file, const char __user *buffer, size_t length, loff_t *offset)
{
    int bytes_to_write = length;

    if (bytes_to_write > PAGE_SIZE)
        bytes_to_write = PAGE_SIZE;

    if (copy_from_user(memory_buffer + *offset, buffer, bytes_to_write))
        return -EFAULT;

    encrypt_memory(memory_buffer + *offset, bytes_to_write, key);

    *offset += bytes_to_write;
    return bytes_to_write;
}

static long device_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
{
    int new_key;

    switch (ioctl_num)
    {
    case IOCTL_SET_KEY:
        new_key = ioctl_param;
        if (new_key != key)
        {
            decrypt_memory(memory_buffer, PAGE_SIZE, key);
            key = new_key;
            encrypt_memory(memory_buffer, PAGE_SIZE, key);
        }
        break;
    default:
        return -EINVAL;
    }

    return 0;
}

static struct file_operations fops = {
    .open = device_open,
    .release = device_release,
    .read = device_read,
    .write = device_write,
    .unlocked_ioctl = device_ioctl,
};

static int __init char_device_init(void)
{
    if (alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME) < 0)
    {
        printk(KERN_ALERT "Failed to allocate character device region\n");
        return -1;
    }

    if ((dev_class = class_create(THIS_MODULE, DEVICE_NAME)) == NULL)
    {
        printk(KERN_ALERT "Failed to create the device class\n");
        unregister_chrdev_region(dev_num, 1);
        return -1;
    }

    if (device_create(dev_class, NULL, dev_num, NULL, DEVICE_NAME) == NULL)
    {
        printk(KERN_ALERT "Failed to create the device\n");
        class_destroy(dev_class);
        unregister_chrdev_region(dev_num, 1);
        return -1;
    }

    cdev_init(&cdev, &fops);
    if (cdev_add(&cdev, dev_num, 1) == -1)
    {
        printk(KERN_ALERT "Failed to add the character device\n");
        device_destroy(dev_class, dev_num);
        class_destroy(dev_class);
        unregister_chrdev_region(dev_num, 1);
        return -1;
    }

    memory_buffer = kmalloc(PAGE_SIZE, GFP_KERNEL);
    if (!memory_buffer)
    {
        printk(KERN_ALERT "Failed to allocate memory buffer\n");
        cdev_del(&cdev);
        device_destroy(dev_class, dev_num);
        class_destroy(dev_class);
        unregister_chrdev_region(dev_num, 1);
        return -1;
    }

    key = 0;
    encrypt_memory(memory_buffer, PAGE_SIZE, key);

    printk(KERN_INFO "Char device initialized\n");
    return 0;
}

static void __exit char_device_exit(void)
{
    kfree(memory_buffer);
    cdev_del(&cdev);
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    unregister_chrdev_region(dev_num, 1);
    printk(KERN_INFO "Char device exited\n");
}

module_init(char_device_init);
module_exit(char_device_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gal");
MODULE_DESCRIPTION("Char Device for Encrypted Memory");
