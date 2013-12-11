#!/usr/bin/perl

sub find_a9_avg_time
{
	my $num_threads = $_[0] ;
	my $num_runs = $_[1] ;
	my $total_time = 0 ;
	my $cur_runs = $num_runs ;
	for($i=0 ; $i<$num_runs ; $i++) {
		my @time = `sudo ./app -l $num_threads` ;
		my $length = @time ;
		my $time_str= $time[$length-1] ;
		chomp($time_str) ;
		my $cur_time = 0 ;
		if($time_str =~ m/^Time[\s\S]+\d+\.\d+\,(\d+\.\d+)\,\d+\.\d+\,\d+\.\d+\,\d+\.\d+\,\d+\.\d+\>$/) {
			$cur_time = $1 ;
		}
		if($cur_time > 14.5) {
			$cur_runs = $cur_runs - 1 ;
		        print "cur_runs = $cur_runs\n" ;
		} else {
		       $total_time += $cur_time ;
		       print "cur_time = $cur_time\n" ;
		}
		sleep(3) ;
	}
	my $avg_time = $total_time/$cur_runs ;
	print "Average time with #threads=$num_threads : $avg_time\n" ;
}

sub find_avg_time
{
	#my $num_threads = $_[0] ;
	#my $num_runs = $_[1] ;
	my $size_m3 = $_[0] ;
	my $size_dsp = $_[1] ;
	my $num_runs = $_[2] ;
	my $total_time = 0 ;
	my $cur_runs = $num_runs ;
	for($i=0 ; $i<$num_runs ; $i++) {
		#my @time = `sudo ./app -l $num_threads` ;
		#my @time = `sudo ./app -r sysm3=4096` ;
		#my @time = `sudo ./app -r dsp=4096` ;
		my @time = `sudo ./app -l 2 -r dsp=$size_dsp,sysm3=$size_m3` ;
		print "sudo ./app -l 2 -r sysm3=$size_m3,dsp=$size_dsp\n" ;
		my $length = @time ;
		my $time_str= $time[$length-1] ;
		chomp($time_str) ;
		my $cur_time = 0 ;
		if($time_str =~ m/^Time[\s\S]+\d+\.\d+\,(\d+\.\d+)\,\d+\.\d+\,\d+\.\d+\,\d+\.\d+\,\d+\.\d+\>$/) {
			$cur_time = $1 ;
		}
		if($cur_time > 14.5) {
			$cur_runs = $cur_runs - 1 ;
		        print "cur_runs = $cur_runs\n" ;
		} else {
		       $total_time += $cur_time ;
		       print "cur_time = $cur_time\n" ;
		}
		sleep(3) ;
	}
	my $avg_time = $total_time/$cur_runs ;
	print "Average time with #threads=$num_threads : $avg_time\n" ;
}

#find_a9_avg_time(2, 10) ;
#find_avg_time(0, 0, 10) ;
#find_avg_time(4096, 0, 10) ;
#find_avg_time(0, 4096, 10) ;
#find_avg_time(416, 416, 10) ;
#find_avg_time(416, 448, 10) ;
#find_avg_time(416, 480, 10) ;
#find_avg_time(448, 416, 10) ;
#find_avg_time(448, 448, 10) ;
#find_avg_time(448, 480, 10) ;
#find_avg_time(480, 416, 10) ;
#find_avg_time(480, 448, 10) ;
#find_avg_time(480, 480, 10) ;

for(my $m3_indx=384 ; $m3_indx<480 ; $m3_indx+=32) {
	for(my $dsp_indx=384 ; $dsp_indx<=512 ; $dsp_indx+=32) {
		find_avg_time($m3_indx, $dsp_indx, 1) ;
	}
}
