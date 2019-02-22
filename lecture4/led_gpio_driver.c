 #include <linux/gpio.h> 
 #include <linux/fs.h>
 #include <linux/cdev.h>
 #include <linux/device.h>
 #include <linux/uaccess.h>
 #include <linux/module.h>

MODULE_LICENSE("GPL");

 static int led3gpio_init(void)
 { 
 	int gpio_request(21,"LED3");

 /* set as input or output, returning 0 or negative errno */
	int gpio_direction_output(21, 0);


/* _______________________KIG PÅ NEDENDSTÅENDE________________*/
	static int led_char_driver_init(...)
	{
		int err = 0; 
		err = try

		if(err< 0) 
		{
			gotoerr_exit;
		}

		devno=MKDEV(MY_MAJOR, MY_MINOR);
		err=register_chrdev_region(...);
		
		if(err< 0)// Acquiremajor/minor
		{
			gotoerr_free_buf;
		}

		cdev_init(...);
        err= cdev_add(...);
        if(err< 0)// Register driver with kernel
        {
			gotoerr_dev_unregister;
        }
        
        return 0; // Success!!!

        err_dev_unregister:
		unregister_chrdev_region(devno, nbr_devices);

		err_free_buf:
		// Release resourceswhichwereacquired
		
		err_exit:
 		
 		return err;
	
	}
/*_________________KIG PÅ OVENSTÅENDE____________________*/


 	printk(KERN_ALERT "LED GPIO driver got Rhino'd\n");
 }

static void led3gpio_exit(void)
 {
 // Delete Cdev
 // Unregister Device
 // Free GPIO
 	 /* release previously-claimed GPIO */
	void gpio_free(unsigned 21);
	printk(KERN_ALERT "LED GPIO driver got UnRhino'd\n");
 }

module_init(led3gpio_init);
module_exit(led3gpio_exit);
