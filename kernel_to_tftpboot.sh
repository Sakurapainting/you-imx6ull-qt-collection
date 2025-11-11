#!/bin/bash

# 内核源码路径
KERNEL_DIR="/home/you/linux/IMX6ULL/linux/linux-imx-rel_imx_4.1.15_2.1.0_ga/arch/arm/boot"

# tftpboot 目录路径
TFTPBOOT_DIR="/home/you/linux/tftpboot"

# 复制 dtb
cp "$KERNEL_DIR/dts/imx6ull-alientek-emmc.dtb" "$TFTPBOOT_DIR/imx6ull-alientek-emmc.dtb"

# 复制 zImage
cp "$KERNEL_DIR/zImage" "$TFTPBOOT_DIR/zImage"

echo "dtb, zImage 已从 $KERNEL_DIR 复制到 $TFTPBOOT_DIR"