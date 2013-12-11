#include "timer.h"
#include <stdio.h>
#include <errno.h>

struct timezone tz ;

void init_timer(struct timer* tm)
{
	tm->is_running = FALSE ;
	tm->time.tv_sec = 0 ;
	tm->time.tv_usec = 0 ;
	tm->accumulator.tv_sec = 0 ;
	tm->accumulator.tv_usec = 0 ;
}

void start_timer(struct timer* tm)
{
	if(tm->is_running == TRUE) {
		perror("Starting a running timer\n") ;
		exit(-1) ;
	}
	gettimeofday(&(tm->time), &tz) ;
	tm->is_running = TRUE ;
}

void add_time(struct timeval* t1, struct timeval* t2, struct timeval* t3)
{
	long int time = ((t1->tv_sec * 1000000 + t1->tv_usec) + (t2->tv_sec * 1000000 + t2->tv_usec)) ;
	t3->tv_sec += time/1000000 ;
	t3->tv_usec += time%1000000 ;
}

void subtract_time(struct timeval* t1, struct timeval* t2, struct timeval* t3)
{
        long int time = ((t1->tv_sec * 1000000 + t1->tv_usec) - (t2->tv_sec * 1000000 + t2->tv_usec)) ;
	t3->tv_sec += time/1000000 ;
	t3->tv_usec += time%1000000 ;
}

void stop_timer(struct timer* tm) 
{
	struct timeval tv_new ;
	if(tm->is_running == FALSE) {
		perror("Stopping a timer which is not running\n") ;
		exit(-1) ;
	}
	gettimeofday(&(tv_new), &tz) ;
	tm->is_running = FALSE ;
        subtract_time(&tv_new, &(tm->time), &(tm->accumulator)) ;
}

long int get_integer_time(struct timer* tm)
{
        long int time = (tm->accumulator.tv_sec * 1000000 + tm->accumulator.tv_usec)/1000000 ;
	return time ;
}

float get_float_time(struct timer* tm)
{
        float time = (float)(tm->accumulator.tv_sec * 1000000 + tm->accumulator.tv_usec)/1000000.00 ;
	return time ;
}

void print_integer_time_summary(struct timer* tm, struct timer* tm_a9, struct timer* tm_a9_0, 
	struct timer* tm_a9_1, struct timer* tm_m3, struct timer* tm_dsp)
{
        long int total_time = get_integer_time(tm) ;
	long int a9_sync_time = get_float_time(tm_a9) ;
	long int a9_0_time = get_float_time(tm_a9_0) ;
	long int a9_1_time = get_float_time(tm_a9_1) ;
	long int a9_time = (a9_0_time >= a9_1_time) ? a9_0_time : a9_1_time ;
	long int m3_time = get_integer_time(tm_m3) ;
	long int dsp_time = get_integer_time(tm_dsp) ;

        printf("Time<total,a9,m3,dsp> = <%ld,%ld,%ld,%ld,%ld>\n", total_time, a9_sync_time, a9_time, m3_time, dsp_time) ;
}

void print_float_time_summary(struct timer* tm, struct timer* tm_a9, struct timer* tm_a9_0, 
	struct timer* tm_a9_1, struct timer* tm_m3, struct timer* tm_dsp)
{
        float total_time = get_float_time(tm) ;
	float a9_sync_time = get_float_time(tm_a9) ;
	float a9_0_time = get_float_time(tm_a9_0) ;
	float a9_1_time = get_float_time(tm_a9_1) ;
	float a9_time = (a9_0_time >= a9_1_time) ? a9_0_time : a9_1_time ;
	float m3_time = get_float_time(tm_m3) ;
	float dsp_time = get_float_time(tm_dsp) ;

        printf("Time<total,a9-sync,a9,m3,dsp> = <%f,%f,%f,%f,%f>\n", total_time, a9_sync_time, a9_time, m3_time, dsp_time) ;
}
