#include<stdio.h>
#include<pthread.h>
#include "timer.h"
#define NR 128
#define NQ 128
#define NP 128
#define NUM_ITER 30
int pA[NR*NQ*NP] ;
int pSUM[NR*NQ*NP] ;
int myA[NR*NQ*NP] ;
int mySUM[NR*NQ*NP] ;
pthread_barrier_t barrier0 ;

void doitgen(int* __restrict A, int* __restrict sum, int start_indx, int end_indx)
{
	int r, q, p, s ; 
	int C4[NP*NP] ;

	int i, j ;
	for (i = 0; i < NP; i++)
		for (j = 0; j < NP; j++)
			C4[i*NP+j] = ((int) i*j) / NP ;

	for (r = start_indx ; r < end_indx ; r++)
		for (q = 0 ; q < NQ ; q++)
			for (p = 0; p < NP; p++)
				A[r*NQ*NP+q*NP+p] = ((int) r*q + p) / NP ;

	for (r = start_indx ; r < end_indx ; r++)
		for (q = 0; q < NQ; q++) {
			for (p = 0; p < NP; p++) {
				sum[r*NQ*NP+q*NP+p] = 0;
				for (s = 0; s < NP; s++)
					sum[r*NQ*NP+q*NP+p] = sum[r*NQ*NP+q*NP+p] + 
							A[r*NQ*NP+q*NP+s] * C4[s*NP+p];
			}   
			for (p = 0; p < NR; p++)
				A[r*NQ*NP+q*NP+p] = sum[r*NQ*NP+q*NP+p];
		}   
}

void* compute(void* tid)
{
	int i ;
	for(i=0 ; i<NUM_ITER ; i++) {
		pthread_barrier_wait(&barrier0);
		if(tid == 0) {
			doitgen(pA, pSUM, 0, NR/2) ;
		} else {
			doitgen(pA, pSUM, NR/2, NR) ;
		}
	}
}

void serial_doitgen(int* A, int* sum)
{
	int r, q, p, s ; 
	int C4[NP*NP] ;

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
				for (s = 0; s < NP; s++) {
					sum[r*NQ*NP+q*NP+p] = 
					sum[r*NQ*NP+q*NP+p] + A[r*NQ*NP+q*NP+s] * C4[s*NP+p];
				}
			}   
			for (p = 0; p < NR; p++)
				A[r*NQ*NP+q*NP+p] = sum[r*NQ*NP+q*NP+p];
		}   
}

int main()
{
	struct timer tm ;
	init_timer(&tm) ;
	pthread_t thread1, thread2;
	pthread_barrier_init(&barrier0,NULL,2);

	start_timer(&tm) ;
	int ret1 = pthread_create( &thread1, NULL, compute, (void*) 0);
	int ret2 = pthread_create( &thread2, NULL, compute, (void*) 1);
	pthread_join(thread1, NULL) ;
	pthread_join(thread2, NULL) ;
	stop_timer(&tm) ; 

	print_time(&tm) ;

	serial_doitgen(myA, mySUM) ;
	int i, j, k ;
	int pass = 1 ;
	for(i=0 ; i<NR ; i++) {
		for(j=0 ; j<NQ ; j++) {
			for(k=0 ; k<NP ; k++) {
				if(mySUM[i*NQ*NP+j*NP+k] != pSUM[i*NQ*NP+j*NP+k])
					pass = 0 ;
				if(myA[i*NQ*NP+j*NP+k] != pA[i*NQ*NP+j*NP+k])
					pass = 0 ;
			}
		}
	}

	if(pass)
		printf("Verification : SUCCESS\n") ;
	else
		printf("Verification : FAILURE\n") ;

	return 0 ;
}

