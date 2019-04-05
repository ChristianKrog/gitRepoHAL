#include <linux/init.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/of_gpio.h>
#include <linux/of_device.h>
#include <linux/of.h>

static int NUM_OF_BUTS = 0;

MODULE_LICENSE("GPL");

struct button_dev {
	int buttonno;
	int gpio_number;
	int dir;
	int default_output;
};

static struct button_dev rhino_button_devs[255];

int devno;

struct cdev rhino_cdev;
struct file_operations rhino_fops;
struct class *rhino_buttons;
struct device *rhino_button[255];
unsigned int rhino_custom;

static int rhino_button_probe(struct platform_device *pdev)
{
	printk(KERN_DEBUG "New Platform device: %s \n", pdev->name);
	int err = 0, i, k;

	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	enum of_gpio_flags flag;

	NUM_OF_BUTS = of_gpio_count(np);
	//const char name[30] = "rhino_custom\0";
	err = of_property_read_u32(np, "rhino_custom", &rhino_custom); 
	if(err != 0) printk(KERN_ALERT "of_property_read_u32 failed with error: %d\n", err);
	else printk(KERN_DEBUG "rhino_custom = %d\n", rhino_custom);

	printk(KERN_DEBUG "gpio count: %d\n", NUM_OF_BUTS);

	for(k = 0; k < NUM_OF_BUTS; k++)
	{
		rhino_button_devs[k].gpio_number = of_get_gpio_flags(np, k, &flag);
		
		rhino_button_devs[k].dir = flag;
	}


	for(i = 0; i < NUM_OF_BUTS; i++)
	{
		rhino_button_devs[i].default_output = 0;
		err = gpio_request(rhino_button_devs[i].gpio_number, strchr("button%s", rhino_button_devs[i].buttonno));
	
		if(err < 0) goto error_exit;

		if(rhino_button_devs[i].dir == 0)
		{
			err = gpio_direction_input(rhino_button_devs[i].gpio_number);
			if(err == 0)printk(KERN_DEBUG "gpio %d directions got set to input\n", rhino_button_devs[i].gpio_number);
			else 
			{
				printk(KERN_ALERT "gpio_direction_input error in gpio: %d\n", rhino_button_devs[i].gpio_number);
		goto error_exit;
		}
		}
		else if(rhino_button_devs[i].dir == 1)
		{
		err = gpio_direction_output(rhino_button_devs[i].gpio_number, rhino_button_devs[i].default_output);	
		
		if(err == 0)
		{
			printk(KERN_DEBUG "gpio %d directions got set to output\n", rhino_button_devs[i].gpio_number);
		
			if(((rhino_custom >> i) & 1) > 0)
                        {
                        	gpio_set_value(rhino_button_devs[i].gpio_number, 1);
                        	printk(KERN_DEBUG "gpio %d got set to high\n", rhino_button_devs[i].gpio_number);
			}
                	else
                	{
				gpio_set_value(rhino_button_devs[i].gpio_number, 0);
                        	printk(KERN_DEBUG "gpio%d got set to low\n", rhino_button_devs[i].gpio_number);
                	}
		}	
		else 
		{
		printk(KERN_ALERT "gpio_direction_output error in gpio: %d\n", rhino_button_devs[i].gpio_number);
		goto error_exit;
		}
		}
		rhino_button[i] = device_create(rhino_buttons, NULL, MKDEV(MAJOR(devno), i), NULL, "button%d", i); 

	}	
	
	printk(KERN_DEBUG "gpio_reguest, successfull\n");

	return err;
	error_exit:
		for(int d = i; d > 0; d--)
		{
			gpio_free(rhino_button_devs[d].gpio_number);
		}
		return err;
}

static int rhino_button_remove(struct platform_device *pdev)
{
printk(KERN_DEBUG "Removing device %s \n", pdev->name);
for(int i = 0; i < NUM_OF_BUTS; i++)
{
device_destroy(rhino_buttons, MKDEV(MAJOR(devno), i));
gpio_free(rhino_button_devs[i].gpio_number);
}
return 0;
}

static const struct of_device_id rhino_button_platform_device_match[] =
{
{ .compatible = "ase, plat_drv",}, {},
};

static struct platform_driver rhino_button_platform_driver = {
.probe = rhino_button_probe,
.remove = rhino_button_remove,
.driver = {
	.name = "ase, plat_drv",
	.of_match_table = rhino_button_platform_device_match,
	.owner = THIS_MODULE, },
};


ssize_t gpio_read(struct file *filep, char __user *buf, size_t count, loff_t *f_pos)
{
	int nodno, but; 
	nodno = MINOR(filep->f_inode->i_rdev);
	but = rhino_button_devs[nodno].gpio_number;
	int cpt_error;
	int temp = gpio_get_value(but);
	char read_value[2];
	sprintf(read_value, "%d", temp); 
	int read_value_len = strlen(read_value) + 1;
	read_value_len = read_value_len > count ? count : read_value_len;
	cpt_error = copy_to_user(buf, read_value, read_value_len);
	*f_pos += read_value_len;
	return read_value_len;
}

ssize_t gpio_write(struct file *filep, const char __user *buf, size_t count, loff_t *f_pos)
{
	int nodno, out_val; 
	nodno = MINOR(filep->f_inode->i_rdev);
  	char rhino_buf[2];
  	int err = copy_from_user(rhino_buf, buf, count);
  	if(err !=0) printk(KERN_ALERT "Write error!");

	sscanf(rhino_buf, "%d", &out_val);
  	gpio_set_value(rhino_button_devs[nodno].gpio_number, out_val);
  	*f_pos += count;
  	return count;
}

 int gpio_open(struct inode *inode, struct file *filep)
 {
 int major, minor;
 major = MAJOR(inode->i_rdev);
 minor = MINOR(inode->i_rdev);
 printk(KERN_DEBUG "Opening gpio Device [major], [minor]: %i, %i\n", major, minor);
 return 0;
 }

 int gpio_release(struct inode *inode, struct file *filep)
 {
 int minor, major;

 major = MAJOR(inode->i_rdev);
 minor = MINOR(inode->i_rdev);
 printk(KERN_DEBUG "Releasing gpio Device [major], [minor]: %i, %i\n", major, minor);

 return 0;
 }



struct file_operations rhino_fops = {
	.owner = THIS_MODULE,
	.read = gpio_read,
	.write = gpio_write,
	.open = gpio_open,
	.release = gpio_release,
};

static int gpio_init(void)
{
	int err;
	err = alloc_chrdev_region(&devno, 0,255, "Buttons");
	if(err < 0) goto error_free_chrdev_region;
	printk(KERN_DEBUG "chrdev_region got registered\n");

	rhino_buttons = class_create(THIS_MODULE, "rhino_buttons_cls");
	cdev_init(&rhino_cdev, &rhino_fops);
	
	err = cdev_add(&rhino_cdev, devno, 255);
	if(err < 0) goto error_free_chrdev_region;
	printk(KERN_DEBUG "cdev got add'ed\n");
	printk(KERN_DEBUG "init got executed\n");

	platform_driver_register(&rhino_button_platform_driver);

	return 0;

	error_free_chrdev_region:
		unregister_chrdev_region(devno, NUM_OF_BUTS);
		return err;
}

static void gpio_exit(void)
{
	platform_driver_unregister(&rhino_button_platform_driver);
	printk(KERN_DEBUG "Platform driver got unregistered\n");
	class_destroy(rhino_buttons);
	printk(KERN_DEBUG "Rhino class got destroyed\n");
	cdev_del(&rhino_cdev);
	printk(KERN_DEBUG "cdev got deleted\n");
	unregister_chrdev_region(devno, 255);
	printk(KERN_DEBUG "chrdev_region got unregistered\n");
	printk(KERN_DEBUG "gpiodriver got Unrhino'd\n");
}

module_init(gpio_init);
module_exit(gpio_exit);

