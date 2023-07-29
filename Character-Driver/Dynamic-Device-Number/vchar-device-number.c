#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>   // Thu vien dinh nghia cac ham cap phat/giai phong device number

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Le Quang Vinh (vinhqle.cp@gmail.com)");
MODULE_DESCRIPTION("Device number - Linux Kernel Drvier");

struct _vchar_drv {
    dev_t dev_num;
} vchar_drv;

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init device_number_init(void) {
    int ret = 0;

    // Cap phat device number cho driver
    ret = alloc_chrdev_region(&vchar_drv.dev_num, 0, 1, "vchar_device");
    if(ret < 0){
        printk("failed to register device number with code=%d\n", ret);
        return ret;
    }

    printk("Initialize vchar_driver successfully with MAJOR(%d) and MINOR(%d).\n", MAJOR(vchar_drv.dev_num), MINOR(vchar_drv.dev_num));
    return 0;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit device_number_exit(void) {
    // Giai phong device number cua driver
    unregister_chrdev_region(vchar_drv.dev_num, 1);
    
	printk("Exit vchar driver\n");
}

module_init(device_number_init);
module_exit(device_number_exit);