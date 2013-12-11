#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "simpletest.h"
#include "util.h"
#include "test.h"
#include "barrier.h"

pthread_barrier_t barrier0 ;

//void doitgen(int* A, int* sum, int start_indx, int end_indx)
void doitgen(int* A, int* sum, int* C4, int start_indx, int end_indx)
{
	int r, q, p, s ; 
	int tmp0, tmp1, tmp2 ;

	for (r = 0; r < NP; r++)
		for (q = 0; q < NP; q++)
			C4[r*NP+q] = ((int) r*q) / NP ;

	for (r = start_indx ; r < end_indx ; r++)
		for (q = 0 ; q < NQ ; q++)
			for (p = 0; p < NP; p++)
				A[r*NQ*NP+q*NP+p] = ((int) r*q + p) / NP ;

	for (r = start_indx ; r < end_indx ; r++) {
		for (q = 0; q < NQ; q++) {
			tmp0 = r*NQ*NP+q*NP ;
			for (p = 0; p < NP; p++) {
				tmp1 = tmp0+p ;
				sum[tmp1] = 0;
				for (s = 0; s < NP; s++) {
				    tmp2 = tmp0+s ;
                                    sum[tmp1] = sum[tmp1] + A[tmp2] * C4[s*NP+p] ;
				}
			}
   
			for (p = 0; p < NR; p++) {
				tmp1 = tmp0+p ;
				A[tmp1] = sum[tmp1] ;
			}
		}
	}   
}

void serial_doitgen(int* A, int* sum, int* C4)
{
	int r, q, p, s ; 

	for (r = 0 ; r < NR ; r++)
		for (q = 0 ; q < NQ ; q++)
			for (p = 0; p < NP; p++)
				A[r*NQ*NP+q*NP+p] = ((int) r*q + p) / NP ;

	for (r = 0; r < NP; r++)
		for (q = 0; q < NP; q++)
			C4[r*NP+q] = ((int) r*q) / NP ;

	for (r = 0; r < NR; r++)
		for (q = 0; q < NQ; q++) {
			for (p = 0; p < NP; p++) {
				sum[r*NQ*NP+q*NP+p] = 0;
				for (s = 0; s < NP; s++)
					sum[r*NQ*NP+q*NP+p] = sum[r*NQ*NP+q*NP+p] + A[r*NQ*NP+q*NP+s] * C4[s*NP+p];
			}   
			for (p = 0; p < NR; p++)
				A[r*NQ*NP+q*NP+p] = sum[r*NQ*NP+q*NP+p];
		}   
}

int C41[NP*NP] ;
int C42[NP*NP] ;
void a9_compute(int tid, struct buffer* buffers, int start_indx, int end_indx)
{
	int i ;
	for(i=0 ; i<NUM_ITER ; i++)
	{
		if(tid == 0) {
		     call_barrier(0) ;
		     pthread_barrier_wait(&barrier0) ;
		     doitgen(buffers[0].bo[0]->map, buffers[1].bo[0]->map, C41, 
				start_indx, end_indx) ;
		} else {
		     pthread_barrier_wait(&barrier0) ;
		     doitgen(buffers[0].bo[0]->map, buffers[1].bo[0]->map, C42, 
				start_indx, end_indx) ;
		}
	}
}

int verify_result(struct buffer* buffers)
{
	int* buffer1 = buffers[0].bo[0]->map ;
	int* buffer2 = buffers[1].bo[0]->map ;

	int* A = (int*)malloc(NR*NQ*NP*sizeof(int)) ;
	int* sum = (int*)malloc(NR*NQ*NP*sizeof(int)) ;
	int* C4 = (int*)malloc(NP*NP*sizeof(int)) ;

	serial_doitgen(A, sum, C4) ;

	int pass = 1 ;

	int r,q,p ;
	for (r = 0 ; r < NR ; r++)
		for (q = 0 ; q < NQ ; q++)
			for (p = 0; p < NP; p++) {
				if(A[r*NQ*NP+q*NP+p] != buffer1[r*NQ*NP+q*NP+p])
					pass = 0 ;
			}

	for (r = 0 ; r < NR ; r++)
		for (q = 0 ; q < NQ ; q++)
			for (p = 0; p < NP; p++) {
				if(sum[r*NQ*NP+q*NP+p] != buffer2[r*NQ*NP+q*NP+p])
					pass = 0 ;
			}
	
	return pass ;
}

void set_buffer_properties(struct buffer* bufs, int nbuf)
{
	int i ;
	for(i=0 ; i<nbuf ; i++) {
		//bo_flags |= OMAP_BO_UNCACHED ;
		//bo_flags |= OMAP_BO_WC ;
		//bo_flags |= OMAP_BO_CACHED;
		bufs[i].cache_type = OMAP_BO_CACHED ;
		bufs[i].privatize = false ;
		bufs[i].rd_only = false ;
		bufs[i].remote_attach = true ;
		if(i>=2) {
			bufs[i].width = NP ;
			bufs[i].height = NP ;
		} else {
			bufs[i].width = NR ;
			bufs[i].height = NQ*NP ;
		}
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
	int i, m, j, k, l ;
	for(i=0 ; i<2 ; i++) {
		for(m=0 ; m<bufs[i].nbo ; m++) {
			for(j=0 ; j<NR ; j++) {
				for(k=0 ; k<NQ ; k++) {
					for(l=0 ; l<NP ; l++) {
						((int*)bufs[i].bo[m]->map)[j*NQ*NP+k*NP+l] = j+k+l ;
					}
				}
			}
		}
	}
	for(i=2 ; i<nbuf ; i++) {
		for(m=0 ; m<bufs[i].nbo ; m++) {
			for(j=0 ; j<NP ; j++) {
				for(k=0 ; k<NP ; k++) {
			((int*)bufs[i].bo[m]->map)[j*NP+k] = 0 ;
				}
			}
		}
	}
}

int find_remaining_size(int size_others)
{
	return NR - size_others ;
}
