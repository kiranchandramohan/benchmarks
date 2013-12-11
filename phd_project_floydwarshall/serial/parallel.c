#include<stdio.h>
#include<pthread.h>
#include "timer.h"

#define N 2048
int path1[N][N] ;
int path2[N][N] ;
pthread_barrier_t barrier ;

struct message
{
	int start_indx ;
	int end_indx ;
	int(*buffer)[N] ;
} ;

inline int mod(int n)
{
	if(n<0)
		return -n ;
	else
		return n ;
}

void* p_fw_single_iter(void* msg)
{
	struct message* mesg = msg ;
	int(*path)[N] = mesg->buffer ;
	int start_indx = mesg->start_indx ;
	int end_indx = mesg->end_indx ;
	int i, j, k ;

	printf("Computing FW from %d to %d\n", start_indx, end_indx) ;

	for(i=start_indx ; i<end_indx ; i++)
		for(j=0 ; j<N ; j++)
			path[i][j] = mod(i-j);

	for (k = 0; k < N; k++)
	{
		pthread_barrier_wait(&barrier);
		for(i = start_indx ; i < end_indx ; i++)
			for (j = 0; j < N; j++)
				path[i][j] = path[i][j] < path[i][k] + path[k][j] ? 
					path[i][j] : path[i][k] + path[k][j] ;
	}
}

void p_floyd_warshall(int path[N][N])
{
	int i, j, k;
	pthread_t thread1, thread2 ;
	int ret1, ret2 ;
	struct message message1, message2 ;
	message1.start_indx = 0 ;
	message1.end_indx = N/2 ;
	message2.start_indx = N/2 ;
	message2.end_indx = N ;
	message1.buffer = message2.buffer = path ;
	ret1 = pthread_create( &thread1, NULL, p_fw_single_iter, (void*) &message1);
	ret2 = pthread_create( &thread2, NULL, p_fw_single_iter, (void*) &message2);
	pthread_join(thread1, NULL) ;
	pthread_join(thread2, NULL) ;
}

void floyd_warshall(int path[N][N])
{
	int i, j, k;

	for(i=0 ; i<N ; i++)
		for(j=0 ; j<N ; j++)
			path[i][j] = mod(i-j) ;

	for (k = 0; k < N; k++)
	{   
		for(i = 0; i < N; i++)
			for (j = 0; j < N; j++)
				path[i][j] = path[i][j] < path[i][k] + path[k][j] ? 
					path[i][j] : path[i][k] + path[k][j] ;
	}   
}

int verify(int path1[N][N], int path2[N][N])
{
	int i, j ;
	for(i = 0; i < N; i++)
		for (j = 0; j < N; j++)
			if(path1[i][j] != path2[i][j])
				return 0 ;
	
	return 1 ;
}

int main()
{
	struct timer tm ;
	init_timer(&tm) ;

	pthread_barrier_init(&barrier,NULL,2);

	start_timer(&tm) ;
	p_floyd_warshall(path2) ;
	stop_timer(&tm) ; 

	print_time(&tm) ;

	floyd_warshall(path1) ;
	if(verify(path1, path2))
		printf("VERIFICATION PASS\n") ;
	else
		printf("VERIFICATION FAILED\n") ;

	return 0 ;
}
