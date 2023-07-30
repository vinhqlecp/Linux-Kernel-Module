#include "health_check.h"
#include "commif.h"

#define DRIVER_LICENSE "GPL"
#define DRIVER_AUTHOR "Le Quang Vinh <vinhqle.cp@gmail.com>"
#define DRIVER_DESC   "Driver for Raspberry Pi 4 health check"
#define DRIVER_VERSION "0.1"

#define DRIVER_NAME "healthcheck"
#define DRIVER_CLASS "rpi4_healthcheck"

#define GPIO_ADDR_BASE      0xFE200000      // GPIO base address
#define GPIO_MOD_SIZE       0xF3            // GPIO module size
#define GPIO_FUNC_SEL2      0x08            // GPIO function select for gpio 2x
#define GPIO_PIN_OUT_SET    0x1C            // GPIO pin output set for gpio 0~31
#define GPIO_PIN_OUT_CLR    0x28            // GPIO pin output clear for gpio 0~31
#define GPIO_PIN_LEVEL      0x34            // GPIO pin level for gpio 0~31
#define GPIO_PIN_PUP_PDN    0xE8            // GPIO pin set resistor pull up/ pull down

#define GPIO_23             23
#define GPIO_23_FUNC	    1 << ((GPIO_23 % 10) * 3)
#define GPIO_23_INDEX 	    1 << (GPIO_23 % 32)
#define FSEL_23_MASK 	    0b111 << ((GPIO_23 % 10) * 3)
#define GPIO_23_INDEX 	    1 << (GPIO_23 % 32)

unsigned int *GPFSEL2_ADDR = NULL;
unsigned int *GPSET0_ADDR = NULL;
unsigned int *GPCLR0_ADDR = NULL;

const char driver_name[13] = "health check";

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

	return delta;
}

/**
 * @brief Write data to buffer
 */
static ssize_t driver_write(struct file *File, const char *user_buffer, size_t count, loff_t *offs) {
	int to_copy, not_copied, delta;

	EM_COMM_MSG_TYPE msgType;
	st_Comm_Header* pHeader;
	st_Led_Ctrl_Req* lcReq;

	/* Get amount of data to copy */
	to_copy = min(count, sizeof(buffer));

	/* Copy data to user */
	not_copied = copy_from_user(buffer, user_buffer, to_copy);
	buffer_pointer = to_copy;

	/* Calculate data */
	delta = to_copy - not_copied;

	pHeader = (st_Comm_Header*)buffer;
	msgType = (EM_COMM_MSG_TYPE)pHeader->dwMessageID;

	switch(msgType){
		case OPT_LED_CTRL_REQ:
			lcReq = (st_Led_Ctrl_Req*)buffer;
			// printk(KERN_INFO "Change LED state to level %d.\n", lcReq->msg.uiLedState);
			if(lcReq->msg.uiLedState == 0){
				// Clear GPIO 23 level output
				iowrite32(GPIO_23_INDEX, GPCLR0_ADDR);
			} else {
				// Set GPIO 23 level output
				iowrite32(GPIO_23_INDEX, GPSET0_ADDR);
			}
			break;
	}

	return delta;
}

static long int driver_ioctl(struct file *file, unsigned cmd, unsigned long arg){
	return 0;
}

/**
 * @brief This function is called, when the device file is closed
 */
static int driver_open(struct inode *device_file, struct file *instance) {
	vchar_drv.open_cnt++;
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
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init symple_module_init(void) {
	int ret = -1;

	ret = alloc_chrdev_region(&vchar_drv.dev_num, 0, 1, DRIVER_NAME);
	if(ret < 0) {
		printk(KERN_ERR "Failed to register device number dynamically.\n");
		goto failed_register_devnum;
	}
	printk(KERN_INFO "Allocated device number with major=%d and minor=%d.\n", MAJOR(vchar_drv.dev_num), MINOR(vchar_drv.dev_num));

	vchar_drv.dev_class = class_create(THIS_MODULE, DRIVER_CLASS);
	if(vchar_drv.dev_class == NULL) {
		printk(KERN_ERR "Failed to create class.\n");
		goto failed_create_class;
	}

	vchar_drv.dev = device_create(vchar_drv.dev_class, NULL, vchar_drv.dev_num, NULL, DRIVER_NAME);
	if(IS_ERR(vchar_drv.dev)) {
		printk(KERN_ERR "Failed to create a device.\n");
		goto failed_create_device;
	}

	vchar_drv.vcdev = cdev_alloc();
	if(vchar_drv.vcdev == NULL) {
		printk(KERN_ERR "Failed to allocate cdev structure.\n");
		goto failed_allocate_cdev;
	}
	cdev_init(vchar_drv.vcdev, &fops);
	ret = cdev_add(vchar_drv.vcdev, vchar_drv.dev_num, 1);
	if(ret < 0) {
		printk(KERN_ERR "Failed to add character device to the system.\n");
		goto failed_allocate_cdev;
	}

	// Mapping register
	GPFSEL2_ADDR = (unsigned int*)ioremap(GPIO_ADDR_BASE + GPIO_FUNC_SEL2, sizeof(__u32));
	if(NULL == GPFSEL2_ADDR){
		printk(KERN_ERR "Mapping set function failure!\n");
		return -EBUSY;
	}
	GPSET0_ADDR = (unsigned int*)ioremap(GPIO_ADDR_BASE + GPIO_PIN_OUT_SET, sizeof(__u32));
	if(NULL == GPSET0_ADDR){
		printk(KERN_ERR "Mapping set level failure!\n");
		return -EBUSY;
	}
	GPCLR0_ADDR = (unsigned int*)ioremap(GPIO_ADDR_BASE + GPIO_PIN_OUT_CLR, sizeof(__u32));
	if(NULL == GPCLR0_ADDR){
		printk(KERN_ERR "Mapping clear level failure!\n");
		return -EBUSY;
	}

	// Set GPIO 23 to OUTPUT
	iowrite32((ioread32(GPFSEL2_ADDR) & ~FSEL_23_MASK) | (GPIO_23_FUNC & FSEL_23_MASK), GPFSEL2_ADDR);

	printk(KERN_INFO "Initialize %s successfully.\n", driver_name);
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
	// Clear GPIO 23 level output
	iowrite32(GPIO_23_INDEX, GPCLR0_ADDR);
	// Unmap register
	iounmap(GPFSEL2_ADDR);
	iounmap(GPSET0_ADDR);
	iounmap(GPCLR0_ADDR);

	cdev_del(vchar_drv.vcdev);
	device_destroy(vchar_drv.dev_class, vchar_drv.dev_num);
	class_destroy(vchar_drv.dev_class);
	unregister_chrdev_region(vchar_drv.dev_num, 1);
	printk(KERN_INFO "Exit %s\n", driver_name);
}

module_init(symple_module_init);
module_exit(simple_module_exit);

/* Meta Information */
MODULE_LICENSE(DRIVER_LICENSE);
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
