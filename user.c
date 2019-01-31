#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <linux/input.h>
#include<pthread.h>
#include<linux/spi/spidev.h>
#include<stdint.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include<poll.h>

#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define MAX_BUF 64
#define SPI_DEVICE_NAME "/dev/spi_device"

unsigned long long int rdtsc(void) // RTDSC FUNCTION DETERMINES THE TIME STAMP COUNTER AT A SPECIFIC INSTANT
{
   unsigned a, d;
   __asm__ volatile("rdtsc" : "=a" (a), "=d" (d));
   return ((unsigned long long)a) | (((unsigned long long)d) << 32);;
}



int gpio_export(unsigned int gpio)  //exports the gpio pins to the board
{
	int fd, len;
	char buf[MAX_BUF];

	fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}

	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);

	return 0;
}

/****************************************************************
 * gpio_unexport
 ****************************************************************/
int gpio_unexport(unsigned int gpio) // unexports the gpio pins to the board
{
	int fd, len;
	char buf[MAX_BUF];

	fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}

	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);
	return 0;
}

/****************************************************************
 * gpio_set_dir
 ****************************************************************/
int gpio_set_dir(unsigned int gpio, unsigned int out_flag) // sets the direction of any gpio pin such as an input or output
{
	int fd, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/direction", gpio);

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/direction");
		return fd;
	}
 	write(fd, buf, len);
	if (out_flag)
		write(fd, "out", 3);
	else
		write(fd, "in", 2);

	close(fd);
	return 0;
}

/****************************************************************
 * gpio_set_value
 ****************************************************************/
int gpio_set_value(unsigned int gpio, unsigned int value) // sets the value of any gpio pin as 0 or 1 
{
	int fd, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/set-value");
		return fd;
	}
 	write(fd, buf, len);
	if (value)
		write(fd, "1", 1);
	else
		write(fd, "0", 1);

	close(fd);
	return 0;
}

/****************************************************************
 * gpio_get_value
 ****************************************************************/
int gpio_get_value(unsigned int gpio, unsigned int *value) // gets the value of gpio pin
{
	int fd, len;
	char buf[MAX_BUF];
	char ch;

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

	fd = open(buf, O_RDONLY);
	if (fd < 0) {
		perror("gpio/get-value");
		return fd;
	}
 	write(fd, buf, len);
	read(fd, &ch, 1);
	write(fd, buf, len);
	if (ch != '0') {
		*value = 1;
	} else {
		*value = 0;
	}

	close(fd);
	return 0;
}


/****************************************************************
 * gpio_set_edge
 ****************************************************************/

int gpio_set_edge(unsigned int gpio, char *edge) // set the edge of the gpio pin as rising or falling
{
	int fd, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/edge", gpio);

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/set-edge");
		return fd;
	}
 	write(fd, buf, len);
	write(fd, edge, strlen(edge) + 1);
	close(fd);
	return 0;
}

/****************************************************************
 * gpio_fd_open  for value
 ****************************************************************/

int gpio_fd_open(unsigned int gpio)   
{
	int fd, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

	fd = open(buf, O_RDONLY | O_WRONLY| O_NONBLOCK );
	if (fd < 0) {
		perror("gpio/fd_open");
	}
	write(fd, buf, len);
	return fd;
}

/****************************************************************
 * gpio_fd_open_read  for value
 ****************************************************************/

int gpio_fd_open_read(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

	fd = open(buf, O_RDONLY | O_NONBLOCK );
	if (fd < 0) {
		perror("gpio/fd_open");
	}
	write(fd, buf, len);
	return fd;
}
/****************************************************************
 * gpio_fd_open_edge
 ****************************************************************/

int gpio_fd_open_edge(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/edge", gpio);

	fd = open(buf, O_RDONLY | O_WRONLY | O_NONBLOCK );
	if (fd < 0) {
		perror("gpio/fd_open_edge");
	}
	write(fd, buf, len);
	return fd;
}

/****************************************************************
 * gpio_fd_close
 ****************************************************************/

int gpio_fd_close(int fd)
{
	return close(fd);
}

int mux_gpio_set(unsigned int gpio, unsigned int value)
{
	gpio_export(gpio);
	gpio_set_dir(gpio, 1);
	gpio_set_value(gpio, value);

	return 0;
}

void *io_init(void *arg) // initialise and setting the gpio pins for the ultrasonic sensor for IO2,IO3 
{
    gpio_export(13);
	gpio_set_dir(13,1);
	gpio_set_value(13,0);

	gpio_export(34);
	gpio_set_dir(34,1);
	gpio_set_value(34,0);


	gpio_export(77);
	gpio_set_value(77,0);

	gpio_export(14);
	gpio_set_dir(14,0);

	gpio_export(16);
	gpio_set_dir(16,0);
	gpio_set_value(16,1);

	gpio_export(76);
	gpio_set_value(76,0);

	gpio_export(64);
	gpio_set_value(64,0);	
    printf("gpio values set \n");
    return NULL;
}

void *echo_init(void *arg) // thread function to obtain the distance from the sensor and the object using the ultrasonic sensor 
{
struct pollfd fdset; //poll function to obtain the time at the rising and falling edge of the echo signal
    int nfds =1,gpio13_fd,timeout = 300,res,gpio14_fd,gpio14_ed;
    unsigned int rise=0,fall=0,dist=0;
    char buf;

    gpio13_fd = open(SYSFS_GPIO_DIR "/gpio13/value",O_RDWR); // open the trigger and echo pins of the ultrasonic sensor
    gpio14_fd = open(SYSFS_GPIO_DIR "/gpio14/value",O_RDWR);
    gpio14_ed = open(SYSFS_GPIO_DIR "/gpio14/edge",O_RDWR);
    fdset.fd = gpio14_fd;
    fdset.events = POLLPRI | POLLERR;
    while(1)
    {
        lseek(gpio14_fd,0,SEEK_SET);
        write(gpio14_ed,"rising",sizeof("rising"));
        write(gpio13_fd,"0",1);
	usleep(5);
        if(write(gpio13_fd,"1",1) < 0);
	{
		printf("error");
	}
        usleep(12);    // keeping the trigger ON for given seconds
        write(gpio13_fd,"0",1);

        res = poll(&fdset,nfds,timeout);

        if(res < 0){
            printf("\nPolling Failed.");
        } else {
            if(fdset.revents & POLLPRI)
            {
                rise = rdtsc();//poll at the rising edge
                read(gpio14_fd,&buf,1);
            }
            lseek(gpio14_fd,0,SEEK_SET);
            write(gpio14_ed,"falling",7);
            res = poll(&fdset,nfds,timeout);

            if(res < 0){
                printf("\nPolling Failed.");
            } else {
                if(fdset.revents & POLLPRI){
                    fall = rdtsc();//poll at the falling edge
                    read(gpio14_fd,&buf,1);
                    dist = ((fall - rise) * 34) / 800000ul; //obtaining the difference of rise and fall time
                    printf("The measured dist - %d\n",dist);
                    usleep(100000);
                }

            }

        }

}
}


void *thread_transmit_spi_user(void *data) // thread to transmit the 10 patterns 
{
	int retValue,fd;

    int patternBuffer [10][8] = {
		{ 0x00, 0xB8, 0xFC, 0x74, 0x5C, 0x7E, 0x3A, 0x00 },  //storing the patterns in a buffer to transmit
		{ 0x02, 0x03, 0x45, 0x7D, 0x7D, 0x43, 0x02, 0x00 }, 
		{ 0x30, 0x70, 0x4D, 0x45, 0x60, 0x20, 0x00, 0x00 }, 
		{ 0x0E, 0x9F, 0x91, 0xB1, 0xFB, 0x4A, 0x00, 0x00 }, 
		{ 0x2B, 0x2F, 0xFC, 0xFC, 0x2F, 0x2B, 0x00, 0x00 }, 
		{ 0x80, 0xFC, 0x7C, 0x20, 0x20, 0x3C, 0x1C, 0x00 }, 
		{ 0xFC, 0xFE, 0x2A, 0x2A, 0x3E, 0x14, 0x00, 0x00 }, 
		{ 0x00, 0x12, 0x3B, 0x6D, 0x6D, 0x3B, 0x12, 0x00 }, 
		{ 0x3C, 0x42, 0xBD, 0x95, 0x95, 0xA9, 0x42, 0x3C }, 
		{ 0x68, 0x7E, 0x7F, 0x49, 0x43, 0x66, 0x20, 0x00 },
	};
	unsigned int sequenceBuffer[20] = {1, 100, 0, 100, 1, 200, 3, 300, 4, 400, 5, 500, 6, 600, 7, 700, 8, 800, 0, 0}; // sequence of the pattern and time delay
	printf("thread_transmit_spi Start\n");

	fd = open(SPI_DEVICE_NAME, O_RDWR); // open the designed driver
	if(fd < 0)
	{
		printf("Can not open device file fd_spi.\n");
		return 0;
	}
	else
	{
		printf("fd_spi device opened succcessfully.\n");
	}

	retValue = ioctl(fd, 1, (int **) patternBuffer); // ioctl command to pass the patterns to the driver
	while(1)
	{
		retValue = write(fd,(unsigned int *)sequenceBuffer,sizeof(sequenceBuffer));
		usleep(10000);
	}
	printf("%d \n",retValue);
	//printf("thread_transmit_spi fd = %d\n",fd);
	close(fd);
	pthread_exit(0);
}

int main(int argc, char **argv, char **envp)
{
pthread_t tid1,tid2,tid3;
int a,b,c;
b = pthread_create(&tid2,NULL,io_init,NULL);//thread initialization for gpio pins
c = pthread_create(&tid3,NULL,echo_init,NULL);//thread initialization for ultrasonic sensor
a = pthread_create(&tid1,NULL,thread_transmit_spi_user,NULL);//thread for spi data transfer
printf("inside thread  transmit user thread\n");
pthread_join(tid1,NULL);
pthread_join(tid3,NULL);
pthread_join(tid2,NULL);
return a;
return b;
return c;
}
