#include <linux/module.h>   // For module macros
#include <linux/init.h>     // For __init and __exit macros
#include <linux/cdev.h>     // For character device operations
#include <linux/fs.h>       // For file_operations structure
#include <linux/device.h>   // For class_create and device_create
#include <linux/uaccess.h>  // For copy_to_user and copy_from_user

#define MY_DEVICE_NAME "my_device_name"
#define MY_CLASS_NAME "my_class_name"
#define BUF_SIZE 50
#define MAX_MINOR 2

static dev_t my_device;
static struct class *my_class;

static struct my_data {
    struct cdev my_cdev;
    char secret[BUF_SIZE];
};

struct my_data devs[2];

static int my_open(struct inode *inode, struct file *file)
{
    pr_info("Opening device: %s (minor = %d)\n", MY_DEVICE_NAME, iminor(inode));


    struct my_data *data = container_of(inode->i_cdev,struct my_data, my_cdev);
    file->private_data = data;

    return 0;
}

static int my_release(struct inode *inode, struct file *file)
{
    pr_info("Closing device: %s (minor = %d)\n", MY_DEVICE_NAME, iminor(inode));

    return 0;
}

static ssize_t my_read(struct file *file, char __user *buf, size_t count, loff_t *pos)
{
    size_t available ;
    struct my_data *data = file->private_data;

    pr_info("Reading the device: %s\n", MY_DEVICE_NAME);

    if (*pos >= sizeof(data->secret) || count == 0) // EOF
        return 0;  

    available = sizeof(data->secret)  - *pos;

    if (count > available)
        count = available;
    

    if(copy_to_user(buf,data->secret + *pos, count))
     return -EFAULT;

    *pos += count;

    return count;
}

static ssize_t my_write(struct file *file, const char __user *buf, size_t count, loff_t *pos)
{
    size_t available;

    pr_info("Writing to the device: %s\n", MY_DEVICE_NAME);

    struct my_data *data = file->private_data;

    if (*pos >= sizeof(data->secret) || count == 0)
        return 0;

    available = sizeof(data->secret)  - *pos;

    if (count > available)
        count = available;
    // empty the region before writing 
    memset(data->secret, 0, sizeof(data->secret));

    if(copy_from_user(data->secret + *pos, buf, count))
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
    int ret, i;
    dev_t dev_num;
    char dev_name[50];

    ret = alloc_chrdev_region(&my_device, 0, MAX_MINOR, MY_DEVICE_NAME);
    if (ret < 0) {
        pr_err("Failed to allocate char device region\n");
        return ret;
    }

    my_class = class_create(MY_CLASS_NAME);
    if (IS_ERR(my_class)) {
        unregister_chrdev_region(my_device, MAX_MINOR);
        pr_err("Failed to create class\n");
        return PTR_ERR(my_class);
    }
    
    for(i = 0; i < MAX_MINOR; i++)
    {
    dev_num = MKDEV(MAJOR(my_device),MINOR(my_device) + i);

    cdev_init(&devs[i].my_cdev, &my_fops);
    ret = cdev_add(&devs[i].my_cdev, dev_num, 1);
    if (ret) {
        class_destroy(my_class);
        unregister_chrdev_region(my_device, MAX_MINOR);
        pr_err("Failed to add cdev\n");
        return ret;
    }

    // Print major and minor numbers of the device
    pr_info("Device registered with Major: %d, Minor: %d\n", MAJOR(dev_num), MINOR(dev_num));

    snprintf(dev_name,sizeof(dev_name),"%s%d",MY_DEVICE_NAME,i);
    device_create(my_class, NULL, dev_num, NULL, dev_name);

    // Initialise the values
    strscpy(devs[i].secret,"InitialMessage",15);
    }
    pr_info("Succeeded in registering character device driver %s\n", MY_DEVICE_NAME);

    return 0;
}

static void __exit my_exit(void)
{
    int i;
    dev_t dev_num;

    for(i=0; i<MAX_MINOR; i++) {
    dev_num = MKDEV(MAJOR(my_device), MINOR(my_device) + i);

    device_destroy(my_class, dev_num);
    cdev_del(&devs[i].my_cdev);

    }

    class_destroy(my_class);
    unregister_chrdev_region(my_device, MAX_MINOR);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dipendra Khadka");
MODULE_DESCRIPTION("Sample Secret Per OPen character device driver");
