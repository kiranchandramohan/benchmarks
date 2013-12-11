#!/usr/bin/perl

my $time_cmd = "/usr/bin/time -f \"%E real,%U user,%S sys\"" ;
my $disp="6:800x600" ;
my @sizes = (320,576,640,704) ;
my $home_dir = "/home/kiran" ;
my $firmware_dir_prefix=$home_dir."/exp" ;
my $matdim=2048;

sub run_exp
{
	my $size = $_[0] ;
	
	`echo "a9 size = $size\n"` ;
	`sudo $time_cmd ./simpletest -s $disp -l 2` ;
	#`echo "m3 size = $size\n" &>> sysm3` ;
	#`sudo $time_cmd ./simpletest -s $disp -r sysm3=$size &>> sysm3` ;
	#`echo "dsp size = $size\n" &>> dsp` ;
	#`sudo $time_cmd ./simpletest -s $disp -r dsp=$size &>> dsp` ;
	#`sudo $time_cmd ./simpletest -s $disp -l 1 >> a9-1` ;
	#`sudo $time_cmd ./simpletest -s $disp -l 2 >> a9-2` ;
}

sub create_exp
{
	my $size = $_[0] ;
	my $file = "simpletest.c" ;
	open FILE, $file or die "cannot open file $file\n" ;
	open (MYFILE, '>tmp.c') ;

	while(<FILE>) {
		my $line = $_ ;
		chomp($line) ;
		if($line =~ m/int remaining_size =/) {
			print MYFILE "int remaining_size = $size ;\n" ;
		} else {
			print MYFILE "$line\n" ;
		}
	}
	close (MYFILE) ; 
	close (FILE) ; 
}

for($size = 32 ; $size <=512 ; $size+=32)
{
	#my $firmware_dir = $firmware_dir_prefix."_"."$size"."_"."0"."_"."$matdim" ;
	#system("sudo cp $firmware_dir/test_omx* /lib/firmware/ ; sudo $home_dir/reset_syslink.sh") ;
	#sleep(1) ;

	create_exp $size ;
	system("cp simpletest.c simpletest.c_bak_$size ; cp tmp.c simpletest.c") ;
	system("make") ;
	sleep(1) ;

	run_exp($size) ;
	sleep(1) ;
}
