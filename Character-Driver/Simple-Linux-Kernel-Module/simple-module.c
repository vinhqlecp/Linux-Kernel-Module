#include <linux/module.h>
#include <linux/init.h>

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Le Quang Vinh (vinhqle.cp@gmail.com)");
MODULE_DESCRIPTION("Simple Linux Kernel Drvier");

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