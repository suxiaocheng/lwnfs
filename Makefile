
ifneq ($(KERNELRELEASE),)

# kbuild part of makefile
obj-m := lwnfs.o
lwnfs-y = lwnfs_main.o

else
# normal makefile
KDIR ?= /work/program/vexpress/buildroot-2022.02.6/output/build/linux-5.15.18
CFLAGS := -I$(KDIR)/include
PWD := $(shell pwd)

all:
	make $(CFLAGS) ARCH=arm CROSS_COMPILE=arm-buildroot-linux-uclibcgnueabihf- -C  $(KDIR) M=$(PWD) modules
clean:
	rm -rf *.o *.mod.c *.ko *.symvers *.order *.makers

endif

