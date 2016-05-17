#define pr_fmt(fmt)	KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/usb.h>
#include <linux/reboot.h>
#include "config.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nadim Kobeissi, based on silk-guardian by Greg Kroah-Hartman and Nate Brune");
MODULE_DESCRIPTION("A module that automatically shreds files and shuts down computer when a defined USB device is removed.");

static void panic_time(struct usb_device *usb)
{
	int i;
	struct device *dev;
	pr_info("shredding...\n");
	for (i = 0; remove_files[i] != NULL; ++i) {
		char *shred_argv[] = {
			"/usr/bin/shred",
			"-f", "-u", "-n",
			shredIterations,
			remove_files[i],
			NULL,
		};
		call_usermodehelper(shred_argv[0], shred_argv,
				NULL, UMH_WAIT_EXEC);
	}
	printk("...done.\n");
	for (dev = &usb->dev; dev; dev = dev->parent)
		mutex_unlock(&dev->mutex);
	printk("Syncing & powering off.\n");
	kernel_power_off();
}

/*
 * returns 0 if no match, 1 if match
 *
 * Taken from drivers/usb/core/driver.c, as it's not exported for our use :(
 */
static int usb_match_device(struct usb_device *dev,
		const struct usb_device_id *id)
{
	if ((id->match_flags & USB_DEVICE_ID_MATCH_VENDOR) &&
			id->idVendor != le16_to_cpu(dev->descriptor.idVendor))
		return 0;

	if ((id->match_flags & USB_DEVICE_ID_MATCH_PRODUCT) &&
			id->idProduct != le16_to_cpu(dev->descriptor.idProduct))
		return 0;

	/* No need to test id->bcdDevice_lo != 0, since 0 is never
	   greater than any unsigned number. */
	if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_LO) &&
			(id->bcdDevice_lo > le16_to_cpu(dev->descriptor.bcdDevice)))
		return 0;

	if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_HI) &&
			(id->bcdDevice_hi < le16_to_cpu(dev->descriptor.bcdDevice)))
		return 0;

	if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_CLASS) &&
			(id->bDeviceClass != dev->descriptor.bDeviceClass))
		return 0;

	if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_SUBCLASS) &&
			(id->bDeviceSubClass != dev->descriptor.bDeviceSubClass))
		return 0;

	if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_PROTOCOL) &&
			(id->bDeviceProtocol != dev->descriptor.bDeviceProtocol))
		return 0;

	return 1;
}

static void usb_dev_removed(struct usb_device *dev)
{
	const struct usb_device_id *dev_id;
	dev_id = &device_table[0];
	while (!dev_id) {
		if (usb_match_device(dev, dev_id))
			panic_time(dev);
		dev_id++;
	}
}

static int notify(struct notifier_block *self, unsigned long action, void *dev)
{
	switch (action) {
		case USB_DEVICE_REMOVE:
			usb_dev_removed(dev);
			break;
		default:
			break;
	}
	return 0;
}

static struct notifier_block usb_notify = {
	.notifier_call = notify,
};

static int __init permakey_init(void)
{
	usb_register_notify(&usb_notify);
	pr_info("Now watching USB devices...\n");
	return 0;
}
module_init(permakey_init);

static void __exit permakey_exit(void)
{
	usb_unregister_notify(&usb_notify);
	pr_info("No longer watching USB devices.\n");
}
module_exit(permakey_exit);
