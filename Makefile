KDIR:= /opt/iot-devkit/1.7.2/sysroots/i586-poky-linux/usr/src/kernel
#PWD:= $(shell pwd)

CC = i586-poky-linux-gcc
ARCH = x86
CROSS_COMPILE = i586-poky-linux-
SROOT= /opt/iot-devkit/1.7.2/sysroots/i586-poky-linux/

APP = driver

obj-m := driver.o

all:
	make ARCH=x86 CROSS_COMPILE=i586-poky-linux- -C $(KDIR) M=$(PWD) modules
	echo my new PATH is ${PATH}
	$(CC) -o $(APP) --sysroot=$(SROOT) user.c -lpthread -Wall
#	$(CC) -o $(APP2) example2.c

	
clean:
	rm -f *.ko
	rm -f *.o
	rm -f Module.symvers
	rm -f modules.order
	rm -f *.mod.c
	rm -rf .tmp_versions
	rm -f *.mod.c
	rm -f *.mod.o
	rm -f \.*.cmd
	rm -f Module.markers
	rm -f $(APP) 
	rm -f *.log

cleanlog:
	rm -f *.log
