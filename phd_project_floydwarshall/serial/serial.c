#include<stdio.h>
#include<pthread.h>
#include "timer.h"

#define NUM_ITER 1
#define N 2048
int path1[N][N] ;

inline int mod(int n)
{
	if(n<0)
		return -n ;
	else
		return n ;
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

int main()
{
	struct timer tm ;
	init_timer(&tm) ;

	start_timer(&tm) ;
	int i ;
	for(i=0 ; i<NUM_ITER ; i++)
		floyd_warshall(path1) ;
	stop_timer(&tm) ; 

	print_time(&tm) ;


	return 0 ;
}
