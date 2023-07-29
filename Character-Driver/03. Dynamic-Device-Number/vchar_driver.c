#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>

#define DRIVER_LICENSE "GPL"
#define DRIVER_AUTHOR "Le Quang Vinh <vinhqle.cp@gmail.com>"
#define DRIVER_DESC   "Simple Linux Kernel Character Drvier"
#define DRIVER_VERSION "0.3"

struct _vchar_drv {
	dev_t dev_num;
} vchar_drv;

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init symple_module_init(void) {
	int ret = 0;

	ret = alloc_chrdev_region(&vchar_drv.dev_num, 0, 1, "vchar_device");
	if(ret < 0) {
		printk("Failed to register device number dynamically.\n");
		return ret;
	}
	printk("Allocated device number with major=%d and minor=%d.\n", MAJOR(vchar_drv.dev_num), MINOR(vchar_drv.dev_num));
	printk("Initialize vchar driver successfully.\n");
	return 0;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit simple_module_exit(void) {
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
