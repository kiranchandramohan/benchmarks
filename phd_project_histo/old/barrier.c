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
}

void init_barrier(int num_threads_barrier)
{
	struct barrier b ;
	b.indx = NUM_BARRIER ;
	b.count = num_threads_barrier ;
	if(ioctl(barrier_fd, INIT_BARRIER_IOCTL, (struct barrier*)&b) < 0)
		perror("init barrier ioctl") ;
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
