#include <stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include "timer.h"

#define SIZE 2048
#define NUM_ITER 100

int A[SIZE*SIZE], B[SIZE*SIZE] ;
int result ;
pthread_mutex_t thread_mutex ;
pthread_barrier_t barrier0, barrier1 ;

void dot_product(int* A, int* B, int* result, int start, int end)
{
	int i ;
	int sum = 0 ;
	for (i = start ; i < end ; i++)
	{
		A[i] = i ;
		B[i] = i ;
	}

	for (i = start ; i < end ; i++)
	{
		sum += A[i] * B[i] ;
	}

	int rc = pthread_mutex_lock(&thread_mutex);
	result[0] += sum ;
	pthread_mutex_unlock(&thread_mutex);
}

int serial_dot_product(int* A, int* B)
{
	int i ;
	int sum = 0 ;
	for (i = 0 ; i < SIZE*SIZE ; i++)
	{
		A[i] = i ;
		B[i] = i ;
	}

	for (i = 0 ; i < SIZE*SIZE ; i++)
	{
		sum += A[i] * B[i] ;
	}

	return sum ;
}

void* local_compute(void* x)
{
	int k ;
	int mid = (SIZE*SIZE)/2 ;
	for(k=0 ; k<NUM_ITER ; k++) {
		pthread_barrier_wait(&barrier0);
		result = 0 ;
		pthread_barrier_wait(&barrier1);
		if(x == 0) {
			dot_product(A, B, &result, 0, mid) ;
		} else {
			dot_product(A, B, &result, mid, SIZE*SIZE) ;
		}
	}
}

int run_parallel()
{
	pthread_t thread1, thread2 ;
	int ret1, ret2 ;	
	pthread_mutex_init(&thread_mutex, NULL);
	ret1 = pthread_create( &thread1, NULL, local_compute, (void*) 0);
	ret1 = pthread_create( &thread2, NULL, local_compute, (void*) 1);
	
	pthread_join(thread1, NULL) ;
	pthread_join(thread2, NULL) ;

	return result ;
}

int run_serial()
{
	return serial_dot_product(A,B) ;
}

int main()
{
	struct timer tm ;
	init_timer(&tm) ;

	pthread_barrier_init(&barrier0,NULL,2);
	pthread_barrier_init(&barrier1,NULL,2);
	start_timer(&tm) ;
	int ret1 = run_parallel() ;
	stop_timer(&tm) ; 
	print_time(&tm) ;

	int ret2 = run_serial() ;
	if(ret1 != ret2) {
		printf("Mismatch\n") ;
		printf("par=%d,ser=%d\n", ret1, ret2) ;
	} else
		printf("Success %d\n", ret1) ;

	return 0 ;
}
