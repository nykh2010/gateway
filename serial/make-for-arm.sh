#!/bin/bash
cp -r Debug ARM_Debug
sed -i 's/gcc/${CC}/g' ./ARM_Debug/makefile ./ARM_Debug/*.mk ./ARM_Debug/*/*.mk ./ARM_Debug/*/*/*.mk
# sed -i 's/ttyS0/ttymxc1/g' ./ARM_Debug/config/serial.ini
sed -i 's/ttyS0/ttyUSB0/g' ./ARM_Debug/config/serial.ini
cd ./ARM_Debug
# json-c sqlite3
# /home/user/MYD-Y6ULX-devel/fsl-release-yocto/myd-y6ull/tmp/sysroots/myd-y6ull14x14/usr/include
# /home/user/MYD-Y6ULX-devel/fsl-release-yocto/myd-y6ull/tmp/sysroots/myd-y6ull14x14/usr/lib
export PATH=/home/user/MYD-Y6ULX-devel/fsl-release-yocto/myd-y6ull/tmp/sysroots/myd-y6ull14x14/usr/include:$PATH
# /home/user/MYD-Y6ULX-devel/fsl-release-yocto/myd-y6ull/tmp/sysroots/myd-y6ull14x14/usr/lib
