#!/usr/bin/perl

my $time_cmd = "/usr/bin/time -f \"%E real,%U user,%S sys\"" ;
my $disp="6:800x600" ;

my @configs = ([1,2],[1,9],[2,8],[3,5],[3,9],[4,8],[4,10],[4,12],[4,11]) ;

my @segf_configs = () ;
my @pass_configs = () ;
my @fail_configs = () ;

sub run_exp
{
	my $sysm3 = $_[0] ;
	my $dsp = $_[1] ;
	my @res = `sudo $time_cmd ./simpletest -s $disp -r sysm3=$sysm3,dsp=$dsp -l 2 | grep Verified` ;

	my $pass=-1 ;
	foreach my $l (@res)
	{
		print "Line=$l\n" ;
		if($l =~ m/Verified\s(\S+)/) {
			if($1 eq "PASS") {
				$pass = 1 ;
			} elsif($1 eq "FAIL") {
				$pass = 0 ;
			}
		}
	}

	return $pass ;
}

sub run_exps
{
	foreach my $c (@configs)
	{
		my ($sysm3,$dsp) = @{$c} ;
		my $ret_val = run_exp($sysm3,$dsp) ;
		if($ret_val == 1) {
			push (@pass_configs, $c) ;
		} elsif($ret_val == 0) {
			push (@fail_configs, $c) ;
		} elsif($ret_val == -1) {
			push (@segf_configs, $c) ;
		}
		sleep 1 ;
	}

	foreach my $c (@pass_configs)
	{
		my ($sysm3,$dsp) = @{$c} ;
		print "Config : $sysm3,$dsp : PASS\n" ;
	}
	foreach my $c (@fail_configs)
	{
		my ($sysm3,$dsp) = @{$c} ;
		print "Config : $sysm3,$dsp : FAIL\n" ;
	}
	foreach my $c (@segf_configs)
	{
		my ($sysm3,$dsp) = @{$c} ;
		print "Config : $sysm3,$dsp : SEGF\n" ;
	}

	if(@segf_configs) {
		@configs = @segf_configs ;
		@segf_configs = () ;
		return 1 ;
	} else {
		@configs = () ;
		return 0 ;
	}

}

my $counter = 0 ;
my $repeat = 0 ;
do {
	$counter = $counter + 1 ;
	print "Attempt #$counter\n" ;
	$repeat = run_exps() ;
} while ($repeat) ;
