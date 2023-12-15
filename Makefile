
ifneq ($(KERNELRELEASE),)

# kbuild part of makefile
obj-m := lwnfs.o
lwnfs-y = lwnfs_main.o

else
# normal makefile
BR_DIR ?= /home/suxiaocheng/program/buildroot/vexpress/buildroot-2022.02.6
KDIR ?= $(BR_DIR)/output/build/linux-5.15.18
CROSS_COMPILE=$(BR_DIR)/output/host/bin/arm-buildroot-linux-uclibcgnueabihf-
CFLAGS := -I$(KDIR)/include
PWD := $(shell pwd)

all:
	make $(CFLAGS) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) -C  $(KDIR) M=$(PWD) modules
clean:
	rm -rf *.o *.mod.c *.ko *.symvers *.order *.makers

endif

