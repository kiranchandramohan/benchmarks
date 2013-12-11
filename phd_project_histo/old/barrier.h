#ifndef _BARRIER_H
#define _BARRIER_H

#define MY_MACIG 'G'
#define READ_IOCTL _IOR(MY_MACIG, 0, int)
#define WRITE_IOCTL _IOW(MY_MACIG, 1, int)
#define BARRIER_IOCTL _IOW(MY_MACIG, 2, struct barrier*)
#define LOCK_IOCTL _IO(MY_MACIG, 3)
#define UNLOCK_IOCTL _IO(MY_MACIG, 4)
#define DEBUG_IOCTL _IO(MY_MACIG, 5)
#define INIT_BARRIER_IOCTL _IOW(MY_MACIG, 6, int*)
#define FINALIZE_BARRIER_IOCTL _IO(MY_MACIG, 7)

#define NUM_BARRIER 3

struct barrier
{
	int indx ;
	int count ;
} ;

void call_barrier(int indx)  ;
void init_barrier(int num_threads_barrier) ;
void open_barrier(void) ;
void close_barrier(void) ;
void finalize_barrier(void) ;

#endif
