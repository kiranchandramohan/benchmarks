#!/usr/bin/perl

my $m3_cmd = "-r sysm3=2048" ;
my $dsp_cmd = "-r dsp=2048" ;
my $a9_1t = "-l 1" ;
my $a9_2t = "-l 2" ;

sub find_avg_time
{
	my $num_runs = $_[0] ;
	my $cmd = $_[1] ;
	my $block_size = 32 ;
	for($j=5 ; $j<=6 ; $j++) {
		my $m3_size = $j*$block_size ;
		for($k=8 ; $k<=9 ; $k++) {
			my $dsp_size = $k*$block_size ;
			print "sudo ./app -l 2 -r dsp=$dsp_size,sysm3=$m3_size\n" ;
			my $total_time = 0 ;
			my $cur_num_runs = $num_runs ;
			for($i=0 ; $i<$num_runs ; $i++) {
				next if($j==5 and $k==8) ;
				#my @time = `sudo ./app -l $num_threads` ;
				#my @time = `sudo ./app -l $num_threads -r sysm3=8,dsp=6` ;
				#my @time = `sudo ./app $cmd` ;
				my @time = `sudo ./app -l 2 -r dsp=$dsp_size,sysm3=$m3_size` ;
				my $length = @time ;
				my $time_str= $time[$length-1] ;
				chomp($time_str) ;
				my $cur_time = 0 ;
				if($time_str =~ m/^Time[\s\S]+\d+\.\d+\,(\d+\.\d+)\,\d+\.\d+\,\d+\.\d+\,\d+\.\d+\,\d+\.\d+\>$/) {
					$cur_time = $1 ;
					print "cur_time = $cur_time\n" ;
				}
				if($cur_time > 6.5) {
					$cur_num_runs = $cur_num_runs - 1 ;
				} else {
					$total_time += $cur_time ;
				}
				sleep(3) ;
			}
			my $avg_time = $total_time/$cur_num_runs ;
			print "Average time with #threads=$num_threads : $avg_time\n" ;
		}
	}
}

#find_avg_time(1, 10) ;
#print "M3 command\n" ;
#find_avg_time(5, $m3_cmd) ;
#print "DSP command\n" ;
#find_avg_time(5, $dsp_cmd) ;
#print "A9 1T command\n" ;
#find_avg_time(5, $a9_1t) ;
#print "A9 2T command\n" ;
#find_avg_time(5, $a9_2t) ;
print "Mixed command\n" ;
find_avg_time(10, $a9_2t) ;
