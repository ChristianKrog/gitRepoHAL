ifeq ($(KERNELRELEASE),)
	
	
KERNELDIR = ~/sources/rpi-4.14/

PWD := $(shell pwd)

default:
	$(MAKE) ARCH=arm CROSS_COMPILE=arm-poky-linux-gnueabi- -C $(KERNELDIR) M=$(PWD) modules

else

	obj-m := km_hello.o
	ccflags-y := -std=gnu99 -Wno-declaration-after-statement -Werror
endif