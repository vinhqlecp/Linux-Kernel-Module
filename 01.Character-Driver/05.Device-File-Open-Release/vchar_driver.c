#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>

#define DRIVER_LICENSE "GPL"
#define DRIVER_AUTHOR "Le Quang Vinh <vinhqle.cp@gmail.com>"
#define DRIVER_DESC   "Simple Linux Kernel Character Drvier"
#define DRIVER_VERSION "0.5"

#define DRIVER_NAME "vchar_device"
#define DRIVER_CLASS "class_vchar_dev"

struct _vchar_drv {
	dev_t dev_num;
	struct class *dev_class;
	struct device *dev;
	struct cdev *vcdev;
	unsigned int open_cnt;
} vchar_drv;

static int driver_open(struct inode *device_file, struct file *instance) {
	vchar_drv.open_cnt++;
	printk("Handle open event (%d)\n", vchar_drv.open_cnt);
	return 0;
}

/**
 * @brief This function is called, when the device file is opened
 */
static int driver_close(struct inode *device_file, struct file *instance) {
	printk("Handle close event\n");
	return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = driver_open,
	.release = driver_close
};

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init symple_module_init(void) {
	int ret = -1;

	ret = alloc_chrdev_region(&vchar_drv.dev_num, 0, 1, DRIVER_NAME);
	if(ret < 0) {
		printk("Failed to register device number dynamically.\n");
		goto failed_register_devnum;
	}
	printk("Allocated device number with major=%d and minor=%d.\n", MAJOR(vchar_drv.dev_num), MINOR(vchar_drv.dev_num));

	vchar_drv.dev_class = class_create(THIS_MODULE, DRIVER_CLASS);
	if(vchar_drv.dev_class == NULL) {
		printk("Failed to create class.\n");
		goto failed_create_class;
	}

	vchar_drv.dev = device_create(vchar_drv.dev_class, NULL, vchar_drv.dev_num, NULL, DRIVER_NAME);
	if(IS_ERR(vchar_drv.dev)) {
		printk("Failed to create a device.\n");
		goto failed_create_device;
	}

	vchar_drv.vcdev = cdev_alloc();
	if(vchar_drv.vcdev == NULL) {
		printk("Failed to allocate cdev structure.\n");
		goto failed_allocate_cdev;
	}
	cdev_init(vchar_drv.vcdev, &fops);
	ret = cdev_add(vchar_drv.vcdev, vchar_drv.dev_num, 1);
	if(ret < 0) {
		printk("Failed to add character device to the system.\n");
		goto failed_allocate_cdev;
	}

	printk("Initialize vchar driver successfully.\n");
	return 0;

failed_allocate_cdev:

failed_create_device:
	class_destroy(vchar_drv.dev_class);
failed_create_class:
	unregister_chrdev_region(vchar_drv.dev_num, 1);
failed_register_devnum:
	return ret;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit simple_module_exit(void) {
	cdev_del(vchar_drv.vcdev);
	device_destroy(vchar_drv.dev_class, vchar_drv.dev_num);
	class_destroy(vchar_drv.dev_class);
	unregister_chrdev_region(vchar_drv.dev_num, 1);
	printk("Exit vchar driver\n");
}

module_init(symple_module_init);
module_exit(simple_module_exit);

/* Meta Information */
MODULE_LICENSE(DRIVER_LICENSE);
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
