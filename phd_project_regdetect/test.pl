#!/usr/bin/perl
use Getopt::Long;
use strict;
use warnings;

my $opti ;
my $unopti ;
GetOptions ("opti"  => \$opti,   # flag
                "unopti"  => \$unopti)   # flag
or die("Error in command line arguments\n");
die "Specify atleast one option : (opti, unopti)" unless ($opti or $unopti) ;

my $smpbios_type ;
if($opti) {
	$smpbios_type =  "smpbios" ;
} 

if($unopti) {
	$smpbios_type = "smpbios_unoptimized" ;
}

#my $ip="192.168.106.107" ;
my $tms470_path = "export TMS470CGTOOLPATH=/opt/ti/ccsv5/tools/compiler/tms470" ; 
my $c6000_path = "export C6000CGTOOLPATH=/opt/ti/ccsv5/tools/compiler/c6000" ; 
my $bios_path = "export BIOSTOOLSROOT=/opt/ti" ; 
my $xdc_version = "export XDCVERSION=xdctools_3_24_02_30" ; 
my $bios_version = "export BIOSVERSION=bios_6_34_01_14" ; 
my $ipc_version = "export IPCVERSION=ipc_1_24_03_32" ;
my $env_vars = $tms470_path." ; ".$c6000_path." ; ".$bios_path." ; ".$xdc_version." ; ".$bios_version." ; ".$ipc_version ;
my $remote_script="/home/kiran/Downloads/$smpbios_type/sysbios-rpmsg_regdetect/setup.pl" ;
my $remote_dir="/home/kiran/Downloads/$smpbios_type/sysbios-rpmsg_regdetect/" ;
my $ip="129.215.90.125" ;
my $time_cmd = "/usr/bin/time -f \"%E real,%U user,%S sys\"" ;
my $disp="6:800x600" ;
my $home_dir = "/home/kiran" ;
my $firmware_dir_prefix=$home_dir."/exp" ;
my $attempt = 0 ;

sub run_a9_1
{
     my $block_size = 4 ;
     my $total_size = 64 ;
     my $j ;
     my $cmd_str ;
     for($j=16 ; $j>=0 ; $j--) {
	     my $size_a9 = $j * $block_size ;
	     my $print_size_a9 = 64 - $size_a9 ;
	     print("A9=$print_size_a9\n") ;
	     $cmd_str = "sudo $time_cmd ./app" ;
	     $cmd_str = $cmd_str." -l 1" ;
	     $cmd_str = $cmd_str." -x $size_a9" ;

	     #for($l=0 ; $l<5 ; $l++) {
	     print "cmd_str = $cmd_str\n" ;
	     system("$cmd_str") ;
	     sleep(5) ;
	     #}
     }
     #system("sudo $time_cmd ./simpletest -s $disp -r sysm3=$mat_size") ;
     #system("sudo $time_cmd ./simpletest -s $disp -r dsp=$mat_size") ;
     #system("sudo $time_cmd ./simpletest -s $disp -l 1") ;
}

sub run_a9
{
     my $block_size = 4 ;
     my $total_size = 64 ;
     my $j ;
     my $cmd_str ;
     for($j=16 ; $j>=0 ; $j--) {
	     my $size_a9 = $j * $block_size ;
	     my $print_size_a9 = 64 - $size_a9 ;
	     print("A9=$print_size_a9\n") ;
	     $cmd_str = "sudo $time_cmd ./app" ;
	     $cmd_str = $cmd_str." -l 2" ;
	     $cmd_str = $cmd_str." -x $size_a9" ;

	     #for($l=0 ; $l<5 ; $l++) {
	     print "cmd_str = $cmd_str\n" ;
	     system("$cmd_str") ;
	     sleep(5) ;
	     #}
     }
     #system("sudo $time_cmd ./simpletest -s $disp -r sysm3=$mat_size") ;
     #system("sudo $time_cmd ./simpletest -s $disp -r dsp=$mat_size") ;
     #system("sudo $time_cmd ./simpletest -s $disp -l 1") ;
}

sub run_dsp
{
     my $block_size = 4 ;
     my $total_size = 64 ;
     my $j ;
     my $cmd_str ;
     for($j=1 ; $j<=16 ; $j++) {
	     my $size_dsp = $j * $block_size ;
	     print("DSP=$size_dsp\n") ;
	     $cmd_str = "sudo $time_cmd ./app" ;
	     $cmd_str = $cmd_str." -r" ;
	     $cmd_str = $cmd_str." dsp=$size_dsp" ;

	     #for($l=0 ; $l<5 ; $l++) {
	     print "cmd_str = $cmd_str\n" ;
	     system("$cmd_str") ;
	     sleep(5) ;
	     #}
     }
     #system("sudo $time_cmd ./simpletest -s $disp -r sysm3=$mat_size") ;
     #system("sudo $time_cmd ./simpletest -s $disp -r dsp=$mat_size") ;
     #system("sudo $time_cmd ./simpletest -s $disp -l 1") ;
}

sub run_m3
{
     my $block_size = 4 ;
     my $total_size = 64 ;
     my $j ;
     my $cmd_str ;
     for($j=0 ; $j<=16 ; $j++) {
	     my $size_m3 = $j * $block_size ;
	     print("M3=$size_m3\n") ;
	     $cmd_str = "sudo $time_cmd ./app" ;
	     $cmd_str = $cmd_str." -r" ;
	     $cmd_str = $cmd_str." sysm3=$size_m3" ;

	     #for($l=0 ; $l<5 ; $l++) {
	     print "cmd_str = $cmd_str\n" ;
	     system("$cmd_str") ;
	     sleep(5) ;
	     #}
     }
     #system("sudo $time_cmd ./simpletest -s $disp -r sysm3=$mat_size") ;
     #system("sudo $time_cmd ./simpletest -s $disp -r dsp=$mat_size") ;
     #system("sudo $time_cmd ./simpletest -s $disp -l 1") ;
}

sub run_exp
{
	my $a9_threads = $_[0] ;
	#system("sudo $time_cmd ./simpletest -s $disp -r sysm3=$size_m3,dsp=$size_dsp -l $a9_threads") ;
	#system("sudo $time_cmd ./simpletest -s $disp -r sysm3=$mat_size") ;
	#system("sudo $time_cmd ./simpletest -s $disp -r dsp=$mat_size") ;
	#system("sudo $time_cmd ./simpletest -s $disp -l 1") ;
}

sub good_run
{
	my $good_run = 1 ;

	my $log_file = "log.txt" ;
	open LOGFILE, "$log_file" or die "cannot open file $log_file\n" ; 
	while(<LOGFILE>) {
		my $line = $_ ;
		if($line =~ m/FATAL:/) {
			$good_run = 0 ;
			print "Found : FATAL\n" ;
		} elsif($line =~ m/Can't open OMX device/) {
			$good_run = 0 ;
			print "Found : Can't open OMX device\n" ;
		} elsif($line =~ m/Can't connect to OMX/) {
			$good_run = 0 ;
			print "Found : Can't connect to OMX\n" ;
		} elsif($line =~ m/error/) {
			$good_run = 0 ;
			print "Found : Error\n" ;
		}   
	}   
	close(LOGFILE) ;

	return $good_run ;
}


my $m3_size = 0 ;
my $dsp_size = 0 ;
my $mat_size = 1024 ;
my $tgz_file="exp.tgz" ;
my $file="/home/kiran/Downloads/$smpbios_type/sysbios-rpmsg_regdetect/".$tgz_file ;
my $home="/home/kiran" ;
system("ssh $ip \"$env_vars ; $remote_script $m3_size $dsp_size $mat_size\"") ; 

my $firmware_dir = $firmware_dir_prefix ;
my $remote_file = $ip.":".$file ;
system("scp $remote_file $home_dir") ;
system("cd $home_dir ; tar xvzf $firmware_dir.tgz ; cd -") ;
system("sudo cp $firmware_dir/test_omx* /lib/firmware/ ; sudo $home_dir/scripts/reset_syslink.sh") ;
my $firmware_tgz_file = $firmware_dir.".tgz" ;
system("sudo rm -rf $firmware_dir ; sudo rm -f $firmware_tgz_file") ;
#sleep(10) ; #KC : reset to 30

#$attempt = $attempt + 1 ;
#print "Attempt #$attempt\n" ;
#run_exp($a9_threads) ;
#run_a9_1() ;
#run_a9() ;
#run_dsp() ;
#run_m3() ;
#$attempt = $attempt + 1 ;
#print "Attempt #$attempt\n" ;
#run_exp($a9_threads) ;
#$attempt = $attempt + 1 ;
#print "Attempt #$attempt\n" ;
#run_exp($a9_threads) ;
#$attempt = $attempt + 1 ;
#print "Attempt #$attempt\n" ;
#run_exp($a9_threads) ;
#$attempt = $attempt + 1 ;
#print "Attempt #$attempt\n" ;
#run_exp($a9_threads) ;

#if(not good_run()) {
#	exit 0 ;
#}
