#include "gps.h"
#include "commif.h"

#define DRIVER_LICENSE "GPL"
#define DRIVER_AUTHOR "Le Quang Vinh <vinhqle.cp@gmail.com>"
#define DRIVER_DESC   "Driver for Raspberry Pi 4 GPS U-Blox"
#define DRIVER_VERSION "0.1"

#define DRIVER_NAME "gps"
#define DRIVER_CLASS "rpi4_gps"

#define IRQ_PIN 23
#define IRQ_PIN_NAME "rpi4-gpio-23"

const char driver_name[18] = "U-Blox GPS driver";

/* Buffer for data */
static char buffer[1024];
static int buffer_pointer = 0;

static unsigned int irq_number;

struct _gps_drv {
	dev_t dev_num;
	struct class *dev_class;
	struct device *dev;
	struct cdev *vcdev;
	unsigned int open_cnt;
} gps_drv;

/**
 * @brief Read data out of the buffer
 */
static ssize_t driver_read(struct file *File, char *user_buffer, size_t count, loff_t *offs) {
	int to_copy, not_copied, delta;

	to_copy = min(count, buffer_pointer);
	not_copied = copy_to_user(user_buffer, buffer, to_copy);
	delta = to_copy - not_copied;

	return delta;
}

/**
 * @brief Write data to buffer
 */
static ssize_t driver_write(struct file *File, const char *user_buffer, size_t count, loff_t *offs) {
	int to_copy, not_copied, delta;

	to_copy = min(count, sizeof(buffer));
	not_copied = copy_from_user(buffer, user_buffer, to_copy);
	buffer_pointer = to_copy;
	delta = to_copy - not_copied;

	return delta;
}

static long int driver_ioctl(struct file *file, unsigned cmd, unsigned long arg){
	return 0;
}

/**
 * @brief This function is called, when the device file is closed
 */
static int driver_open(struct inode *device_file, struct file *instance) {
	gps_drv.open_cnt++;
	return 0;
}

/**
 * @brief This function is called, when the device file is opened
 */
static int driver_close(struct inode *device_file, struct file *instance) {
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
 * @brief Interrupt service routine is called, when interrupt is triggered
 */
static irq_handler_t gpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs) {
	// printk(KERN_INFO "GPS Input Interrupt!\n");
	return (irq_handler_t) IRQ_HANDLED; 
}

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init symple_module_init(void) {
	int ret = -1;

	ret = alloc_chrdev_region(&gps_drv.dev_num, 0, 1, DRIVER_NAME);
	if(ret < 0) {
		printk(KERN_ERR "Failed to register device number dynamically.\n");
		goto failed_register_devnum;
	}
	printk(KERN_INFO "Allocated device number with major=%d and minor=%d.\n", MAJOR(gps_drv.dev_num), MINOR(gps_drv.dev_num));

	gps_drv.dev_class = class_create(THIS_MODULE, DRIVER_CLASS);
	if(gps_drv.dev_class == NULL) {
		printk(KERN_ERR "Failed to create class.\n");
		goto failed_create_class;
	}

	gps_drv.dev = device_create(gps_drv.dev_class, NULL, gps_drv.dev_num, NULL, DRIVER_NAME);
	if(IS_ERR(gps_drv.dev)) {
		printk(KERN_ERR "Failed to create a device.\n");
		goto failed_create_device;
	}

	gps_drv.vcdev = cdev_alloc();
	if(gps_drv.vcdev == NULL) {
		printk(KERN_ERR "Failed to allocate cdev structure.\n");
		goto failed_allocate_cdev;
	}
	cdev_init(gps_drv.vcdev, &fops);
	ret = cdev_add(gps_drv.vcdev, gps_drv.dev_num, 1);
	if(ret < 0) {
		printk(KERN_ERR "Failed to add character device to the system.\n");
		goto failed_allocate_cdev;
	}
	
	printk(KERN_INFO "Initialize %s successfully.\n", driver_name);

	/* Setup the gpio */
	if(gpio_request(IRQ_PIN, IRQ_PIN_NAME)) {
		printk(KERN_ERR "Can not allocate GPIO %d\n", IRQ_PIN);
		goto failed_allocate_cdev;
	}

	/* Set GPIO direction */
	if(gpio_direction_input(IRQ_PIN)) {
		printk(KERN_ERR "Can not set GPIO %d to input!\n", IRQ_PIN);
		goto failed_setup_irq_pin;
	}

	/* Setup the interrupt */
	irq_number = gpio_to_irq(IRQ_PIN);
	if(request_irq(irq_number, (irq_handler_t) gpio_irq_handler, IRQF_TRIGGER_RISING, "gps_gpio_irq", NULL) != 0){
		printk(KERN_ERR "Can not request interrupt nr.: %d\n", irq_number);
		goto failed_setup_irq_pin;
	}
	printk(KERN_INFO "GPIO %d is mapped to IRQ Nr.: %d\n", IRQ_PIN, irq_number);

	return 0;

failed_setup_irq_pin:
	gpio_free(IRQ_PIN);
failed_allocate_cdev:
failed_create_device:
	class_destroy(gps_drv.dev_class);
failed_create_class:
	unregister_chrdev_region(gps_drv.dev_num, 1);
failed_register_devnum:
	return ret;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit simple_module_exit(void) {
	// Release GPS IRQ pin
	free_irq(irq_number, NULL);
	gpio_free(IRQ_PIN);

	cdev_del(gps_drv.vcdev);
	device_destroy(gps_drv.dev_class, gps_drv.dev_num);
	class_destroy(gps_drv.dev_class);
	unregister_chrdev_region(gps_drv.dev_num, 1);
	printk(KERN_INFO "Exit %s\n", driver_name);
}

module_init(symple_module_init);
module_exit(simple_module_exit);

/* Meta Information */
MODULE_LICENSE(DRIVER_LICENSE);
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
