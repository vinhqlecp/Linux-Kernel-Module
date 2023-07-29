#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>

#include "vchar_driver.h"

#define DRIVER_LICENSE "GPL"
#define DRIVER_AUTHOR "Le Quang Vinh <vinhqle.cp@gmail.com>"
#define DRIVER_DESC   "Simple Linux Kernel Character Drvier"
#define DRIVER_VERSION "0.7"

#define DRIVER_NAME "vchar_device"
#define DRIVER_CLASS "class_vchar_dev"

/* Buffer for data */
static char buffer[255];
static int buffer_pointer = 0;

struct _vchar_drv {
	dev_t dev_num;
	struct class *dev_class;
	struct device *dev;
	struct cdev *vcdev;
	unsigned int open_cnt;
} vchar_drv;

/**
 * @brief Read data out of the buffer
 */
static ssize_t driver_read(struct file *File, char *user_buffer, size_t count, loff_t *offs) {
	int to_copy, not_copied, delta;

	/* Get amount of data to copy */
	to_copy = min(count, buffer_pointer);

	/* Copy data to user */
	not_copied = copy_to_user(user_buffer, buffer, to_copy);

	/* Calculate data */
	delta = to_copy - not_copied;

	printk("Read data from device file.\n");

	return delta;
}

/**
 * @brief Write data to buffer
 */
static ssize_t driver_write(struct file *File, const char *user_buffer, size_t count, loff_t *offs) {
	int to_copy, not_copied, delta;

	/* Get amount of data to copy */
	to_copy = min(count, sizeof(buffer));

	/* Copy data to user */
	not_copied = copy_from_user(buffer, user_buffer, to_copy);
	buffer_pointer = to_copy;

	/* Calculate data */
	delta = to_copy - not_copied;

	printk("Write data to device file.\n");

	return delta;
}

static long int driver_ioctl(struct file *file, unsigned cmd, unsigned long arg){
	int32_t answer = 42;
	switch(cmd){
		case WR_VALUE:
			if(copy_from_user(&answer, (int32_t *) arg, sizeof(answer))) 
				printk("vchar_driver ioctl - Error copying data from user!\n");
			else
				printk("vchar_driver ioctl - Update the answer to %d\n", answer);
			break;
		case RD_VALUE:
			if(copy_to_user((int32_t *) arg, &answer, sizeof(answer))) 
				printk("vchar_driver ioctl - Error copying data to user!\n");
			else
				printk("vchar_driver ioctl - The answer was copied!\n");
			break;
		default:
			break;
	}
	return 0;
}

/**
 * @brief This function is called, when the device file is closed
 */
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
	.release = driver_close,
	.read = driver_read,
	.write = driver_write,
	.unlocked_ioctl = driver_ioctl
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
