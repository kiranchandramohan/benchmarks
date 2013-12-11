#include <stdlib.h>
#include <stdio.h>

#include "simpletest.h"
#include "util.h"
#include "test.h"
#include "barrier.h"

extern pthread_barrier_t barrier0, barrier1, barrier2 ;

void multiply(int* A, int* B, int* C, int start, int end)
{
	int i,j,k;
	for (i = start ; i < end ; i++)
	{   
		for (j = 0; j < SIZE; j++)
		{   
			C[i*SIZE+j] = 0;
			for ( k = 0; k < SIZE; k++)
				C[i*SIZE+j] += A[i*SIZE+k]*B[k*SIZE+j];
		}   
	}
}

int local_A[SIZE*SIZE] ;
int local_B[SIZE*SIZE] ;
int local_C[SIZE*SIZE] ;
int verify_result(struct buffer* bufs)
{
	int pass = 1 ;
	//int i, j ;
	//int* C = bufs[2].bo[0]->map ;
	//
	//multiply(local_A, local_B, local_C, 0, SIZE) ;
	//for(i=0 ; i<SIZE ; i++)
	//	for(j=0 ; j<SIZE ; j++) {
	//		if(C[i*SIZE+j] != local_C[i*SIZE+j]) {
	//			printf("Difference in i=%d,j=%d\n", i, j) ;
	//			pass = 0 ;
	//			break ;
	//		}

	//	}

	//if(pass) {
	//	printf("SUCCESS\n") ;
	//} else {
	//	printf("FAIL\n") ;
	//}

	return pass ;
}

void set_buffer_properties(struct buffer* bufs, int nbuf)
{
	int i ;
	for(i=0 ; i<nbuf ; i++) {
		//bufs[i].cache_type = OMAP_BO_WC ;
		//bufs[i].cache_type = OMAP_BO_UNCACHED ;
		bufs[i].cache_type = OMAP_BO_CACHED ;
		bufs[i].privatize = false ;
		bufs[i].rd_only = false ;
		bufs[i].nbo = 1 ;
		bufs[i].remote_attach = true ;

		bufs[i].width = SIZE ;
		bufs[i].height = SIZE ;
	}
}

void init_buffers(struct buffer* bufs, int nbuf)
{
	int i, j, k ;
	int init_val = 0 ;
	for(i=0 ; i<nbuf ; i++) {
		if(i==0)
			init_val = 3 ;
		else if(i==1)
			init_val = 5 ;
		else
			init_val = 0 ;

		for(j=0 ; j<SIZE ; j++) {
			for(k=0 ; k<SIZE ; k++) {
				((int*)bufs[i].bo[0]->map)[j*SIZE+k] = init_val ;
				if(i==0)
					local_A[j*SIZE+k] = init_val ;
				else if(i==1)
					local_B[j*SIZE+k] = init_val ;
				else
					local_C[j*SIZE+k] = init_val ;
			}
		}
	}
}

void a9_compute(int tid, struct buffer* buffers, int start_indx, int end_indx)
{
	int* A = buffers[0].bo[0]->map ;
	int* B = buffers[1].bo[0]->map ;
	int* C = buffers[2].bo[0]->map ;

	int i ;
	for(i=0 ; i<NUM_ITER ; i++)
	{
		if(tid == 0) {
			call_barrier(0) ;
		}

		pthread_barrier_wait(&barrier0) ;
		multiply(A, B, C, start_indx, end_indx) ;
	}
}

int find_remaining_size(int size_others)
{
	return SIZE - size_others ;
}
