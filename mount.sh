#!/bin/sh

lsmod |grep lwnfs

if [ $? -eq 1 ]; then
	insmod lwnfs.ko
fi

if [ ! -d lwnfs ]; then
	mkdir lwnfs
fi
mount -t lwnfs none ./lwnfs
