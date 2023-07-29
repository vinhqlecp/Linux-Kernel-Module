#include <linux/module.h>
#include <linux/init.h>

#define DRIVER_LICENSE "GPL"
#define DRIVER_AUTHOR "Le Quang Vinh <vinhqle.cp@gmail.com>"
#define DRIVER_DESC   "Simple Linux Kernel Character Drvier"
#define DRIVER_VERSION "0.1"

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init symple_module_init(void) {
	printk("Hello, Linux Kernel!\n");
	return 0;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit simple_module_exit(void) {
	printk("Goodbye, Linux Kernel\n");
}

module_init(symple_module_init);
module_exit(simple_module_exit);

/* Meta Information */
MODULE_LICENSE(DRIVER_LICENSE);
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
