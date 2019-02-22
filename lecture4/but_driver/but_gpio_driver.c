#include <linux/init.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>

#define b0 12
#define b1 16

#define BUT0_MAJOR 12
#define BUT0_MINOR 0
#define NUM_OF_BUTS 1

MODULE_LICENSE("GPL");

char but0[8] = "button0";
char but1[8] = "button1";

int devno;

struct cdev rhino_cdev;
struct file_operations rhino_fops;
struct file_operations gpio_fops;


rhino_fops = gpio_fops;

static int gpio_init(void)
{
	int err;
	err = gpio_request(b0, but0);
	if(err < 0) goto error_exit;
	printk(KERN_ALERT "gpio_reguest, successfull\n");
	err = gpio_direction_input(b0);
	if(err < 0) goto error_free_gpio;
	printk(KERN_ALERT "gpio_direction got set to input\n");
	devno = MKDEV(BUT0_MAJOR, BUT0_MINOR);

	err = register_chrdev_region(devno, NUM_OF_BUTS, "Button0");
	if(err < 0) goto error_free_chrdev_region;
	printk(KERN_ALERT "chrdev_region got registered\n");

	cdev_init(&rhino_cdev, &rhino_fops);
	
	err = cdev_add(&rhino_cdev, devno, NUM_OF_BUTS);
	if(err < 0) goto error_unreg_cdev;
	printk(KERN_ALERT "cdev got add'ed\n");
	printk(KERN_ALERT "gpiodriver got Rhino'd\n");

	return 0;


		error_unreg_cdev:
		unregister_chrdev_region(devno, NUM_OF_BUTS);
	error_free_chrdev_region:
		goto error_free_gpio;
	error_free_gpio:
		gpio_free(b0);
	error_exit:
		return err;
}

ssize_t gpio_read(struct file *filep, char __ *buf, size_t count, loff_t *f_pos)
{
	int temp = gpio_get_value(b0);
	int read_value[2];
	sprintf(read_value, "%s", temp); 
	int read_value_len = strlen(read_value) + 1;
	read_value_len = read_value_len > count ? count : read_value_len;
	copy_to_user(buf, read_value, read_value_len);
	*f_pos += read_value_len;
	return read_value_len;
}

 int gpio_open(struct inode *inode, struct file *filep)
 {
 int major, minor;
 major = MAJOR(inode->i_rdev);
 minor = MINOR(inode->i_rdev);
 printk("Opening gpio Device [major], [minor]: %i, %i\n", major, minor);
 return 0;
 }

 int gpio_release(struct inode *inode, struct file *filep)
 {
 int minor, major;

 major = MAJOR(inode->i_rdev);
 minor = MINOR(inode->i_rdev);
 printk("Releasing gpio Device [major], [minor]: %i, %i\n", major, minor);

 return 0;
 }

static void gpio_exit(void)
{
	cdev_del(&rhino_cdev);
	printk(KERN_ALERT "cdev got deleted\n");
	unregister_chrdev_region(devno, NUM_OF_BUTS);
	printk(KERN_ALERT "chrdev_region got unregistered\n");
	gpio_free(b0);
	printk(KERN_ALERT "gpio got free'd\n");
	printk(KERN_ALERT "gpiodriver got Unrhino'd\n");
}

struct file_operations gpio_fops = {
	.owner = THIS_MODULE,
	.read = gpio_read,
	//.write = gpio_write,
	.open = gpio_open,
	.release = gpio_release,
};

module_init(gpio_init);
module_exit(gpio_exit);

