#!/usr/bin/perl

my $file = "log2";
my $repetitions = 5 ; 
my $current_count = 1 ; 
my $total_time = 0 ; 
my %config_to_runtime = () ;
my $current_config ;
open(HANDLE, $file) or die("Could not open file $file.");
foreach $line (<HANDLE>) {
        if($line =~ m/<(\d+),(\d+),(\d+)>/) {
		$current_config = "a9=$3,m3=$1,dsp=$2" ;
                #print "$current_config\n" ;
        } elsif($line =~ m/Time<total,a9-sync,a9,m3,dsp>\s=\s<(\d+\.\d+),(\d+\.\d+),(\d+\.\d+),(\d+\.\d+),(\d+\.\d+)>/) {
		if($current_count != 1) {
			$total_time += $1 ;
		}
                #print "total time = $1\n" ;
        }   
        if($current_count eq $repetitions) {
                $current_count = 1 ; 
                my $average_total_time = $total_time/($repetitions-1) ;
                #print "Average time = $average_total_time\n" ;
		$config_to_runtime{$current_config} = $average_total_time ;
                $total_time = 0 ; 
        } else {
                $current_count = $current_count + 1 ; 
        }   
}
close(HANDLE) ;

foreach (sort {$config_to_runtime{$b} <=> $config_to_runtime{$a}} keys %config_to_runtime) {
	  print "$_: $config_to_runtime{$_}\n";
}

