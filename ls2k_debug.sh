#!/bin/bash 

RUNENV_PREFIX="/home/lishuang/qemu-loongarch/qemu-ls2k"
KERNEL_PREFIX="/home/lishuang/xv6-loongarch/xv6-loongarch-exp"

cd $RUNENV_PREFIX

./bin/qemu-system-loongarch64 \
	-M ls2k \
	-serial stdio \
	-k ./share/qemu/keymaps/en-us \
	-kernel ~/xv6-loongarch/xv6-loongarch-exp/kernel/kernel \
	-initrd ./tmp/2kfs.img \
	-serial vc \
	-m 1G \
	-device usb-kbd,bus=usb-bus.0 \
	-device usb-tablet,bus=usb-bus.0 \
	-device usb-storage,drive=udisk \
	-drive if=none,id=udisk,file=./tmp/disk \
	-net nic \
	-net user,net=127.0.0.1/8,tftp=/srv/tftp \
	-vnc :0 \
	-s -S
	# -D /dev/null \
	# -vnc :0 -D ./tmp/qemu.log


# echo $RUN_CMD
# $RUN_CMD