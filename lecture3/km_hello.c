#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENCE("GPL");

static int hello_init(void)
{
	printk(KERN_ALERT "You got Rhino'd");
	return 0;
}

static void hello_exit(void)
{
	printk(KERN_ALERT, "You got Unrhino'd");
}

module_init(hello_init);
module_exit(hello_init);