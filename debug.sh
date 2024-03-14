#!/bin/bash 

RUNENV_PREFIX="/home/lishuang/qemu-loongarch/qemu-loongarch-runenv"
KERNEL_PREFIX="/home/lishuang/xv6-loongarch/xv6-loongarch-exp"

cd $RUNENV_PREFIX

${RUNENV_PREFIX}/qemu/x86_64/qemu-system-loongarch64 \
	-m 4G \
	-smp 1 \
	-bios ${RUNENV_PREFIX}/loongarch_bios_0310_debug.bin \
	-kernel ${KERNEL_PREFIX}/kernel/kernel \
	-initrd ${RUNENV_PREFIX}/busybox-rootfs.img \
	-append 'root=/dev/ram console=ttyS0,115200 rdinit=/init' \
	-vga none \
	-nographic \
	-s -S 

# echo $RUN_CMD
# $RUN_CMD