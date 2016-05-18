/* Set your chosen USB's device ID here. Use lsusb to find it. */
static const struct usb_device_id my_device =
	{USB_DEVICE(0x1234, 0x5678)};

/* Files permakey will remove upon chosen USB removal. */
static char *remove_files[] = {
	"/home/you/secrets.txt",
	"/home/you/whatever.zip",
	NULL, /* Must be NULL terminated. */
};
