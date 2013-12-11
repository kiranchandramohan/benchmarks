#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "simpletest.h"
#include "barrier.h"
#include "util.h"
#include "test.h"

pthread_barrier_t barrier0, barrier1, barrier2 ;
pthread_mutex_t thread_mutex ;

void dot_product(int* __restrict__ A, int* __restrict__ B, int* __restrict__ result, int start, int end)
{
	int i ;
	int sum = 0 ;
	for (i = start ; i < end ; i++)
	{
		A[i] = B[i] = i ;
	}

	for (i = start ; i < end ; i++)
	{
		sum += A[i] * B[i] ;
	}

	*result = sum ;
}

int serial_dot_product(int* A, int* B)
{
	int i ;
	int sum = 0 ;
	for (i = 0 ; i < SIZE*SIZE ; i++)
	{
		A[i] = B[i] = i ;
	}

	for (i = 0 ; i < SIZE*SIZE ; i++)
	{
		sum += A[i] * B[i] ;
	}

	printf("Serial Sum = %d\n", sum) ;
	return sum ;
}

void set_buffer_properties(struct buffer* bufs, int nbuf)
{
	int i ;
	for(i=0 ; i<nbuf ; i++) {
		//bo_flags |= OMAP_BO_UNCACHED ;
		//bo_flags |= OMAP_BO_WC ;
		//bo_flags |= OMAP_BO_CACHED;
		bufs[i].cache_type = OMAP_BO_CACHED ;
		//bufs[i].cache_type = OMAP_BO_UNCACHED ;
		bufs[i].privatize = false ;
		bufs[i].rd_only = false ;
		bufs[i].remote_attach = true ;

		bufs[i].width = SIZE ;
		bufs[i].height = SIZE ;
		bufs[i].nbo = 1 ;
		//if(i==0) {
		//	bufs[i].width = SIZE ;
		//	bufs[i].height = SIZE ;
		//	bufs[i].nbo = 1 ;
		//} else {
		//	bufs[i].width = SIZE ;
		//	bufs[i].height = 1 ;
		//	bufs[i].nbo = 2 ;
		//	if(i==1) {
		//		bufs[i].privatize = true ;
		//		if(remote_fd.sysm3 >= 0)
		//			bufs[i].nbo ++ ;
		//		if(remote_fd.dsp >= 0)
		//			bufs[i].nbo = 4 ;
		//	}
		//}
	}
}

void init_buffers(struct buffer* bufs, int nbuf)
{
	int i, j, k, l ;
	for(i=0 ; i<nbuf ; i++) {
		for(l=0 ; l<bufs[i].nbo ; l++) {
			for(j=0 ; j<SIZE ; j++) {
				for(k=0 ; k<SIZE ; k++) {
					((int*)bufs[i].bo[l]->map)[j*SIZE+k] = j*k ;
				}
			}
		}
	}
}

int mem[2] ;
void a9_compute(int tid, struct buffer* buffers, int start_indx, int end_indx)
{
	int i ;
	for(i=0 ; i<NUM_ITER ; i++)
	{
		if(tid == 0) {
			call_barrier(0) ;
			mem[0] = mem[1] = 0 ;
			init_reduce(0, 0) ;
			call_barrier(1) ;
			pthread_barrier_wait(&barrier1);
			dot_product(buffers[0].bo[0]->map, buffers[1].bo[0]->map, &mem[0], 
						start_indx, end_indx) ;
			pthread_barrier_wait(&barrier2) ;
			
			call_reduce(0, mem[0]+mem[1]) ;
			call_barrier(2) ;
		} else {
			pthread_barrier_wait(&barrier1);
			dot_product(buffers[0].bo[0]->map, buffers[1].bo[0]->map, &mem[1], 
						start_indx, end_indx) ;
			pthread_barrier_wait(&barrier2);
		}
	}

}

int verify_result(struct buffer* bufs)
{
	int* buffer1 = bufs[0].bo[0]->map ;
	int* buffer2 = bufs[1].bo[0]->map ;
	int dotp = serial_dot_product(buffer1, buffer2) ;

	int* buffer3 = bufs[2].bo[0]->map ;
	buffer3[0] = call_read_reduce(0) ;
	if(buffer3[0] != dotp) {
		printf("dotp = %d, buffer3[0]=%d\n", dotp,buffer3[0]) ;
		return false ;
	} else
		return true ;
}

int find_remaining_size(int size_others)
{
	return SIZE*SIZE - size_others ;
}
