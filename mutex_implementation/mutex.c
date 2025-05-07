#include <linux/module.h>   // For module macros
#include <linux/init.h>     // For __init and __exit macros
#include <linux/cdev.h>     // For character device operations
#include <linux/fs.h>       // For file_operations structure
#include <linux/device.h>   // For class_create and device_create
#include <linux/uaccess.h>  // For copy_to_user and copy_from_user
#include <linux/ioctl.h>

#define MY_DEVICE_NAME "my_device_name"
#define MY_CLASS_NAME "my_class_name"
#define BUF_SIZE 50
#define MAX_MINOR 2
#define MY_IOCTL_MAGIC_NUMBER 'D'
#define IOCTL_GET_SECRET _IOR(MY_IOCTL_MAGIC_NUMBER, 1, char *)
#define IOCTL_SET_SECRET _IOW(MY_IOCTL_MAGIC_NUMBER, 2, char *)

static dev_t my_device;
static struct class *my_class;

static struct my_data {
    struct cdev my_cdev;
    char secret[BUF_SIZE];
    struct mutex lock;
};

struct my_data devs[MAX_MINOR];

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

    mutex_lock(&data->lock);

    available = sizeof(data->secret)  - *pos;

    if (count > available)
        count = available;
    

    if(copy_to_user(buf,data->secret + *pos, count)) {
        mutex_unlock(&data->lock);
        return -EFAULT;
    }

    mutex_unlock(&data->lock);
    *pos += count;

    return count;
}

static ssize_t my_write(struct file *file, const char __user *buf, size_t count, loff_t *pos)
{
    size_t available;
    struct my_data *data = file->private_data;

    pr_info("Writing to the device: %s\n", MY_DEVICE_NAME);

    mutex_lock(&data->lock);

    if (*pos >= sizeof(data->secret) || count == 0)
        return 0;

    available = sizeof(data->secret)  - *pos;

    count = min(count, available);

    if(copy_from_user(data->secret + *pos, buf, count)) {
        mutex_unlock(&data->lock);
        return -EFAULT;
    }

    mutex_unlock(&data->lock);
    *pos += count;

    return count;
}

static loff_t my_llseek(struct file *file, loff_t offset, int whence)
{
    loff_t new_pos;

    switch (whence) {

        case SEEK_SET:
            new_pos = offset;
            break;

        case SEEK_CUR:
            new_pos = file->f_pos + offset;
            break;
        
        case SEEK_END:
            new_pos = BUF_SIZE + offset;
            break;

        default:
            return -EINVAL;
    }

    if (new_pos < 0 || new_pos > BUF_SIZE)
        return -EINVAL;
    
    file->f_pos = new_pos;
    return new_pos;

} // command to check : dd if=/dev/my_device_name0 bs=1 skip=7 count=15

static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {

    struct my_data *data = file->private_data;

    mutex_lock(&data->lock);

    switch(cmd) {
        case IOCTL_GET_SECRET:
            if(copy_to_user((char __user *)arg, data->secret, sizeof(data->secret))) {
                mutex_unlock(&data->lock);
                return -EINVAL;
            }
            pr_info("Secret get by ioctl: %s \n",data->secret);
            break;

        case IOCTL_SET_SECRET:
            if(copy_from_user(data->secret, (char __user *)arg, sizeof(data->secret))) {
                mutex_unlock(&data->lock);
                return -EINVAL;
            }
            pr_info("Secret set by ioctl: %s \n",data->secret);
            break;

        default:
            mutex_unlock(&data->lock);
            return -EINVAL;
    }

    mutex_unlock(&data->lock);

    return 0;
}

static const struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .read = my_read,
    .write = my_write,
    .open = my_open,
    .release = my_release,
    .llseek = my_llseek,
    .unlocked_ioctl = my_ioctl,
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
    mutex_init(&devs[i].lock);
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
