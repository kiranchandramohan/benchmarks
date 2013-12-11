#!/bin/bash
echo "Removing Modules"
sudo modprobe -r rpmsg_omx
sudo modprobe -r omap_remoteproc
sudo modprobe -r virtio_rpmsg_bus
cd $HOME/my_modules/ioctl/hw_spinlock/ && sudo rmmod hw_spinlock_ioctl && cd $HOME
sudo modprobe -r remoteproc

echo "Installing Modules"
sudo modprobe remoteproc
cd $HOME/my_modules/ioctl/hw_spinlock/ && sudo insmod hw_spinlock_ioctl.ko && cd $HOME
sudo modprobe virtio_rpmsg_bus
sudo modprobe omap_remoteproc
sudo modprobe rpmsg_omx
