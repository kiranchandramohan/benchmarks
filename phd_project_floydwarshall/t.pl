#!/usr/bin/perl

my @results ;
my $indx = 0 ;

sub find_avg_time
{
	my $size_m3 = $_[0] ;
	my $size_dsp = $_[1] ;
	$results[$indx] = "Average time : m3=$size_m3,dsp=$size_dsp : $avg_time" ;
	$indx = $indx+1 ;
}

#find_a9_avg_time(2, 10) ;
for($indx1=0; $indx1<=4; $indx1++) {
	for($indx2=0; $indx2<=4; $indx2++) {
		my $block_size = 16 ;
		my $m3_size = $indx1 * $block_size ;
		my $dsp_size = $indx2 * $block_size ;
		find_avg_time($m3_size, $dsp_size) ;
	}
}

foreach $item (@results) {
	print "$item\n" ;
}
