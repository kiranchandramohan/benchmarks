#!/bin/bash
sudo service lightdm stop
sudo service avahi-daemon stop
sudo service network-manager stop
sudo service whoopsie stop
sudo $HOME/scripts/remove_dce_syslink.sh
sudo $HOME/scripts/reset_syslink.sh
cd $HOME/my_modules/ioctl && sudo insmod ioctl.ko && cd $HOME
sudo mknod /dev/cdev_example c 247 0
sudo mknod /dev/hw_spinlock c 249 0
#sudo mknod /dev/hw_spinlock c 248 0

