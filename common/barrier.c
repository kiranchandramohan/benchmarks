#include "barrier.h"
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

int barrier_fd ;

void call_barrier(int indx) 
{
	if(ioctl(barrier_fd, BARRIER_IOCTL, (int *)&indx) < 0)
		perror("barrier ioctl") ;
	//printf("Finished barrier = %d\n", indx) ;
}

int call_read_reduce(int red_num)
{
	struct my_buffer mybuf ;
	mybuf.red_num = red_num ;
	mybuf.inc_val = 0 ;
	if(ioctl(barrier_fd, READ_REDUCE_IOCTL, (struct my_buffer *)&mybuf) < 0)
		perror("reduce ioctl") ;
	printf("Finished read reduce\n") ;
	return mybuf.inc_val ;
}

void init_reduce(int red_num, int inc_val)
{
	struct my_buffer mybuf ;
	mybuf.red_num = red_num ;
	mybuf.inc_val = inc_val ;
	if(ioctl(barrier_fd, INIT_REDUCE_IOCTL, (struct my_buffer *)&mybuf) < 0)
		perror("init reduce ioctl") ;
	//printf("Finished init reduce\n") ;
}

void call_reduce(int red_num, int inc_val)
{
	struct my_buffer mybuf ;
	mybuf.red_num = red_num ;
	mybuf.inc_val = inc_val ;
	if(ioctl(barrier_fd, REDUCE_IOCTL, (struct my_buffer *)&mybuf) < 0)
		perror("reduce ioctl") ;
	//printf("Finished reduce\n") ;
}

void init_barrier(int num_threads_barrier)
{
	struct barrier b ;
	b.indx = NUM_BARRIER ;
	b.count = num_threads_barrier ;
	#ifdef DEBUG
	printf("Initializing barrier with count = %d\n", num_threads_barrier) ;
	#endif
	if(ioctl(barrier_fd, INIT_BARRIER_IOCTL, (struct barrier*)&b) < 0)
		perror("init barrier ioctl") ;
}

void init_count_barrier(int num_threads_barrier)
{
	struct barrier b ;
	b.indx = NUM_BARRIER ;
	b.count = num_threads_barrier ;
	printf("Initializing barrier with count = %d\n", num_threads_barrier) ;
	if(ioctl(barrier_fd, INIT_COUNT_BARRIER_IOCTL, (struct barrier*)&b) < 0)
		perror("init barrier ioctl") ;
}

void lock_hwspinlock(int lock_id)
{
	if(ioctl(barrier_fd, LOCK_IOCTL, (int *)&lock_id) < 0)
		perror("lock ioctl") ;
}

void unlock_hwspinlock(int lock_id)
{
	if(ioctl(barrier_fd, UNLOCK_IOCTL, (int *)&lock_id) < 0)
		perror("lock ioctl") ;
}

void open_barrier(void)
{
	if ((barrier_fd = open("/dev/hw_spinlock", O_RDWR)) < 0) {
		perror("open");
	}
}

void finalize_barrier(void)
{
	if(ioctl(barrier_fd, FINALIZE_BARRIER_IOCTL) < 0)
		perror("finalize barrier ioctl") ;
}

void close_barrier(void)
{
	if (close(barrier_fd) < 0) {
		perror("close");
	}
}
