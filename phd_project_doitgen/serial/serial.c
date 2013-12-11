#include<stdio.h>
#include "timer.h"
#define NR 128
#define NQ 128
#define NP 128
#define NUM_ITER 30
int myA[NR*NQ*NP] ;
int mySUM[NR*NQ*NP] ;

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

	start_timer(&tm) ;
	int i ;
	for(i=0 ; i<NUM_ITER ; i++)
		serial_doitgen(myA, mySUM) ;
	stop_timer(&tm) ; 
	print_time(&tm) ;

	return 0 ;
}
