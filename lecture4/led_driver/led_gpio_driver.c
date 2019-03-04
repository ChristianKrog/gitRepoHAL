 #include <linux/gpio.h> 
 #include <linux/fs.h>
 #include <linux/cdev.h>
 #include <linux/device.h>
 #include <linux/uaccess.h>
 #include <linux/module.h>

#define LED2_GPIO 21
#define LED_MAJOR 64
#define LED_MINOR 1
#define LED_AMOUNT 1

static struct cdev led_cdev;
struct file_operations led_fops;
int devno;

MODULE_LICENSE("GPL");

 static int led2gpio_init(void)
 { 
	int err = 0; 
	
	err = gpio_request(LED2_GPIO,"LED2");
	if(err < 0) 
	{
		goto err_exit;
	}
	printk("gpio_reguest was successfull\n");

	err = gpio_direction_output(LED2_GPIO, 1);
	if(err < 0)
	{
		goto err_free_gpio;
	}
	printk("gpio_direction for LED2 got set to output\n");

	devno=MKDEV(LED_MAJOR, LED_MINOR);
	err=register_chrdev_region(devno, LED_AMOUNT, "LED2");
	if(err < 0)// Acquiremajor/minor
	{
		goto err_free_chrdev_region;
	}
	printk("chrdev_region registered\n");

	cdev_init(&led_cdev, &led_fops);
    err = cdev_add(&led_cdev, devno, LED_AMOUNT);
    if(err< 0)// Register driver with kernel
    {
		goto err_dev_unregister;
    }
    printk("cdev_add successfull\n");
	printk("LED GPIO driver got Rhino'd\n");
        
    return 0; // Success!!!

	err_free_gpio:
	gpio_free(LED2_GPIO);

	err_free_chrdev_region:
	gpio_free(LED2_GPIO);

	err_dev_unregister:
	unregister_chrdev_region(devno, LED_AMOUNT);

	err_exit:
	return err;
 }

static void led2gpio_exit(void)
 {
 	cdev_del(&led_cdev);
 	printk("cdev deleted\n");
 	unregister_chrdev_region(devno, LED_AMOUNT);
 	printk("chrdev_region unregistered\n");
 	gpio_free(LED2_GPIO);
 	printk("gpio got free'd\n");
	printk("LED GPIO driver got UnRhino'd\n");
 }

 int led2gpio_open(struct inode *inode, struct file *filep)
 {
 	int major, minor;

 	major = MAJOR(inode->i_rdev);
	minor = MINOR(inode->i_rdev);
 	printk("Opening MyGpio Device [major], [minor]: %i, %i\n", major, minor);
 	return 0;
 }

 int led2gpio_release(struct inode *inode, struct file *filep)
 {
	int minor, major;

 	major = MAJOR(inode->i_rdev);
 	minor = MINOR(inode->i_rdev);
 	printk("Closing/Releasing MyGpio Device [major], [minor]: %i, %i\n", major, minor);

 return 0;
 }

 //KIG PÅ NEDENSTÅENDE 
 //
 //

  ssize_t led2gpio_write(struct file *filep, const char __user *ubuf, size_t count, loff_t *f_pos)
 {
 	int write_value = 0;
    sscanf(ubuf, "%d", &write_value);
    gpio_set_value(LED2_GPIO, write_value);

 return count;
 }

 struct file_operations led_fops = 
 {
	.owner = THIS_MODULE,
	.write = led2gpio_write,
	.open = led2gpio_open,
	.release = led2gpio_release,
};

module_init(led2gpio_init);
module_exit(led2gpio_exit);
