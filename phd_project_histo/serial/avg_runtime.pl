#!/usr/bin/perl

my $parallel = 0 ;
my $serial = 0 ;
my $num_runs = 10 ;
for($i=0 ; $i<$num_runs ; $i++) {
	$parallel += `./par` ;
	sleep(3) ;
}
my $avg_parallel = $parallel/$num_runs ;
print "Parallel time = $avg_parallel\n" ;

for($i=0 ; $i<$num_runs ; $i++) {
	$serial += `./ser` ;
	sleep(3) ;
}
my $avg_serial = $serial/$num_runs ;
print "Serial time = $avg_serial\n" ;
