#ifndef TIMER_H
#define TIMER_H

#define FALSE 0
#define TRUE 1

#include <sys/time.h>
#include <stdlib.h>

struct timer
{
	int is_running ;
        struct timeval time ;
        struct timeval accumulator ;
} ;

void init_timer(struct timer* tm) ;
void start_timer(struct timer* tm) ;
void add_time(struct timeval* t1, struct timeval* t2, struct timeval* t3) ;
void subtract_time(struct timeval* t1, struct timeval* t2, struct timeval* t3) ;
void stop_timer(struct timer* tm) ;
long int get_integer_time(struct timer* tm) ;
float get_float_time(struct timer* tm) ;
void print_integer_time_summary(struct timer* tm, struct timer* tm_a9, struct timer* tm_a9_0, 
			struct timer* tm_a9_1, struct timer* tm_m3, struct timer* tm_dsp) ;
void print_float_time_summary(struct timer* tm, struct timer* tm_a9, struct timer* tm_a9_0,
			struct timer* tm_a9_1, struct timer* tm_m3, struct timer* tm_dsp) ;

#endif
