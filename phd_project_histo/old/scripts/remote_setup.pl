#!/usr/bin/perl

$ip="192.168.106.107" ;
#my @m3sizes = (50,100,200,300,400,500,600,700,800,900,1000,64,128,256,512,768,1024) ;
#my @m3sizes = (192,284,448) ;
my @m3sizes = (320,576,640,704) ;
foreach my $tmp_size (@m3sizes)
{
	my $m3_size=$tmp_size ;
	my $dsp_size=0 ;
	my $size=2048 ;
	my $tgz_file="exp_".$m3_size."_".$dsp_size."_".$size.".tgz" ;
	my $file="/home/kiran/Downloads/sysbios-rpmsg/".$tgz_file ;
	my $home="/home/kiran" ;
	system("ssh $ip \"export TMS470CGTOOLPATH=/opt/ti/ccsv5/tools/compiler/tms470 ; export C6000CGTOOLPATH=/opt/ti/ccsv5/tools/compiler/c6000 ; export BIOSTOOLSROOT=/opt/ti ; export XDCVERSION=xdctools_3_22_04_46 ; export BIOSVERSION=bios_6_32_05_54 ; export IPCVERSION=ipc_1_23_05_40 ; /home/kiran/Downloads/sysbios-rpmsg/setup.pl $m3_size $dsp_size $size\"") ; 
	system("scp $ip:$file $home") ;
	system("cd $home ; tar -xmvzf $tgz_file ; cd -") ;
}
