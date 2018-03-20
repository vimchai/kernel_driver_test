#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/device.h>
#include <asm/uaccess.h>

#include "hello.h"
#include "hello_test.h"

static int hello_major = 0;
static int hello_minor = 0;
static struct class* hello_class = NULL;
static struct hello_dev* hello_dev = NULL;

static ssize_t __hello_get_val(struct hello_dev* dev, char* buf) {
    int val = 0;

    mutex_lock(&dev->mutex);
    val = dev->val;
    mutex_unlock(&dev->mutex);

    return snprintf(buf, PAGE_SIZE, "%d\n", val);
}

static ssize_t __hello_set_val(struct hello_dev* dev, const char* buf, size_t count) {
    int val = 0;

    val = simple_strtol(buf, NULL, 10);

    mutex_lock(&dev->mutex);

    dev->val = val;
    mutex_unlock(&dev->mutex);

    return count;
}


static ssize_t hello_val_show(struct device* dev, struct device_attribute* attr, char* buf) {
    struct hello_dev* hdev = (struct hello_dev*)dev_get_drvdata(dev);

    return __hello_get_val(hdev, buf);
}


static ssize_t hello_val_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t count) {
    struct hello_dev* hdev = (struct hello_dev*)dev_get_drvdata(dev);

    return __hello_set_val(hdev, buf, count);
}

static DEVICE_ATTR(val, S_IRUGO | S_IWUSR, hello_val_show, hello_val_store);

static int hello_open(struct inode* inode, struct file* filp) {
    struct hello_dev *dev;

    dev = container_of(inode->i_cdev, struct hello_dev, dev);
    filp->private_data = dev;

    return 0;
}

static int hello_release(struct inode* inode, struct file* filp) {
    return 0;
}

static ssize_t hello_read(struct file* filp, char __user *buf, size_t count, loff_t* f_pos) {
    ssize_t err = 0;
    struct hello_dev* dev = filp->private_data;

    mutex_lock(&dev->mutex);
    printk(KERN_ALERT "%s count:%lu, fpos:%lld, %ld\n", __func__, count, *f_pos, sizeof(dev->val));

    if (*f_pos >= sizeof(dev->val))
        goto out;

    printk(KERN_ALERT "%s count:%lu, val:%d\n", __func__, count, dev->val);

    if(copy_to_user(buf, &(dev->val), sizeof(dev->val))) {
        err = -EFAULT;
        goto out;
    }

    *f_pos += sizeof(dev->val);

    err = sizeof(dev->val);
out:
    mutex_unlock(&dev->mutex);
    return err;
}

static ssize_t hello_write(struct file* filp, const char __user *buf, size_t count, loff_t* f_pos) {
    struct hello_dev* dev = filp->private_data;
    ssize_t err = 0;

    printk(KERN_ALERT "%s count:%lu\n", __func__, count);
    mutex_lock(&dev->mutex);
    if(count > sizeof(dev->val)) {
        goto out;
    }

    if(copy_from_user(&(dev->val), buf, count)) {
        err = -EFAULT;
        goto out;
    }

    err = sizeof(dev->val);

out:
    mutex_unlock(&dev->mutex);
    return err;
}

static struct file_operations hello_fops = {
    .owner = THIS_MODULE,
    .open = hello_open,
    .release = hello_release,
    .read = hello_read,
    .write =  hello_write,
    .mmap = hello_mmap,

};
static int hello_init(void)
{
    int err = 0;
    dev_t dev;
    struct device *temp;

    err = alloc_chrdev_region(&dev, 0, 1, HELLO_DEV_NAME);
    if (err < 0) {
        pr_err("alloc dev number fail\n");
        goto fail;
    }

    hello_dev = kmalloc(sizeof(struct hello_dev), GFP_KERNEL);
    if(!hello_dev) {
        err = -ENOMEM;
        printk(KERN_ALERT"Failed to alloc hello_dev.\n");
        goto unregister;
    }
    hello_major = MAJOR(dev);
    hello_minor = MINOR(dev);
    mutex_init(&hello_dev->mutex);
    cdev_init(&hello_dev->dev, &hello_fops);
    err = cdev_add(&hello_dev->dev, dev, 1);
    if (err)
        goto kfree;

    hello_class = class_create(THIS_MODULE,  HELLO_DEV_CLASS);
    if(IS_ERR(hello_class)) {
        err = PTR_ERR(hello_class);
        printk(KERN_ALERT"Failed to create hello class.\n");
        goto destroy_cdev;
    }

    temp = device_create(hello_class, NULL, dev, "%s", HELLO_DEV_NAME);
    if(IS_ERR(temp)) {
        err = PTR_ERR(temp);
        printk(KERN_ALERT"Failed to create hello device.");
        goto destroy_class;
    }

    err = device_create_file(temp, &dev_attr_val);
    if(err < 0) {
        printk(KERN_ALERT"Failed to create attribute val.");
        goto destroy_device;
    }
    dev_set_drvdata(temp, hello_dev);

    printk(KERN_ALERT "Hello linux driver world\n");
    return 0;

destroy_device:
    device_destroy(hello_class, dev);

destroy_class:
    class_destroy(hello_class);
destroy_cdev:
    cdev_del(&(hello_dev->dev));
kfree:
    kfree(hello_dev);
unregister:
    unregister_chrdev_region(dev,1);

fail:
    return err;
}

static void hello_exit(void)
{
    dev_t devno = MKDEV(hello_major, hello_minor);

    if(hello_class) {
        device_destroy(hello_class, MKDEV(hello_major, hello_minor));
        class_destroy(hello_class);
    }

    if(hello_dev) {
        cdev_del(&(hello_dev->dev));
        kfree(hello_dev);
    }

    unregister_chrdev_region(devno, 1);
    printk(KERN_ALERT "Goodbye linux driver\n");
}
MODULE_LICENSE("GPL");
module_init(hello_init);
module_exit(hello_exit);