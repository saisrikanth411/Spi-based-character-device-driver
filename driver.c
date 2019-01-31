#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/module.h>
#include<linux/kdev_t.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()
#include<linux/ioctl.h>
#include<linux/kthread.h>
#include<linux/gpio.h>
#include<linux/spi/spi.h>
#include<linux/spi/spidev.h>
#include<linux/delay.h>

#define MAJOR_NUMBER 153 //major number of the device driver
#define MINOR_NUMBER 0 // minor number of the device driver
#define DRIVER_NAME "spidev" //name of the driver
#define DEVICE_NAME "spi_device" //name of the device
#define DEVICE_CLASS_NAME  "spi_device" //device class name


#define GPIO15 15 // gpio pins of the IO 11, 12, 13
#define GPIO24 24
#define GPIO44 44
#define GPIO72 72
#define GPIO42 42
#define GPIO30 30
#define GPIO46 46


struct spidev_data {                            // spi device driver data
	dev_t devt;
	struct spi_device *spi;
	spinlock_t spi_lock;
	int pattern_buffer[10][8]; // pattern buffer for 10 patterns for the 8 led columns
	unsigned int sequence_buffer[20];
};

static DEFINE_MUTEX(device_list_lock);

static struct spidev_data *spi_data; //pointer to the spi data
static struct class *spi_class;
static unsigned buffer = 4096; //buffer size
static struct spi_message m; // message structure to transfer the data
static unsigned int flag = 0;
uint8_t tx[2];

static struct spi_transfer t = {       // parameters for transferring the data
.tx_buf = tx,
.rx_buf = 0,
.len = 2,
.cs_change = 1,
.bits_per_word = 8,
.speed_hz = 1000000,
.delay_usecs = 1,
};

static int transfer(uint8_t a , uint8_t b)     //transfer function to transfer the values on to the device
{
int status;
tx[0] = a;
tx[1] = b;
gpio_set_value(GPIO15,0);  // setting the value of gpio 15 from 0 to 1 is equivalent to the chip select pin in spidev driver
spi_message_init(&m); //transfer the data
spi_message_add_tail(&t,&m);
status = spi_sync(spi_data->spi,&m);
if(status < 0){
    printk(" Error in sending spi message.\n");
}
gpio_set_value(GPIO15,1);
return status;
}

static int spi_device_open(struct inode *inode,struct file *filp) // open function to which enables access to the spi device driver
{
    gpio_free(GPIO15);
    gpio_free(GPIO24);
    gpio_free(GPIO44);
    gpio_free(GPIO72);
    gpio_free(GPIO42);
    gpio_free(GPIO30);
    gpio_free(GPIO46);


    gpio_request_one(GPIO24,GPIOF_OUT_INIT_LOW,"gpio24");  //setting the MOSI as the output pin
    gpio_set_value(GPIO44,1);
    gpio_set_value(GPIO72,0);
    gpio_request_one(GPIO15,GPIOF_OUT_INIT_LOW,"gpio15");
    gpio_request_one(GPIO42,GPIOF_OUT_INIT_LOW,"gpio42");


    gpio_request_one(GPIO30,GPIOF_OUT_INIT_LOW,"gpio30");
    gpio_set_value(GPIO46,1);

    flag = 0;
    transfer(0x0C,0x01); //transferring the essential values to the 5 registers of max7219 driver
    transfer(0X09,0x00);
    transfer(0x0A,0x0F);
    transfer(0x0B,0x07);
    transfer(0x0F,0x00);

    transfer(0x01,0x00);
    transfer(0x02,0x00);
    transfer(0x03,0x00);
    transfer(0x04,0x00);
    transfer(0x05,0x00);
    transfer(0x06,0x00);
    transfer(0x07,0x00);
    transfer(0x08,0x00);
    printk("led initialised \n");
    return 0;
}

static int spi_device_release(struct inode *inode , struct file *filp) // releases the driver after the functions have been implemented 
{
    flag =0;
    transfer(0x01,0x00); //setting each column of the matrix with 0 value switches off the led
    transfer(0x02,0x00);
    transfer(0x03,0x00);
    transfer(0x04,0x00);
    transfer(0x05,0x00);
    transfer(0x06,0x00);
    transfer(0x07,0x00);
    transfer(0x08,0x00);
    gpio_free(GPIO15);
    gpio_free(GPIO24);
    gpio_free(GPIO44);
    gpio_free(GPIO72);
    gpio_free(GPIO42);
    gpio_free(GPIO30);
    gpio_free(GPIO46);
    printk("led closing \n");
    return 0;
}

int spi_thread(void *data) // thread function which implements in transferring the data on to the buffers
{
    int i;

/*
if(spi_data->sequence_buffer[0][0] == 0 && spi_data->sequence_buffer[0][1] == 0)
	{
		transfer(0x01,0x00);
		transfer(0x02,0x00);
		transfer(0x03,0x00);
		transfer(0x04,0x00);
		transfer(0x05,0x00);
		transfer(0x06,0x00);
		transfer(0x07,0x00);
		transfer(0x08,0x00);
		flag =0;
	}
for(i = 0; i < 10;i++)
{
	for(j =0;j<10;j++)
	{
		if(spi_data->sequence_buffer[i][0] == j)
		{
			if(spi_data->sequence_buffer[i][0] == 0 && spi_data->sequence_buffer[i][1] == 0)
			{
			transfer(0x01,0x00);
			transfer(0x02,0x00);
			transfer(0x03,0x00);
			transfer(0x04,0x00);
			transfer(0x05,0x00);
			transfer(0x06,0x00);
			transfer(0x07,0x00);
			transfer(0x08,0x00);
			flag=0;
			}
			else
			{
			transfer(0x01,spi_data->pattern_buffer[j][0]);
			transfer(0x02,spi_data->pattern_buffer[j][1]);
			transfer(0x03,spi_data->pattern_buffer[j][2]);
			transfer(0x04,spi_data->pattern_buffer[j][3]);
			transfer(0x05,spi_data->pattern_buffer[j][4]);
			transfer(0x06,spi_data->pattern_buffer[j][5]);
			transfer(0x07,spi_data->pattern_buffer[j][6]);
			transfer(0x08,spi_data->pattern_buffer[j][7]);
			msleep(spi_data->sequence_buffer[i][1]);
			}
		}
	}
}
*/

    for (i=0;i<20;i+=2){                       
        if(spi_data->sequence_buffer[i] == 0 && spi_data->sequence_buffer[i+1]==0){  //if the sequence has (0,0) end the sequence
            transfer(0x01,0x00);
            transfer(0x02,0x00);
            transfer(0x03,0x00);
            transfer(0x04,0x00);
            transfer(0x05,0x00);
            transfer(0x06,0x00);
            transfer(0x07,0x00);
            transfer(0x08,0x00);
            break;
        }
        else {
            transfer(0x01,spi_data->pattern_buffer[spi_data->sequence_buffer[i]][0]); //if the sequence contains any  pattern , transfer it to the sequence buffer 
            transfer(0x02,spi_data->pattern_buffer[spi_data->sequence_buffer[i]][1]);
            transfer(0x03,spi_data->pattern_buffer[spi_data->sequence_buffer[i]][2]);
            transfer(0x04,spi_data->pattern_buffer[spi_data->sequence_buffer[i]][3]);
            transfer(0x05,spi_data->pattern_buffer[spi_data->sequence_buffer[i]][4]);
            transfer(0x06,spi_data->pattern_buffer[spi_data->sequence_buffer[i]][5]);
            transfer(0x07,spi_data->pattern_buffer[spi_data->sequence_buffer[i]][6]);
            transfer(0x08,spi_data->pattern_buffer[spi_data->sequence_buffer[i]][7]);
            msleep(spi_data->sequence_buffer[i+1]);
        }
    }
    flag = 0;
    return 0;
}

static ssize_t spi_device_write(struct file *filp,const char *buf,size_t count , loff_t *ppos) // write function which helps the user program to write to the driver
{
    int value;
    int i;
    unsigned int seqbuf[20];
    struct task_struct *kthread;
    if(flag == 1)
    {
        return -EBUSY;
    }
    if(count > buffer)
    {
        return -EMSGSIZE;
    }
    value = copy_from_user(seqbuf,(unsigned int *)buf,sizeof(seqbuf));  // copy the sequence buffer from the user space to the kernel space
    printk("*************************************\n");
    for(i=0;i<20;i++)
    {
        spi_data->sequence_buffer[i] = seqbuf[i];  
        printk("%d\n",seqbuf[i]);
    }
    if(value != 0)
    {
        printk("%d failure", value);
    }
    flag = 1;
    kthread = kthread_run(&spi_thread,(void *)seqbuf,"kthread");  // run the thread function defined above to pass the values on to the sequence buffer
    return value;
}

static long spi_device_ioctl(struct file *filp,unsigned int cmd,unsigned long arg)  // ioctl command which helps in the input/output control of the driver
{
int i=0;
int j=0;
int value;
int kernbuf[10][8];
value = copy_from_user(kernbuf, (int **)arg, sizeof(kernbuf)); // copy the values from user space to the buffer in kernel space
for(i=0;i<10;i++)
{
	for(j=0;j<8;j++)
	{
		spi_data->pattern_buffer[i][j] = kernbuf[i][j]; // pass the 10 commands 
	}
}
return value;
}

static struct file_operations spi_fops = {  //  file operations structure
.owner = THIS_MODULE,
.write = spi_device_write,
.open = spi_device_open,
.unlocked_ioctl = spi_device_ioctl,
.release = spi_device_release
};

static int spi_device_probe(struct spi_device *spi)  //probe function to initialise the driver 
{
int status;
struct device *device;
spi_data = kzalloc(sizeof(*spi_data),GFP_KERNEL);  // allocate kernel space for spi_data 
if(!spi_data)
{
return -ENOMEM;
}
spi_data->spi = spi;
spi_data->devt = MKDEV(MAJOR_NUMBER,MINOR_NUMBER);
device = device_create(spi_class,&spi->dev,spi_data->devt,spi_data,DEVICE_NAME); // create device for the device driver
printk("spi driver probed \n");
return status;
}


static int spi_device_remove(struct spi_device *spi) //remove the spi device 
{
int value = 0;
device_destroy(spi_class,spi_data->devt);
kfree(spi_data);
printk("spi driver released \n");
return value;
}

static struct spi_driver spi_device_driver = { //initialise the driver function
.driver = {
	  .name = DRIVER_NAME,
	  .owner = THIS_MODULE,
	},
	.probe = spi_device_probe,
	.remove = spi_device_remove,
};

static int __init spi_device_init(void) //init module to initiate the driver 
{
int status;
status = register_chrdev(MAJOR_NUMBER,DEVICE_NAME,&spi_fops); //register the device driver with the major and minor number
if(status < 0)
{
printk("device registration failed \n");
}
spi_class = class_create(THIS_MODULE,DEVICE_CLASS_NAME); // creates the spi class for spi device driver
if(IS_ERR(spi_class)) {
	unregister_chrdev(MAJOR_NUMBER,spi_device_driver.driver.name);
	return PTR_ERR(spi_class);
}
status = spi_register_driver(&spi_device_driver);
if(status < 0)
{
	class_destroy(spi_class);
	unregister_chrdev(MAJOR_NUMBER,spi_device_driver.driver.name);
}

printk("driver initialized \n");
return status;
}


static void __exit spi_device_exit(void)  // exit module to exit the driver
{
	spi_unregister_driver(&spi_device_driver);
	class_destroy(spi_class);
	unregister_chrdev(MAJOR_NUMBER,spi_device_driver.driver.name);
	printk("spi driver uninitialized \n");
}

module_init(spi_device_init);
module_exit(spi_device_exit);

MODULE_AUTHOR("Sai Srikanth");
MODULE_DESCRIPTION("SPI Driver");
MODULE_LICENSE("GPL");
