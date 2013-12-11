#include <stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include "timer.h"
#define SIZE 2048
#define NUM_ITER 1
int A[SIZE*SIZE], B[SIZE*SIZE] ;

int serial_dot_product(int* A, int* B)
{
	int i, k ;
	int sum ;
	for(k=0 ; k<NUM_ITER ; k++)
	{
		sum = 0 ;
		for (i = 0 ; i < SIZE*SIZE ; i++)
		{
			A[i] = i ;
			B[i] = i ;
		}

		for (i = 0 ; i < SIZE*SIZE ; i++)
		{
			sum += A[i] * B[i] ;
		}
	}

	//printf("Serial Sum = %d\n", sum) ;
	return sum ;
}

int run_serial()
{
	return serial_dot_product(A,B) ;
}

int main()
{
	struct timer tm ;
	init_timer(&tm) ;

	start_timer(&tm) ;
	int ret = run_serial() ;
	stop_timer(&tm) ; 

	print_time(&tm) ;

	return 0 ;
}
