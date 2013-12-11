#!/usr/bin/perl

#my $ip="192.168.106.107" ;
my $tms470_path = "export TMS470CGTOOLPATH=/opt/ti/ccsv5/tools/compiler/tms470" ; 
my $c6000_path = "export C6000CGTOOLPATH=/opt/ti/ccsv5/tools/compiler/c6000" ; 
my $bios_path = "export BIOSTOOLSROOT=/opt/ti" ; 
my $xdc_version = "export XDCVERSION=xdctools_3_24_02_30" ; 
my $bios_version = "export BIOSVERSION=bios_6_34_01_14" ; 
my $ipc_version = "export IPCVERSION=ipc_1_24_03_32" ;
my $env_vars = $tms470_path." ; ".$c6000_path." ; ".$bios_path." ; ".$xdc_version." ; ".$bios_version." ; ".$ipc_version ;
my $remote_script="/home/kiran/Downloads/smpbios/sysbios-rpmsg/setup.pl" ;
my $remote_dir="/home/kiran/Downloads/smpbios/sysbios-rpmsg/" ;
my $ip="129.215.90.125" ;
my $time_cmd = "/usr/bin/time -f \"%E real,%U user,%S sys\"" ;
my $disp="6:800x600" ;
my $home_dir = "/home/kiran" ;
my $firmware_dir_prefix=$home_dir."/exp" ;
my $mat_size = 1000 ;

sub run_exp
{
	my $size_m3 = $_[0] ;
	my $size_dsp = $_[1] ;
	my $size_a9 = $_[2] ;
	my $a9_threads = $_[3] ;

	if(($size_m3 > 0) and ($size_dsp > 0) and ($size_a9 > 0)) {
		print "m3 size = $size_m3, dsp size = $size_dsp, a9 size = $size_a9\n" ;
		system("sudo $time_cmd ./simpletest -s $disp -r sysm3=$size_m3,dsp=$size_dsp -l $a9_threads") ;
	} elsif(($size_m3 > 0) and ($size_dsp > 0)) {
		print "m3 size = $size_m3, dsp size = $size_dsp\n" ;
		system("sudo $time_cmd ./simpletest -s $disp -r sysm3=$size_m3,dsp=$size_dsp") ;
	} elsif(($size_m3 > 0) and ($size_a9 > 0)) {
		print "m3 size = $size_m3, a9 size = $size_a9\n" ;
		system("sudo $time_cmd ./simpletest -s $disp -r sysm3=$size_m3 -l $a9_threads") ;
	} elsif(($size_dsp > 0) and ($size_a9 > 0)) {
		print "dsp size = $size_dsp, a9 size = $size_a9\n" ;
		system("sudo $time_cmd ./simpletest -s $disp -r dsp=$size_dsp -l $a9_threads") ;
	} elsif($size_m3 > 0) {
		print "m3 size = $size_m3\n" ;
		system("sudo $time_cmd ./simpletest -s $disp -r sysm3=$size_m3") ;
	} elsif($size_dsp > 0) {
		print "dsp size = $size_dsp\n" ;
		system("sudo $time_cmd ./simpletest -s $disp -r dsp=$size_dsp") ;
	} elsif($size_a9 > 0) {
		print "a9 size = $size_a9\n" ;
		system("sudo $time_cmd ./simpletest -s $disp -l $a9_threads") ;
	}
	
	#print "a9 size = $size_a9\n" ;
	#system("sudo $time_cmd ./simpletest -s $disp -l $a9_threads -x $size_m3") ;
}

my $step_size = 32 ;
my $m3_size = 128 ;
my $dsp_size = $m3_size ;
my $a9_threads = 2 ;
my $log_file = "log" ;
my $attempt = 0 ;

sub good_run
{
	my $good_run = 1 ;

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


for ($m3_size = 0 ; $m3_size <= 256 ; $m3_size+=$step_size) {
	for ($dsp_size = 0 ; $dsp_size <= 512 ; $dsp_size+=$step_size) {
		if(($m3_size + $dsp_size) > 2048) {
			next ;
		}
		my $a9_size = $mat_size-($m3_size+$dsp_size) ;
		print "<$m3_size,$dsp_size,$a9_size>\n" ;

		#my $tgz_file="exp_".$m3_size."_".$dsp_size."_".$mat_size.".tgz" ;
		#my $file="/home/kiran/Downloads/smpbios/sysbios-rpmsg/".$tgz_file ;
		#my $home="/home/kiran" ;
		#system("ssh $ip \"$env_vars ; $remote_script $m3_size $dsp_size $mat_size\"") ; 
		#
		#my $firmware_dir = $firmware_dir_prefix."_"."$m3_size"."_"."$dsp_size"."_"."$mat_size" ;
		#my $remote_file = $ip.":".$file ;
		#system("scp $remote_file $home_dir") ;
		#system("cd $home_dir ; tar xvzf $firmware_dir.tgz ; cd -") ;
		#system("sudo cp $firmware_dir/test_omx* /lib/firmware/ ; sudo $home_dir/reset_syslink.sh") ;
		#my $firmware_tgz_file = $firmware_dir.".tgz" ;
		#system("sudo rm -rf $firmware_dir ; sudo rm -f $firmware_tgz_file") ;
		sleep(10) ; #KC : reset to 30

		$attempt = $attempt + 1 ;
		print "Attempt #$attempt\n" ;
		run_exp($m3_size,$dsp_size,$a9_size,$a9_threads) ;
		$attempt = $attempt + 1 ;
		print "Attempt #$attempt\n" ;
		run_exp($m3_size,$dsp_size,$a9_size,$a9_threads) ;
		$attempt = $attempt + 1 ;
		print "Attempt #$attempt\n" ;
		run_exp($m3_size,$dsp_size,$a9_size,$a9_threads) ;
		$attempt = $attempt + 1 ;
		print "Attempt #$attempt\n" ;
		run_exp($m3_size,$dsp_size,$a9_size,$a9_threads) ;
		$attempt = $attempt + 1 ;
		print "Attempt #$attempt\n" ;
		run_exp($m3_size,$dsp_size,$a9_size,$a9_threads) ;
		$attempt = 0 ;

		if(not good_run()) {
			exit 0 ;
		}
	}
}
