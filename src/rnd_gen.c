/** 
 * @file    main.c 
 * @author  Jan Wojciech Zembowicz
 * @version 0.1 
*/
/** 
KONSULTACJE:

DODAC
IOCTL:

Praca:
ARM
RTOS
mikrokotrolery
free rtos






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
#include <linux/delay.h>



#define BYTES_IN_TIME 11
#define NUMBER_OF_BYTES 12


  
#define DEVICE_NAME "JZ_rnd_gen"
#define SUCCESS 0
#define cores_number 1

MODULE_LICENSE("GPL"); 
  
MODULE_AUTHOR("Jan Wojciech Zembowicz"); 
  
MODULE_DESCRIPTION("A simple rnd_gen driver"); 
  
MODULE_VERSION("0.1"); 

// IOCTL types
struct ioctl_packet
{
	size_t byte_count;
	size_t time_ms_bound;
	char*  bytes;
};


struct generator_t 
{
	dev_t dev_no;
	struct cdev *cdev;
	struct mutex lock;
};




struct generator_t core = {
	.dev_no = 0,
	.cdev = NULL,
};


static struct class *rnd_gen_class = NULL;

struct generator_t* get_free_core(void);
char get_random_byte(struct generator_t* both);


static void unregister_all(void);
static int dev_open(struct inode*, struct file*);
static int dev_release(struct inode*, struct file*);
static ssize_t dev_read(struct file*, char*, size_t, loff_t*);
static ssize_t dev_write(struct file*, const char*, size_t, loff_t*);
long dev_ioctl (struct file *filp, unsigned int cmd, unsigned long arg);

static struct file_operations fops = {
   .owner = THIS_MODULE,
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
   .unlocked_ioctl = dev_ioctl
};

static void unregister_all()
{
  printk(KERN_ALERT "DO WIDZENIA - rnd_gen exited.\n");

  // Deleting device from class
  if(core.dev_no && rnd_gen_class)
  {
    device_destroy(rnd_gen_class,core.dev_no);
  }

  // Char device removal
  if(core.cdev)
  {
  	cdev_del(core.cdev);
  	core.cdev = NULL;
  }


  // Unregistering dev no
  unregister_chrdev_region(core.dev_no, 1);

  // Class destruction
  if(rnd_gen_class)
  {
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
	    res = PTR_ERR(rnd_gen_class);
	    goto err1;
	}  

	// Device number allocation
	res=alloc_chrdev_region(&core.dev_no, 0, 1, DEVICE_NAME);
	if(res) {
	    printk ("Alocation of the device number for %s failed\n",DEVICE_NAME);
	    goto err1; 
	};  

	core.cdev = cdev_alloc();

	if (core.cdev==NULL) {
	    printk (KERN_ERR "Allocation of cdev for %s failed\n", DEVICE_NAME);
	    res = -ENODEV;
	    goto err1;
	};

	core.cdev->ops = &fops;
	core.cdev->owner = THIS_MODULE;

	// Char device addition
	res=cdev_add(core.cdev, core.dev_no, 1);
	if(res) {
	    printk (KERN_ERR "Registration of the device number for %s failed\n",
	            DEVICE_NAME);
	    goto err1;
	};

	// Device creation
	device_create(rnd_gen_class,NULL,core.dev_no,NULL,"core.dev_no%d",MINOR(core.dev_no));
	printk (KERN_ALERT "Registeration succesful. The major device number %s is %d.\n",
		  DEVICE_NAME,
		  MAJOR(core.dev_no));

	// Mutex init
    mutex_init(&core.lock);


	return SUCCESS;

	err1:
	  unregister_all();
	  return res;
}


static void __exit end(void) {
	unregister_all();
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
	// 2. device_ioctl allocates the requested number of bytes and fills arr with random bytes
	// 3. device_read reads and copies bytes
	// 4. device_release frees kmemory

	// Current solution:
	// dev_read finds out how big (in bytes - size_t len) the number is requested, allocates virtual memory
	// then copies to user space and frees virtual memory.

    int errors = 0;
    int i;
    char *packet = (char*)vmalloc(sizeof(char) * len);

    if (!packet){
    	printk(KERN_ALERT "dev_read: memory allocation failed");
    	return -EFAULT;
    };

    mutex_lock(&core.lock);

    // Delay simulation
    for(i = 0; i < len; i++)
    {
    	packet[i] = get_random_byte(get_free_core());
    }

    errors = copy_to_user(buffer, packet, len);

    vfree(packet);
    mutex_unlock(&core.lock);

    return errors == 0 ? len : -EFAULT;
}


long dev_ioctl (struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct ioctl_packet packet;
	char* rnd_byte_arr;
	int i;
	int ret_val;

	i = 0;

	switch (cmd) {
		
	
   	case NUMBER_OF_BYTES: 

   		ret_val = copy_from_user(&packet,(struct ioctl_packet*)arg,sizeof(struct ioctl_packet));
		if (ret_val){
           printk(KERN_WARNING "COPY_FROM_USER failed !");
           return -1;
		}


   		// v. mem. allocation
   		if((rnd_byte_arr = vmalloc(packet.byte_count)) == NULL )
   		{
   			printk(KERN_ERR "VMem. allocation fail");
   			return -1;
   		}

   		// // Byte array fill
   		for( i = 0; i < packet.byte_count; i++ )
   		{
   			rnd_byte_arr[i] = get_random_byte(get_free_core());
   			
   		}

   		printk(KERN_ERR "WYLOSOWANO: %s\n", packet.bytes);

   		// Copying struct
   		ret_val = copy_to_user((struct ioctl_packet*)arg,&packet,sizeof(struct ioctl_packet));
   		if (ret_val){
           printk(KERN_ERR "STRUCT COPY_TO_USER failed !");
           return -1;
		}

		// Copying bytes
   		ret_val = copy_to_user(packet.bytes,rnd_byte_arr,packet.byte_count);
   		if (ret_val){
          printk(KERN_ERR "BYTE ARR COPY_TO_USER failed !");
          return -1;
		}

   		vfree(rnd_byte_arr);
    	return NUMBER_OF_BYTES;
   
   	default:
   		// Wrong command
   		return -1; 
 	} // END OF SWITCH
}
char get_random_byte(struct generator_t* both)
{
	char r;

	mutex_lock(&both->lock);
	get_random_bytes( &r, 1 );

	// Simulates delay

	mutex_unlock(&both->lock);
	mdelay(500);
	return r;
}
struct generator_t* get_free_core()
{
	return &core;
}

  
module_init(start);
module_exit(end);


