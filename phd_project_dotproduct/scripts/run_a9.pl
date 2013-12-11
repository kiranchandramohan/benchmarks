#!/usr/bin/perl

$ip="192.168.106.107" ;
my $time_cmd = "/usr/bin/time -f \"%E real,%U user,%S sys\"" ;
my $disp="6:800x600" ;
my $home_dir = "/home/kiran" ;
my $firmware_dir_prefix=$home_dir."/exp" ;
my $mat_size = 1024 ;

sub run_exp
{
	my $size_m3 = $_[0] ;
	my $size_dsp = $_[1] ;
	my $size_a9 = $_[2] ;
	
	#print "m3 size = $size_m3, dsp size = $size_dsp, a9 size = $size_a9\n" ;
	#system("sudo $time_cmd ./simpletest -s $disp -r sysm3=$size_m3,dsp=$size_dsp -l 2") ;
	print "a9 size = $size_a9\n" ;
	system("sudo $time_cmd ./simpletest -s $disp -l 2") ;
}

my $step_size = 32 ;
my $a9_size = 32 ;

for ($a9_size = 32 ; $a9_size <= 1024 ; $a9_size+=$step_size) {
		
	run_exp(0,0,$a9_size) ;
}
