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
my $ip="129.215.90.125" ;
my $time_cmd = "/usr/bin/time -f \"%E real,%U user,%S sys\"" ;
my $disp="6:800x600" ;
my $home_dir = "/home/kiran" ;
my $firmware_dir_prefix=$home_dir."/exp" ;
my $mat_size = 1024 ;

my $mpy_fn = <<END1 ;
MULTIPLYFN
{
	int i,j,k;
	FORLOOP
	{
		for (j = 0; j < SIZE; j++)
		{
			for ( k = 0; k < SIZE; k++)
				c[i*SIZE+j] += a[i*SIZE+k]*b[k*SIZE+j];
		}
	}
}
END1

sub get_modified_fn
{
        my $start_indx = $_[0] ;
        my $end_indx = $_[1] ;
	my $thread = $_[2] ;

        my @mpy_fn_lines = split /\n/, $mpy_fn ;

        my @new_mpy_fn_lines ;
        foreach my $line (@mpy_fn_lines) {
		if($line =~ m/MULTIPLYFN/) {
                        my $nline = "void multiply$thread(int* __restrict a, int* __restrict b, int* __restrict c)" ;
                        push @new_mpy_fn_lines, $nline ;
		} elsif($line =~ m/FORLOOP/) {
                        my $nline = "\tfor (i = $start_indx ; i < $end_indx ; i++)" ;
                        push @new_mpy_fn_lines, $nline ;
                } else {
                        push @new_mpy_fn_lines, $line ;
                }
        }

        my $new_mpy_fn_str = join("\n",@new_mpy_fn_lines) ;

        return $new_mpy_fn_str ;
}

sub create_file
{
        my $file_name = $_[0] ;
        my $template_file_name = $_[1] ;
	my $start_indx = $_[2] ;
	my $end_indx = $_[3] ;
	my $num_threads = $_[4] ;

	my $block_size = ($_[3] - $_[2])/2 ;

        open MYFILE, ">$file_name" or die "cannot open file $file_name\n" ; 
        open FILE, "$template_file_name" or die "cannot open file $template_file_name\n" ; 
        while(<FILE>) {
                my $line = $_ ;
		if($line =~ m/#INSERT_MPY1/) {
			my $mpy1_start = $start_indx  ;
			my $mpy1_end = $start_indx + $block_size ;
			my $modified_fn1 = get_modified_fn($mpy1_start, $mpy1_end, 1) ;
			print MYFILE $modified_fn1 ;
                } elsif($line =~ m/#INSERT_MPY2/) {
			my $mpy2_start = $start_indx + $block_size ;
			my $mpy2_end = $end_indx ;
			my $modified_fn2 = get_modified_fn($mpy2_start, $mpy2_end, 2) ;
			print MYFILE $modified_fn2 ;
                } else {
                        print MYFILE $line ;
                }   
        }   
        close(FILE) ;
        close(MYFILE) ;
}


sub run_exp
{
	my $size_m3 = $_[0] ;
	my $size_dsp = $_[1] ;
	my $size_a9 = $_[2] ;
	my $a9_threads = $_[3] ;
	
	#print "m3 size = $size_m3, dsp size = $size_dsp, a9 size = $size_a9\n" ;
	#system("sudo $time_cmd ./simpletest -s $disp -r sysm3=$size_m3,dsp=$size_dsp -l $a9_threads") ;
	#system("sudo $time_cmd ./simpletest -r sysm3=$size_m3,dsp=$size_dsp -l $a9_threads") ;
	#print "m3 size = $size_m3\n" ;
	#system("sudo $time_cmd ./simpletest -s $disp -r sysm3=$size_m3") ;
	print "dsp size = $size_dsp\n" ;
	system("sudo $time_cmd ./simpletest -s $disp -r dsp=$size_dsp") ;
	#print "a9 size = $size_a9\n" ;
	#system("sudo $time_cmd ./simpletest -s $disp -l $a9_threads -x $size_m3") ;
	#print "m3 size = $size_m3, dsp size = $size_dsp\n" ;
	#system("sudo $time_cmd ./simpletest -s $disp -r sysm3=$size_m3,dsp=$size_dsp") ;
}

my $step_size = 32 ;
my $m3_size = 32 ;
my $dsp_size = $m3_size ;
my $a9_threads = 2 ;
my $log_file = "log" ;

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


#for ($m3_size = 992 ; $m3_size >= 512 ; $m3_size-=$step_size) {
for ($m3_size = 0 ; $m3_size <= 0 ; $m3_size+=$step_size) {
	for ($dsp_size = 1024 ; $dsp_size <= 1024 ; $dsp_size+=$step_size) {
		if(($m3_size + $dsp_size) > 1024) {
			next ;
		}
		my $a9_size = $mat_size-($m3_size+$dsp_size) ;
		print "<$m3_size,$dsp_size,$a9_size>\n" ;

		my $tgz_file="exp_".$m3_size."_".$dsp_size."_".$mat_size.".tgz" ;
		my $file="/home/kiran/Downloads/smpbios/sysbios-rpmsg/".$tgz_file ;
		my $home="/home/kiran" ;
		#system("ssh $ip \"$env_vars ; $remote_script $m3_size $dsp_size $mat_size\"") ; 
		
		my $firmware_dir = $firmware_dir_prefix."_"."$m3_size"."_"."$dsp_size"."_"."$mat_size" ;
		while(not (-e $firmware_dir)) {
			print "File $firmware_dir doesn't exist : Initiating copying\n" ;
			system("scp $ip:$file $home") ;
			system("cd $home ; tar -xmvzf $tgz_file ; cd -") ;
			sleep(0) ; #KC : reset to 60
		}
		system("sudo cp $firmware_dir/test_omx* /lib/firmware/ ; sudo $home_dir/reset_syslink.sh") ;
		my $firmware_tgz_file = $firmware_dir.".tgz" ;
		system("sudo rm -rf $firmware_dir ; sudo rm -f $firmware_tgz_file") ;
		sleep(30) ; #KC : reset to 30

		system("rm -f simpletest.c") ;
		create_file("simpletest.c","template_simpletest.c",$m3_size+$dsp_size,$mat_size,$a9_threads) ;
		system("make clean ; make") ;
		
		run_exp($m3_size,$dsp_size,$a9_size,$a9_threads) ;

		if(not good_run()) {
			exit 0 ;
		}
	}
}
