#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/spi/spi.h>

#define DEV_NAME "spimasterdev"
#define MAX_DEV 2

struct spimasterdev {
    struct cdev cdev;
};

static int dev_major = 0;
static struct class *dev_class = NULL;

static ssize_t spimaster_new_device_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t size)
{
	struct spi_master *spimaster = dev_get_drvdata(dev);
	struct spimasterdev *spimasterdev = spi_master_get_devdata(spimaster);

	struct spi_board_info new_device_info = {
		.modalias = "m25p128",
		.max_speed_hz = 11000000,
		.chip_select = 0,
		.mode = SPI_MODE_0,
		.platform_data = NULL,
	};

	spi_new_device(spimaster, &new_device_info);

	return size;
}

static DEVICE_ATTR(new_device, S_IWUSR, NULL, spimaster_new_device_store);

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

static int spimasterdev_spi_setup(struct spi_device *spi)
{
	return 0;
}

static int spimasterdev_spi_transfer_one(struct spi_controller *ctlr, struct spi_device *spi,
					 struct spi_transfer *transfer)
{
	return 0;
}

static void spimasterdev_set_cs(struct spi_device *spi, bool enable)
{

}

static int __init spimasterdev_init(void)
{
	int ret, i;
	dev_t dev;
	struct device *device;
	struct spi_master *master;
	struct spimasterdev *spimasterdev;

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
		device = device_create(dev_class, NULL, MKDEV(dev_major, i), NULL, DEV_NAME"-%d", i);
		if (IS_ERR(device)) {
			ret = PTR_ERR(device);
			goto unregister_devs;
		}

		device_create_file(device, &dev_attr_new_device);

		master = spi_alloc_master(device, sizeof(struct spimasterdev));
		if (!master) {
			ret = -ENOMEM;
			goto unregister_devs;
		}
		dev_set_drvdata(device, master);

		spimasterdev = spi_master_get_devdata(master);
		cdev_init(&spimasterdev->cdev, &spimasterdev_fops);
		spimasterdev->cdev.owner = THIS_MODULE;
		ret = cdev_add(&spimasterdev->cdev, MKDEV(dev_major, i), 1);
		if (ret)
			goto unregister_devs;

		master->bus_num = -1;
		master->num_chipselect = 1;
		master->mode_bits = SPI_CPHA | SPI_CPOL;
		master->flags = SPI_CONTROLLER_HALF_DUPLEX;
		master->setup = spimasterdev_spi_setup;
		master->transfer_one = spimasterdev_spi_transfer_one;
		master->set_cs = spimasterdev_set_cs;

		ret = devm_spi_register_master(device, master);
		if (ret)
			goto unregister_devs;
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
