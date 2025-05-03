#include <linux/module.h>   // For module macros
#include <linux/init.h>     // For __init and __exit macros
#include <linux/cdev.h>     // For character device operations
#include <linux/fs.h>       // For file_operations structure
#include <linux/device.h>   // For class_create and device_create
#include <linux/uaccess.h>  // For copy_to_user and copy_from_user

#define MY_DEVICE_NAME "my_device_name"
#define MY_CLASS_NAME "my_class_name"
#define BUF_SIZE 50

static dev_t my_device;
static struct class *my_class;
static struct cdev my_cdev;

static char secret[BUF_SIZE];

static int my_open(struct inode *inode, struct file *file)
{
    pr_info("Opening device : %s\n", MY_DEVICE_NAME);
    return 0;
}

static int my_release(struct inode *inode, struct file *file)
{
    pr_info("Closing device: %s\n", MY_DEVICE_NAME);
    return 0;
}

static ssize_t my_read(struct file *file, char __user *buf, size_t count, loff_t *pos)
{
    size_t available ;
    pr_info("Reading the device: %s\n", MY_DEVICE_NAME);

    if (*pos >= BUF_SIZE || count == 0) // EOF
        return 0;  

    available = BUF_SIZE - *pos;

    if (count > available)
        count = available;
    

    if(copy_to_user(buf,secret + *pos, count))
     return -EFAULT;

    *pos += count;

    return count;
}

static ssize_t my_write(struct file *file, const char __user *buf, size_t count, loff_t *pos)
{
    size_t available;

    pr_info("Writing to the device: %s\n", MY_DEVICE_NAME);

    if (*pos >= BUF_SIZE || count == 0)
        return 0;

    available = BUF_SIZE - *pos;

    if (count > available)
        count = available;

    if(copy_from_user(secret + *pos, buf, count))
        return -EFAULT;

    *pos += count;

    return count;
}

static const struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .read = my_read,
    .write = my_write,
    .open = my_open,
    .release = my_release,
};

static int __init my_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&my_device, 0, 1, MY_DEVICE_NAME);
    if (ret < 0) {
        pr_err("Failed to allocate char device region\n");
        return ret;
    }

    my_class = class_create(MY_CLASS_NAME);
    if (IS_ERR(my_class)) {
        unregister_chrdev_region(my_device, 1);
        pr_err("Failed to create class\n");
        return PTR_ERR(my_class);
    }

    cdev_init(&my_cdev, &my_fops);
    ret = cdev_add(&my_cdev, my_device, 1);
    if (ret) {
        class_destroy(my_class);
        unregister_chrdev_region(my_device, 1);
        pr_err("Failed to add cdev\n");
        return ret;
    }

    // Print major and minor numbers of the device
    pr_info("Device registered with Major: %d, Minor: %d\n", MAJOR(my_device), MINOR(my_device));

    device_create(my_class, NULL, my_device, NULL, MY_DEVICE_NAME);

    pr_info("Succeeded in registering character device driver %s\n", MY_DEVICE_NAME);

    // Initialize the buffer
    strscpy(secret,"InitialMessage",15);

    return 0;
}

static void __exit my_exit(void)
{
    device_destroy(my_class, my_device);
    cdev_del(&my_cdev);
    class_destroy(my_class);
    unregister_chrdev_region(my_device, 1);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dipendra Khadka");
MODULE_DESCRIPTION("Sample Secret character device driver");
