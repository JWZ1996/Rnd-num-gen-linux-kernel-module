/** 
 * @file    main.c 
 * @author  Jan Wojciech Zembowicz
 * @version 0.1 
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/random.h>
  
#define DEVICE_NAME "JZ_rnd_gen"
#define SUCCESS 0

MODULE_LICENSE("GPL"); 
  
MODULE_AUTHOR("Jan Wojciech Zembowicz"); 
  
MODULE_DESCRIPTION("A simple rnd_gen driver"); 
  
MODULE_VERSION("0.1"); 

void test(void);

dev_t my_dev = 0;
struct cdev *rnd_gen_cdev = NULL;
static struct class *rnd_gen_class = NULL;

static void unregister_all(void);
static int dev_open(struct inode*, struct file*);
static int dev_release(struct inode*, struct file*);
static ssize_t dev_read(struct file*, char*, size_t, loff_t*);
static ssize_t dev_write(struct file*, const char*, size_t, loff_t*);

static struct file_operations fops = {
   .owner = THIS_MODULE,
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
};

static int major;

static void unregister_all()
{
  printk(KERN_ALERT "DO WIDZENIA - rnd_gen exited.\n");

  // Deleting device from class
  if(my_dev && rnd_gen_class)
  {
    device_destroy(rnd_gen_class,my_dev);
  }

  if(rnd_gen_cdev)
  {
  	cdev_del(rnd_gen_cdev);
  	rnd_gen_cdev=NULL;
  }
  

  // Unregistering dev no
  unregister_chrdev_region(my_dev, 1);

  // Class destruction
  if(rnd_gen_class)
  {
  	printk(KERN_ALERT "CLASS DESTROYED.\n");
    class_destroy(rnd_gen_class);
    rnd_gen_class=NULL;
  }
}

static int __init start(void)
{
	int res;
    printk(KERN_INFO "DZIEN DOBRY - rnd_gen module has been loaded.");

	// Creating device class for udev system
	rnd_gen_class = class_create(THIS_MODULE, "rnd_gen_class");
	if (IS_ERR(rnd_gen_class))
	{
	    printk(KERN_ERR "Error creating rnd_gen_class class.\n");
	    res=PTR_ERR(rnd_gen_class);
	    goto err1;
	}  

	// Device number allocation
	res=alloc_chrdev_region(&my_dev, 0, 1, DEVICE_NAME);
	if(res) {
	    printk ("<1>Alocation of the device number for %s failed\n",DEVICE_NAME);
	    goto err1; 
	};  

	rnd_gen_cdev = cdev_alloc();
	if (rnd_gen_cdev==NULL) {
	    printk (KERN_ERR "Allocation of cdev for %s failed\n", DEVICE_NAME);
	    res = -ENODEV;
	    goto err1;
	};

	rnd_gen_cdev->ops = &fops;
	rnd_gen_cdev->owner = THIS_MODULE;

	// Char device addition
	res=cdev_add(rnd_gen_cdev, my_dev, 1);
	if(res) {
	    printk (KERN_ERR "Registration of the device number for %s failed\n",
	            DEVICE_NAME);
	    goto err1;
	};

	// Device creation
	device_create(rnd_gen_class,NULL,my_dev,NULL,"my_dev%d",MINOR(my_dev));
	printk (KERN_ALERT "Registeration succesful. The major device number %s is %d.\n",
		  DEVICE_NAME,
		  MAJOR(my_dev));
	return SUCCESS;

	err1:
	  unregister_all();
	  return res;
}


static void __exit end(void) {
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "DO WIDZENIA - rnd_gen module has been unloaded\n");
}

static int dev_open(struct inode *inodep, struct file *filep)
{
   printk(KERN_INFO "rnd_gen DEVICE OPENED\n");
   return SUCCESS;
}

static ssize_t dev_write(struct file *filep, const char *buffer,size_t len, loff_t *offset)
{

   printk(KERN_ALERT "rnd_gen device is read only\n");
   return -EFAULT;
}

static int dev_release(struct inode *inodep, struct file *filep) {
   printk(KERN_INFO "rnd_gen DEVICE CLOSED \n");
   return SUCCESS;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    
	// Previous solution:
	// 0. Global pointer to byte array
	// 1. dev_open allocates kmemory
	// 2. device_write allocates the requested number of bytes and fills arr with random bytes
	// 3. device_read reads and copies bytes
	// 4. device_release frees kmemory

	// Current solution:
	// dev_read finds out how big (in bytes - size_t len) the number is requested, allocates virtual memory
	// then copies to user space and frees virtual memory.

    int errors = 0;
    char *packet = (char*)vmalloc(sizeof(char) * len);

    if (!packet){
    	printk(KERN_ALERT "dev_read: memory allocation failed");
    	return -EFAULT;
    }

    get_random_bytes( packet, len );

    errors = copy_to_user(buffer, packet, len);

    vfree(packet);
    return errors == 0 ? len : -EFAULT;

}

  
module_init(start);
module_exit(end);

