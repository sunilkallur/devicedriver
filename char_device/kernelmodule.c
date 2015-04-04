#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/fs.h>//file operations structure -which allows to open/close,read/write to device
#include<linux/cdev.h> // this is a char driver; makes cdev aavailable
#include<linux/semaphore.h> //used to access semaphores; synchronization behaviors
#include<asm/uaccess.h> //copy to user; copy from user

// creating the structure for our device
struct my_device{
	char data[100];
	struct semaphore sem;
} virtual_device; 

// to register our device need a cdev object and some other variables

struct cdev *mcdev; // m stands my
int major_number; // will store our major number
int ret; //will be used to hold return values of functions
dev_t dev_num; //will hold major number that kernel gives us

#define DEVICE_NAME "sunildevice"

// called on device_file_open
//innode reference to the file on disk and contains information about that file struct file represents an abr=stract open file

int device_open(struct inode *inode,struct file *filp) {
	// only allow one process to open this device by using a semaphore as mututal exclusive lock mutex

	if(down_interruptible(&virtual_device.sem)!= 0){
		printk(KERN_ALERT "sunilcode: could not lock device during open");
		return -1;

	}

	printk(KERN_INFO "sunilcode: opened device");
	return 0;
}


ssize_t device_read(struct file* filp,char* bufStoreData,size_t bufCount,loff_t* curOffset){

	//take data from kernel space(device) to user_space (process)
	//copy_to_user (destination,source,sizeToTransfer)
	printk(KERN_INFO "sunilcode: Reading from Device");
	ret = copy_to_user(bufStoreData,virtual_device.data,bufCount);
	return ret;
}



ssize_t device_write(struct file* filp,const char* bufSourceData,size_t bufCount,loff_t* curOffset){

	//send data from user to kernel
	//copy from user (dest,source,count)
	printk(KERN_INFO "sunilcode: writing to  Device");
	ret = copy_from_user(virtual_device.data,bufSourceData,bufCount);
	return ret;
}

// called upon user close
int device_close(struct inode *inode,struct file *filp){
	// by claling up, which is opposite of down for semaphore, we release the mutex that we obtained at device open; this has the effect of allowing other process to use the device now

	up(&virtual_device.sem);
	printk(KERN_INFO "sunilcode: closed device");
	return 0;
}





// Tell the kernel which functions to call when user operates on our device file
struct file_operations fops = {

	.owner = THIS_MODULE, // prevent unloading of this module when operations are in use
	.open = device_open, //points to the method to call when opening the device
	.release = device_close, // points to the method to call when closing the device
	.write = device_write, // points to the method to call when writing to the device
	.read = device_read // points to the method to call when reading from the device 

}; 


static int driver_entry(void){
	// register our device with the system; a 2 step process
	//step 1: use dynamic allocation to assign our device
	// a major number-- alloc_chrdev_region(dev_t*,uint fminor,uint count,char* name)
	ret = alloc_chrdev_region(&dev_num,0,1,DEVICE_NAME);
	if(ret<0) {
		// at times kernel functions return negatives,there is an error
		printk(KERN_ALERT "sunilcode: failed to allocate a amajor number");
		return ret;
	}

	major_number = MAJOR(dev_num); //extracts the major number and store in our variable (MACRO)
	printk(KERN_INFO "sunilcode: major number is %d",major_number);
	printk(KERN_INFO "\tuse \"mknod /dev/%s c %d 0\" for device file",DEVICE_NAME,major_number);

	//step (2)

	mcdev = cdev_alloc(); // create our cdev structure, initialized our cdev
	mcdev->ops = &fops; // struct file_operation
	mcdev->owner = THIS_MODULE;

	//now that we create cdev,we have to add it to the kernel
	//int cdev_add(struct cdev* dev,dev_t num,unsigned intcount)
	ret = cdev_add(mcdev,dev_num,1);
	if(ret<0) {
		//always check errors
		printk(KERN_ALERT "sunilcode: unable to add cdev to kernel");
		return ret;
	}
	// initialize our semaphore

	sema_init(&virtual_device.sem,1); //initial value of one

	return 0; 
}

static void driver_exit(void){
	// unregister everything in reverse order
	cdev_del(mcdev);

	unregister_chrdev_region(dev_num,1);
	printk(KERN_ALERT "sunilcode: unloaded module");
}

//inform the kernel where to start and stop with our module/driver
module_init(driver_entry);
module_exit(driver_exit);
