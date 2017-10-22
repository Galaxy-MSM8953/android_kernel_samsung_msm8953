#!/bin/bash

export PATH=/home/minz1/twrp/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/bin:$PATH 

mkdir out

make -C $(pwd) O=$(pwd)/out ARCH=arm64 CROSS_COMPILE=aarch64-linux-android- msm8953_sec_defconfig VARIANT_DEFCONFIG=msm8953_sec_j7poplte_usa_spr_defconfig SELINUX_DEFCONFIG=selinux_defconfig
make -C $(pwd) O=$(pwd)/out ARCH=arm64 CROSS_COMPILE=aarch64-linux-android-

cp out/arch/arm64/boot/Image $(pwd)/arch/arm64/boot/Image
