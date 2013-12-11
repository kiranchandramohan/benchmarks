#!/bin/bash
echo "Removing Modules"
modprobe -r omapdce               
modprobe -r omaprpc               
modprobe -r rpmsg_resmgr_common   
modprobe -r omap_rpmsg_resmgr     
modprobe -r rpmsg_resmgr        
modprobe -r rpmsg_omx
modprobe -r omap_remoteproc       
modprobe -r remoteproc         
modprobe -r virtio_rpmsg_bus      
modprobe -r virtio_ring            
modprobe -r virtio                 
