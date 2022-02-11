#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/leds.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/module.h>
#include <mach/gpio.h>
#include <linux/device.h>
#include <linux/gpio/driver.h>
#include <linux/gpio/consumer.h>

#define DRVNAME "nuc980-led"
#define NUC980_LED_GPIO	5

static struct platform_device *pdev;

static void nuc980_led_1_set(struct led_classdev *led_cdev, enum led_brightness value)
{
	// Set GPIO value depending on "value" we are called with.
	// LED is either on or off
	if (value) {
		gpio_set_value(NUC980_LED_GPIO, 1);
	} else {
		gpio_set_value(NUC980_LED_GPIO, 0);
	}
}

// Structure defining the name of the LED, function to call to set it, and it's default trigger
static struct led_classdev nuc980_led_1 = {
	.name			= "nuc980::led1",
	.brightness_set		= nuc980_led_1_set,
	.default_trigger	= "heartbeat",
	.flags			= LED_CORE_SUSPENDRESUME,
};

// What is the function of _probe?
static int nuc980_led_probe(struct platform_device *pdev)
{
	int return_value = 0;
	pr_debug("nuc980_led_probe: Registering classdev...");
	return_value = devm_led_classdev_register(&pdev->dev, &nuc980_led_1);

	pr_debug("nuc980_led_probe: Classdev register returned %d", return_value);
	return return_value;
}

// Structure defining ourselves as a platform(?) driver, registering the name and function to use when probing for hardware presence(?)
static struct platform_driver nuc980_led_driver = {
	.probe			= nuc980_led_probe,
	.driver			= {
		.name		= DRVNAME,
	},
};


static int __init nuc980_led_init(void)
{

	printk(KERN_INFO "leds-nuc980: NUC980 Custom LED driver");

	int return_value;

	bool gpio_valid;

	gpio_valid = gpio_is_valid(NUC980_LED_GPIO);
	pr_debug("nuc980_led_init: GPIO validity is %d\n", gpio_valid);

	// Request the GPIO
	pr_debug("nuc980_led_init: Claiming GPIO...\n");
	return_value = gpio_request(NUC980_LED_GPIO, "NUC_LED_GPIO");
	if (return_value < 0) {
		pr_err("nuc980_led_init: Unable to claim GPIO.  Error: %d\n", return_value);
		goto out;
	}

	pr_debug("nuc980_led_init: Finished GPIO claim\n");

	// Set the direction to output
	pr_debug("nuc980_led_init: Setting direction as output...\n");
	return_value = gpio_direction_output(NUC980_LED_GPIO, 0);
	if (return_value < 0) {
		pr_err("nuc980_led_init: Unable to set GPIO direction.  Error: %d\n", return_value);
		goto out;
	}

	pr_debug("nuc980_led_init: Finished GPIO direction set\n");

	// Write value as high
	pr_debug("nuc980_led_init: Set GPIO value to 1...\n");
	gpio_set_value(NUC980_LED_GPIO, 1);
	
	pr_debug("nuc980_led_init: Finished set GPIO value to 1\n");

	pr_debug("nuc980_led_init: Registering platform driver...\n");
	return_value = platform_driver_register(&nuc980_led_driver);
	if (return_value < 0) {
		goto out;
	}

	pr_debug("nuc980_led_init: Finished platform register\n");

	pr_debug("nuc980_led_init: Registering device driver...\n");
	pdev = platform_device_register_simple(DRVNAME, -1, NULL, 0);	// What do these mean?
	if (IS_ERR(pdev)) {
		return_value = PTR_ERR(pdev);
		platform_driver_unregister(&nuc980_led_driver);
		goto out;
	}

	pr_debug("nuc980_led_init: Finished device register\n");

out:
	pr_debug("nuc980_led_init: out:\n");

	return return_value;
}


static void __exit nuc980_led_exit(void)
{
	pr_debug("nuc980_led_exit: Unregistering device driver\n");
	platform_device_unregister(pdev);

	pr_debug("nuc980_led_exit: Unregistering platform driver\n");
	platform_driver_unregister(&nuc980_led_driver);

	pr_debug("nuc980_led_exit: Freeing GPIO\n");
	gpio_free(NUC980_LED_GPIO);

	pr_debug("nuc980_led_exit: Finished\n");

}

module_init(nuc980_led_init);
module_exit(nuc980_led_exit);

MODULE_AUTHOR("Paul Roegele <paul@gammaquadrant.net>");
MODULE_DESCRIPTION("Nuvoton NUC980 Custom GPIO LED driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.01");
