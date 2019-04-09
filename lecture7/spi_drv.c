#include <linux/cdev.h>   // cdev_add, cdev_init
#include <linux/uaccess.h>  // copy_to_user
#include <linux/module.h> // module_init, GPL
#include <linux/spi/spi.h> // spi_sync,
#include <linux/gpio.h>

#define MAXLEN 32
#define MODULE_DEBUG 1   // Enable/Disable Debug messages

#define LDAC 5

/* Char Driver Globals */
static struct spi_driver spi_drv_spi_DAC;
static struct spi_driver spi_drv_spi_ADC;
struct file_operations spi_drv_fops;
static struct class *spi_drv_class;
static dev_t devno;
static struct cdev spi_drv_cdev;



/* Definition of SPI devices */
struct Myspi {
  struct spi_device *spi; // Pointer to SPI device
  int channel;
  int gain;
  int OE;
  int value;            // channel, ex. adc ch 0
};
/* Array of SPI devices */
/* Minor used to index array */
struct Myspi spi_devs[4];
const int spi_devs_len = 4;  // Max nbr of devices
static int spi_devs_cnt = 0; // Nbr devices present

/* Macro to handle Errors */
#define ERRGOTO(label, ...)                     \
  {                                             \
    printk (__VA_ARGS__);                       \
    goto label;                                 \
  } while(0)

/**********************************************************
 * CHARACTER DRIVER METHODS
 **********************************************************/

/*
 * Character Driver Module Init Method
 */

int write_to_DAC(struct Myspi spi_dev);
void toggleLDAC(void);
int read_from_ADC(struct Myspi spi_dev, int *value);
int adc_convert(int adc_data, float *voltage);

static int __init spi_drv_init(void)
{
  int err=0;

  printk("spi_drv driver initializing\n");

  /* Allocate major number and register fops*/
  err = alloc_chrdev_region(&devno, 0, 255, "spi_drv driver");
  if(MAJOR(devno) <= 0)
    ERRGOTO(err_no_cleanup, "Failed to register chardev\n");
  printk(KERN_ALERT "Assigned major no: %i\n", MAJOR(devno));

  cdev_init(&spi_drv_cdev, &spi_drv_fops);
  err = cdev_add(&spi_drv_cdev, devno, 255);
  if (err)
    ERRGOTO(err_cleanup_chrdev, "Failed to create class");

  /* Polulate sysfs entries */
  spi_drv_class = class_create(THIS_MODULE, "spi_drv_class");
  if (IS_ERR(spi_drv_class))
    ERRGOTO(err_cleanup_cdev, "Failed to create class");

  /* Register SPI Driver */
  /* THIS WILL INVOKE PROBE, IF DEVICE IS PRESENT!!! */
  err = spi_register_driver(&spi_drv_spi_DAC);
  if(err)
    ERRGOTO(err_cleanup_class, "Failed SPI Registration\n");

  err = spi_register_driver(&spi_drv_spi_ADC);
  if(err)
    ERRGOTO(err_cleanup_class, "Failed SPI Registration\n");

  /* Success */
  return 0;

  /* Errors during Initialization */
 err_cleanup_class:
  class_destroy(spi_drv_class);

 err_cleanup_cdev:
  cdev_del(&spi_drv_cdev);

 err_cleanup_chrdev:
  unregister_chrdev_region(devno, 255);

 err_no_cleanup:
  return err;
}

/*
 * Character Driver Module Exit Method
 */
static void __exit spi_drv_exit(void)
{
  printk("spi_drv driver Exit\n");

  spi_unregister_driver(&spi_drv_spi_DAC);
  spi_unregister_driver(&spi_drv_spi_ADC);
  class_destroy(spi_drv_class);
  cdev_del(&spi_drv_cdev);
  unregister_chrdev_region(devno, 255);
}

/*
 * Character Driver Write File Operations Method
 */
ssize_t spi_drv_write(struct file *filep, const char __user *ubuf,
                      size_t count, loff_t *f_pos)
{
  int minor, len, value;
  char kbuf[MAXLEN];

  minor = iminor(filep->f_inode);

  printk(KERN_ALERT "Writing to spi_drv [Minor] %i \n", minor);

  /* Limit copy length to MAXLEN allocated andCopy from user */
  len = count < MAXLEN ? count : MAXLEN;
  if(copy_from_user(kbuf, ubuf, len))
    return -EFAULT;

  /* Pad null termination to string */
  kbuf[len] = '\0';

  if(MODULE_DEBUG)
    printk("string from user: %s\n", kbuf);

  /* Convert sting to int */
  sscanf(kbuf,"%i", &value);
  if(MODULE_DEBUG)
    printk("value %i\n", value);

  //spi_devs[minor].gain =    (value & 0x2000) >> 8;
  //spi_devs[minor].OE =      (value & 0x1000) >> 8;
  spi_devs[minor].value =     value / 16;

  //printk("Masked studd in struct: channel: %d gain: %d OE: %d value: %d\n", spi_devs[minor].channel, spi_devs[minor].gain, spi_devs[minor].OE, spi_devs[minor].value);

  /* Legacy file ptr f_pos. Used to support
   * random access but in char drv we dont!
   * Move it the length actually  written
   * for compability */

  write_to_DAC(spi_devs[minor]);

  *f_pos += len;

  /* return length actually written */
  return len;
}

/*
 * Character Driver Read File Operations Method
 */
ssize_t spi_drv_read(struct file *filep, char __user *ubuf,
                     size_t count, loff_t *f_pos)
{
  int minor, len;
  char resultBuf[MAXLEN];
  static int result = 0;
  char converted[8];

  memset(converted, 0, sizeof(converted));

  minor = iminor(filep->f_inode);

  /*
    Provide a result to write to user space
  */

  read_from_ADC(spi_devs[minor], &result);

  if(MODULE_DEBUG)
    printk(KERN_ALERT "%s-%i read: %i\n",
           spi_devs[minor].spi->modalias, spi_devs[minor].channel, result);

  /* Convert integer to string limited to "count" size. Returns
   * length excluding NULL termination */
  //sprintf(converted, "%d", result);

  len = snprintf(resultBuf, count, "%d\n", result);

  /* Append Length of NULL termination */
  len++;
  //len = count < MAXLEN ? count : MAXLEN;

  /* Copy data to user space */
  if(copy_to_user(ubuf, resultBuf, len))
    return -EFAULT;

  /* Move fileptr */
  *f_pos += len;

  return len;
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
/*
 * Character Driver File Operations Structure
 */
struct file_operations spi_drv_fops =
  {
    .owner   = THIS_MODULE,
    .write   = spi_drv_write,
    .read    = spi_drv_read,
    .open    = gpio_open,
    .release = gpio_release,
  };

/**********************************************************
 * LINUX DEVICE MODEL METHODS (spi)
 **********************************************************/

/*
 * spi_drv Probe
 * Called when a device with the name "spi_drv" is
 * registered.
 */
static int spi_drv_probe(struct spi_device *sdev)
{
  int err = 0;
  struct device *spi_drv_device;
  struct device *spi_drv_dac_LDAC;
  char name[4];

  memset(name, 0, sizeof(name)); 

  printk(KERN_DEBUG "New SPI device: %s using chip select: %i\n",
         sdev->modalias, sdev->chip_select);

  /* Check we are not creating more
     devices than we have space for */
  if (spi_devs_cnt > spi_devs_len) {
    printk(KERN_ERR "Too many SPI devices for driver\n");
    return -ENODEV;
  }

  /* Configure bits_per_word, always 8-bit for RPI!!! */
  sdev->bits_per_word = 8;
  spi_setup(sdev);

  /* Create devices, populate sysfs and
     active udev to create devices in /dev */
  if(sdev->chip_select == 1)
  {
    err = gpio_request(LDAC, "LDAC");
    err = gpio_direction_output(LDAC, 1);
    spi_drv_dac_LDAC = device_create(spi_drv_class, NULL, MKDEV(MAJOR(devno), 255), NULL, "LDAC");
    sprintf(name, "DAC");

    //name = "DAC";
  }
  else sprintf(name, "ADC");

  for (int j =0 ; j<2;j++)
{
  /* We map spi_devs index to minor number here */
  spi_drv_device = device_create(spi_drv_class, NULL,
                                 MKDEV(MAJOR(devno), spi_devs_cnt),
                                 NULL, "%s_ch.%d", name, j);
  if (IS_ERR(spi_drv_device))
    printk(KERN_ALERT "FAILED TO CREATE DEVICE\n");
  else
    printk(KERN_ALERT "Using spi_devs%i on major:%i, minor:%i\n",
           spi_devs_cnt, MAJOR(devno), spi_devs_cnt);

  /* Update local array of SPI devices */
  spi_devs[spi_devs_cnt].spi = sdev;
  spi_devs[spi_devs_cnt].channel = (0x00 + j) << 7; // channel address
  ++spi_devs_cnt;
 }
  return err;
}

/*
 * spi_drv Remove
 * Called when the device is removed
 * Can deallocate data if needed
 */
static int spi_drv_remove(struct spi_device *sdev)
{
  //int its_minor = 0;

  printk (KERN_ALERT "Removing spi device\n");
  for(int i = spi_devs_cnt; i >= 0; i--)
  {
  /* Destroy devices created in probe() */
  device_destroy(spi_drv_class, MKDEV(MAJOR(devno), i));
  }
  device_destroy(spi_drv_class, MKDEV(MAJOR(devno), 255));
  return 0;
}



/*
 * spi Driver Struct
 * Holds function pointers to probe/release
 * methods and the name under which it is registered
 */
static const struct of_device_id of_spi_drv_spi_DAC_match[] = {
  { .compatible = "spi_drv_DAC", }, {},
};

static const struct of_device_id of_spi_drv_spi_ADC_match[] = {
  { .compatible = "spi_drv_ADC", }, {},
};

static struct spi_driver spi_drv_spi_DAC = {
  .probe      = spi_drv_probe,
  .remove           = spi_drv_remove,
  .driver     = {
    .name   = "spi_drv_DAC",
    .bus    = &spi_bus_type,
    .of_match_table = of_spi_drv_spi_DAC_match,
    .owner  = THIS_MODULE,
  },
};

static struct spi_driver spi_drv_spi_ADC = {
  .probe      = spi_drv_probe,
  .remove           = spi_drv_remove,
  .driver     = {
    .name   = "spi_drv_ADC",
    .bus    = &spi_bus_type,
    .of_match_table = of_spi_drv_spi_ADC_match,
    .owner  = THIS_MODULE,
  },
};

/**********************************************************
 * GENERIC LINUX DEVICE DRIVER STUFF
 **********************************************************/

/*
 * Assignment of module init/exit methods
 */
module_init(spi_drv_init);
module_exit(spi_drv_exit);

/*
 * Assignment of author and license
 */
MODULE_AUTHOR("Peter Hoegh Mikkelsen <phm@ase.au.dk>");
MODULE_LICENSE("GPL");


int write_to_DAC(struct Myspi spi_dev)
{
  struct spi_transfer t[2];
  struct spi_message m;
  u8 cmd[2];
  cmd[0] = spi_dev.channel | 0x10 | ((spi_dev.value & 0xf0) >> 4);
  cmd[1] = (u8)(spi_dev.value << 4) & 0xf0;

  memset(&t, 0, sizeof(t)); 
  spi_message_init(&m);
  m.spi = spi_dev.spi;

  if(MODULE_DEBUG)
    printk(KERN_DEBUG "MCP4802: Writing %d to channel %d\n", spi_dev.value, spi_dev.channel); 

  printk("cmd values: 1= %u : 0= %u\n", cmd[1], cmd[0]);
  /* Configure tx/rx buffers */
  t[0].tx_buf = &cmd[0];
  t[0].rx_buf = NULL;
  t[0].len = 1;
  spi_message_add_tail(&t[0], &m);

  t[1].tx_buf = &cmd[1];
  t[1].rx_buf = NULL;
  t[1].len = 1;
  spi_message_add_tail(&t[1], &m);

  /* Transmit SPI Data (blocking) */
  int err;

  err = spi_sync(m.spi, &m);
  if(err != 0) return -EFAULT;
  else toggleLDAC(); 
  return 0;
}

void toggleLDAC(void)
{
  int time = 2;
  gpio_set_value(LDAC, 0);
  for(int i = 0; i < 100000000; i ++) time /= 3;
}

int read_from_ADC(struct Myspi spi_dev, int *value)
{
  struct spi_transfer t[3];
  struct spi_message m;
  u8 cmd[2];
  u8 data[2];
  cmd[0] = 0x01;
  cmd[1] = 0x80 | ((spi_dev.channel >> 1) & 0x40) | 0x20;

  memset(&t, 0, sizeof(t)); 
  spi_message_init(&m);
  m.spi = spi_dev.spi;

  if(MODULE_DEBUG)
    printk(KERN_DEBUG "MCP3202: Getting value from channel %d\n", spi_dev.channel); 

  printk("cmd values: 1= %u : 0= %u\n", cmd[1], cmd[0]);

  /* Configure tx/rx buffers */
  t[0].tx_buf = &cmd[0];
  t[0].rx_buf = NULL;
  t[0].len = 1;
  spi_message_add_tail(&t[0], &m);

  t[1].tx_buf = &cmd[1];
  t[1].rx_buf = &data[0];
  t[1].len = 1;
  spi_message_add_tail(&t[1], &m);

  t[2].tx_buf = NULL;
  t[2].rx_buf = &data[1];
  t[2].len = 1;
  spi_message_add_tail(&t[2], &m);

  int err;

  err = spi_sync(m.spi, &m);
  if(err != 0) return -EFAULT;
  int temp = 0;
  temp = ((data[0] & 0x0f) << 8) | data[1]; 

  printk("RAW adc data: %d\n", temp);

  *value = temp;
  
  return 0;
}
