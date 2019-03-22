#include <linux/init.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/sched.h>

#define b0 19

#define BUT0_MAJOR 12
#define BUT0_MINOR 0
#define NUM_OF_BUTS 1

MODULE_LICENSE("GPL");

char but0[8] = "button0";

int devno;
unsigned int irq;

volatile unsigned int isr_but_val, read_but_val, cnt_isr, cnt_read;

char my_irq[3] = "mi";

static DECLARE_WAIT_QUEUE_HEAD(wq);
static int flag = 0;

struct cdev rhino_cdev;
struct file_operations rhino_fops;

irqreturn_t but_ISR(int irq, void *dev_id);

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

	irq = gpio_to_irq(b0);
	printk(KERN_ALERT "gpio on irq line:%d\n", irq);

	err = request_irq(irq, but_ISR, IRQF_TRIGGER_RISING, "but_IRQ", my_irq);

	if(err < 0) goto error_req_irq;
	printk(KERN_ALERT "request IRQ, Successs\n");



	printk(KERN_ALERT "gpiodriver got Rhino'd\n");

	return 0;

	error_req_irq:
		goto error_unreg_cdev;
	error_unreg_cdev:
		unregister_chrdev_region(devno, NUM_OF_BUTS);
	error_free_chrdev_region:
		goto error_free_gpio;
	error_free_gpio:
		gpio_free(b0);
	error_exit:
		return err;
}

irqreturn_t but_ISR(int irq, void *dev_id)
{
	isr_but_val = gpio_get_value(b0);
	cnt_isr++;
	flag = 1;
	wake_up_interruptible(&wq);
	return IRQ_HANDLED;
}

ssize_t gpio_read(struct file *filep, char __user *buf, size_t count, loff_t *f_pos)
{
	wait_event_interruptible(wq, flag == 1);
	flag = 0;
	read_but_val = gpio_get_value(b0);
	cnt_read++;
	char read_value[64];
	sprintf(read_value, "%d\t%d\t%d\t%d\n", cnt_isr, cnt_read, isr_but_val, read_but_val); 
	int read_value_len = strlen(read_value) + 1;
	read_value_len = read_value_len > count ? count : read_value_len;
	int cpt_error;
	cpt_error = copy_to_user(buf, read_value, read_value_len);
	printk("read/copy_to_user successfull");
	*f_pos += read_value_len;
	return read_value_len;
}

 int gpio_open(struct inode *inode, struct file *filep)
 {

isr_but_val = -1;
read_but_val = -1;
cnt_isr = -1; 
cnt_read = -1;
flag = 0;
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
	free_irq(irq, my_irq);
	printk(KERN_ALERT "IRQ released\n");
	cdev_del(&rhino_cdev);
	printk(KERN_ALERT "cdev got deleted\n");
	unregister_chrdev_region(devno, NUM_OF_BUTS);
	printk(KERN_ALERT "chrdev_region got unregistered\n");
	gpio_free(b0);
	printk(KERN_ALERT "gpio got free'd\n");
	printk(KERN_ALERT "gpiodriver got Unrhino'd\n");
}

struct file_operations rhino_fops = {
	.owner = THIS_MODULE,
	.read = gpio_read,
	//.write = gpio_write,
	.open = gpio_open,
	.release = gpio_release,
};

module_init(gpio_init);
module_exit(gpio_exit);

