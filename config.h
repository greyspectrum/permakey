/* Set your chosen USB's device ID here. Use lsusb to find it. */
static const struct usb_device_id device_table[] = {
	{ USB_DEVICE(0x1050, 0x0406) },
	{  },
};

/* Files permakey will remove upon chosen USB removal. */
static char *remove_files[] = {
	"/home/user/privatekey",
	"/private/ssnumber.pdf",
	NULL,	/* Must be NULL terminated */
};

/* How many times to shred file. The more iterations the longer it takes. */
static char *shredIterations = "3";
