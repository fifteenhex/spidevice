#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/fs.h>

#define DEV_NAME "spimasterdev"
#define MAX_DEV 2

struct spimasterdev {
    struct cdev cdev;
};

static int dev_major = 0;
static struct class *dev_class = NULL;
static struct spimasterdev devs[MAX_DEV];

static int spimasterdev_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int spimasterdev_release(struct inode *inode, struct file *file)
{
	return 0;
}

static long spimasterdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	return 0;
}

static ssize_t spimasterdev_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
	return 0;
}

static ssize_t spimasterdev_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
	return 0;
}

static const struct file_operations spimasterdev_fops = {
	.owner		= THIS_MODULE,
	.open		= spimasterdev_open,
	.release	= spimasterdev_release,
	.unlocked_ioctl	= spimasterdev_ioctl,
	.read		= spimasterdev_read,
	.write		= spimasterdev_write
};

static int spimasterdev_uevent(struct device *dev, struct kobj_uevent_env *env)
{
	add_uevent_var(env, "DEVMODE=%#o", 0666);
	return 0;
}

static int __init spimasterdev_init(void)
{
	int ret, i;
	dev_t dev;
	struct device *device;

	ret = alloc_chrdev_region(&dev, 0, MAX_DEV, DEV_NAME);
	if (ret)
		return ret;

	dev_major = MAJOR(dev);

	dev_class = class_create(THIS_MODULE, DEV_NAME);
	if (IS_ERR(dev_class)){
		ret = PTR_ERR(dev_class);
		goto unregister_region;
	}

	dev_class->dev_uevent = spimasterdev_uevent;

	for (i = 0; i < MAX_DEV; i++) {
		cdev_init(&devs[i].cdev, &spimasterdev_fops);
		devs[i].cdev.owner = THIS_MODULE;
		ret = cdev_add(&devs[i].cdev, MKDEV(dev_major, i), 1);
		if (ret)
			goto unregister_devs;

		device = device_create(dev_class, NULL, MKDEV(dev_major, i), NULL, DEV_NAME"-%d", i);
		if (IS_ERR(device)) {
			ret = PTR_ERR(device);
			goto unregister_devs;
		}
	}

	return 0;

unregister_devs:
	for (i--; i >= 0; i--)
		device_destroy(dev_class, MKDEV(dev_major, i));
unregister_region:
	unregister_chrdev_region(MKDEV(dev_major, 0), MINORMASK);

	return ret;
}

static void __exit spimasterdev_exit(void)
{
	int i;

	for (i = 0; i < MAX_DEV; i++)
		device_destroy(dev_class, MKDEV(dev_major, i));

	class_unregister(dev_class);
	class_destroy(dev_class);

	unregister_chrdev_region(MKDEV(dev_major, 0), MINORMASK);
}

module_init(spimasterdev_init);
module_exit(spimasterdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Daniel Palmer <daniel@thingy.jp>");
