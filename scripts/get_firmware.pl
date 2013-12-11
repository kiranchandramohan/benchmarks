#!/usr/bin/perl


my $tms470_path = "export TMS470CGTOOLPATH=/opt/ti/ccsv5/tools/compiler/tms470" ; 
my $c6000_path = "export C6000CGTOOLPATH=/opt/ti/ccsv5/tools/compiler/c6000" ; 
my $bios_path = "export BIOSTOOLSROOT=/opt/ti" ; 
my $xdc_version = "export XDCVERSION=xdctools_3_24_02_30" ; 
my $bios_version = "export BIOSVERSION=bios_6_34_01_14" ; 
my $ipc_version = "export IPCVERSION=ipc_1_24_03_32" ;
my $env_vars = $tms470_path." ; ".$c6000_path." ; ".$bios_path." ; ".$xdc_version." ; ".$bios_version." ; ".$ipc_version ;
my $remote_ip="129.215.90.125" ;
my $home_dir="/home/kiran" ;
my $tgz_file="exp.tgz" ;
my $remote_file="/home/kiran/Downloads/smpbios/sysbios-rpmsg/$tgz_file" ;
my $remote_script="/home/kiran/Downloads/smpbios/sysbios-rpmsg/setup.pl" ;

system("cd $home_dir ; rm -f exp.tgz ; rm -rf exp") ;
system("ssh $remote_ip \"$env_vars ; $remote_script\"") ; 
system("scp $remote_ip:$remote_file $home_dir") ;
system("cd $home_dir ; tar xmvzf $tgz_file ; sudo cp exp/test_omx_dsp.xe64T exp/test_omx_ipu.xem3 /lib/firmware/") ;
system("cd $home_dir ; sudo ./reset_syslink.sh") ;
