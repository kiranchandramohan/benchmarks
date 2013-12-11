#!/usr/bin/perl

my @results ;
my $gindx = 0 ;

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
		if($cur_time > 16.5) {
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
	my $total_a9_time = 0 ;
	my $total_m3_time = 0 ;
	my $total_dsp_time = 0 ;
	my $cur_runs = $num_runs ;
	for($i=0 ; $i<$num_runs ; $i++) {
		#my @time = `sudo ./app -l $num_threads` ;
		#my @time = `sudo ./app -r sysm3=512` ;
		#my @time = `sudo ./app -r dsp=512` ;
		#my @time = `sudo ./app -r dsp=4096` ;
		my $time_threshold ;
		my @time ;
		if($size_m3 > 0 and $size_dsp > 0) {
			print "sudo ./app -l 2 -r sysm3=$size_m3,dsp=$size_dsp\n" ;
			@time = `sudo ./app -l 2 -r dsp=$size_dsp,sysm3=$size_m3` ;
			$time_threshold = 48.0 ;
		} elsif($size_m3 > 0) {
			if($size_m3 == 1024) {
				print "sudo ./app -r sysm3=$size_m3\n" ;
				@time = `sudo ./app -r sysm3=$size_m3` ;
			} else {
				print "sudo ./app -l 2 -r sysm3=$size_m3\n" ;
				@time = `sudo ./app -l 2 -r sysm3=$size_m3` ;
			}
			$time_threshold = 215.0 ;
		} elsif($size_dsp > 0) {
			if($size_dsp == 1024) {
				print "sudo ./app -r dsp=$size_dsp\n" ;
				@time = `sudo ./app -r dsp=$size_dsp` ;
			} else {
				print "sudo ./app -l 2 -r dsp=$size_dsp\n" ;
				@time = `sudo ./app -l 2 -r dsp=$size_dsp` ;
			}
			$time_threshold = 100.0 ;
		} else {
			print "sudo ./app -l 2\n" ;
			@time = `sudo ./app -l 2` ;
			$time_threshold = 228.5 ;
		}
		#my @time = `sudo ./app -l 2` ;

		#if($size_m3 > 0) {
		#	print "sudo ./app -r sysm3=$size_m3\n" ;
		#	@time = `sudo ./app -r sysm3=$size_m3` ;
		#} elsif($size_dsp > 0) {
		#	print "sudo ./app -r dsp=$size_dsp\n" ;
		#	@time = `sudo ./app -r dsp=$size_dsp` ;
		#}

		my $length = @time ;
		my $time_str= $time[$length-1] ;
		chomp($time_str) ;
		my $cur_time = 0 ;
		if($time_str =~ m/^Time[\s\S]+\d+\.\d+\,(\d+\.\d+)\,(\d+\.\d+)\,(\d+\.\d+)\,(\d+\.\d+)\,(\d+\.\d+)\>$/) {
			$total_time = $1 ;
			$total_a9_time = $2 ;
			$total_m3_time = $4 ;
			$total_dsp_time = $5 ;
		}
		#if($cur_time > $time_threshold) {
		#	$cur_runs = $cur_runs - 1 ;
		#        print "cur_runs = $cur_runs\n" ;
		#} else {
		#      $total_time += $cur_time ;
		#      print "cur_time = $cur_time\n" ;
		#}
		sleep(3) ;
	}
	if($cur_runs > 0) {
		print "Average time : m3=$size_m3,dsp=$size_dsp : $total_time, $total_a9_time, $total_m3_time, $total_dsp_time\n" ;
		$results[$gindx] = "Average time : m3=$size_m3,dsp=$size_dsp : $total_time, $total_a9_time, $total_m3_time, $total_dsp_time" ;
		$gindx = $gindx+1 ;
	} else {
		print "Number of valid runs equal to zero\n" ;
	}
}

sub find_avg_a9_time
{
	my $size_a9 = $_[0] ;
	my $num_runs = $_[1] ;
	my $total_time = 0 ;
	my $cur_runs = $num_runs ;
	for($i=0 ; $i<$num_runs ; $i++) {
		my $time_threshold ;
		my @time ;
		print "sudo ./app -l 2 -x $size_a9\n" ;
		@time = `sudo ./app -l 2 -x $size_a9` ;
		my $length = @time ;
		my $time_str= $time[$length-1] ;
		chomp($time_str) ;
		my $cur_time = 0 ;
		if($time_str =~ m/^Time[\s\S]+\d+\.\d+\,(\d+\.\d+)\,\d+\.\d+\,\d+\.\d+\,\d+\.\d+\,\d+\.\d+\>$/) {
			$cur_time = $1 ;
		}
		$total_time += $cur_time ;
		print "cur_time = $cur_time\n" ;
		sleep(3) ;
	}
	if($cur_runs > 0) {
		my $avg_time = $total_time/$cur_runs ;
		print "Average time : m3=$size_m3,dsp=$size_dsp : $avg_time\n" ;
		$results[$gindx] = "Average time : m3=$size_m3,dsp=$size_dsp : $avg_time" ;
		$gindx = $gindx+1 ;
	} else {
		print "Number of valid runs equal to zero\n" ;
	}
}

#find_avg_time(210, 470, 5) ;
#find_avg_time(216, 460, 5) ;
#find_avg_time(0, 0, 5) ;
#find_avg_time(1024, 0, 5) ;
#find_avg_time(0, 1024, 5) ;
#find_avg_time(64, 192, 5) ;
#find_avg_time(96, 192, 5) ;
#find_avg_time(92, 194, 5) ;
#find_avg_time(88, 188, 5) ;
#find_avg_time(416, 416, 5) ;
#find_avg_time(416, 448, 5) ;
#find_avg_time(416, 480, 5) ;
#find_avg_time(534, 576, 5) ;
#find_avg_time(448, 416, 5) ;
#find_avg_time(448, 448, 5) ;
#find_avg_time(448, 480, 5) ;
#find_avg_time(480, 416, 5) ;
#find_avg_time(480, 448, 5) ;
#find_avg_time(480, 480, 5) ;
#find_a9_avg_time(2, 10) ;
#for($indx1=20 ; $indx1<=25 ; $indx1++) {
#	for($indx2=45 ; $indx2<=50 ; $indx2++) {
#		my $block_size = 4 ;
#		my $m3_size = $indx1 * $block_size ;
#		my $dsp_size = $indx2 * $block_size ;
#		find_avg_time($m3_size, $dsp_size, 5) ;
#	}
#}

for($indx1=20 ; $indx1<=20 ; $indx1+=4) {
	for($indx2=0 ; $indx2<=64 ; $indx2+=4) {
		my $block_size = 16 ;
		my $m3_size = $indx1 * $block_size ;
		my $dsp_size = $indx2 * $block_size ;
		if($m3_size + $dsp_size <= 1024) {
			find_avg_time($m3_size, $dsp_size, 1) ;
		}
	}
}

#for($indx1=0 ; $indx1<=1 ; $indx1++) {
#	for($indx2=0 ; $indx2<=32 ; $indx2++) {
#		my $block_size = 32 ;
#		my $m3_size = $indx1 * $block_size ;
#		my $dsp_size = $indx2 * $block_size ;
#		if($m3_size + $dsp_size <= 1024) {
#			find_avg_time($m3_size, $dsp_size, 1) ;
#		}
#	}
#}

#for($indx1=256 ; $indx1>=0 ; $indx1--) {
#	my $block_size = 4 ;
#	my $a9_size = $indx1 * $block_size ;
#	find_avg_a9_time($a9_size, 1) ;
#}

#find_avg_time(88, 188, 1) ;

foreach $item (@results) {
	print "$item\n" ;
}
