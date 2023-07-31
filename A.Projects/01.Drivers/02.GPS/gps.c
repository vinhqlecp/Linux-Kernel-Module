#include "gps.h"
#include "commif.h"

#define DRIVER_LICENSE "GPL"
#define DRIVER_AUTHOR "Le Quang Vinh <vinhqle.cp@gmail.com>"
#define DRIVER_DESC   "Driver for Raspberry Pi 4 GPS U-Blox"
#define DRIVER_VERSION "0.1"

#define DRIVER_NAME "gps"
#define DRIVER_CLASS "rpi4_gps"

#define IRQ_PIN 23
#define IRQ_PIN_NAME "rpi-gpio-23"

const char driver_name[18] = "U-Blox GPS driver";

unsigned int irq_number;

/**
 * @brief Interrupt service routine is called, when interrupt is triggered
 */
static irq_handler_t gpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs) {
	printk("gpio_irq: Interrupt was triggered and ISR was called!\n");
	return (irq_handler_t) IRQ_HANDLED; 
}

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init gps_driver_init(void) {
	/* Setup the gpio */
	if(gpio_request(IRQ_PIN, IRQ_PIN_NAME)) {
		printk("Error!\nCan not allocate GPIO %d\n", IRQ_PIN);
		return -1;
	}

	/* Set GPIO 23 direction */
	if(gpio_direction_input(IRQ_PIN)) {
		printk("Error!\nCan not set GPIO %d to input!\n", IRQ_PIN);
		gpio_free(IRQ_PIN);
		return -1;
	}

	printk(KERN_INFO "Initialize %s successfully.\n", driver_name);

	/* Setup the interrupt */
	irq_number = gpio_to_irq(IRQ_PIN);

	if(request_irq(irq_number, (irq_handler_t) gpio_irq_handler, IRQF_TRIGGER_RISING, "my_gpio_irq", NULL) != 0){
		printk("Error!\nCan not request interrupt nr.: %d\n", irq_number);
		gpio_free(IRQ_PIN);
		return -1;
	}
	printk("GPIO %d is mapped to IRQ Nr.: %d\n", IRQ_PIN, irq_number);

	return 0;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit gps_driver_exit(void) {
	free_irq(irq_number, NULL);
	gpio_free(IRQ_PIN);
	printk(KERN_INFO "Exit %s\n", driver_name);
}

module_init(gps_driver_init);
module_exit(gps_driver_exit);

/* Meta Information */
MODULE_LICENSE(DRIVER_LICENSE);
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
