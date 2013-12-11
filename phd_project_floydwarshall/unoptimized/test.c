#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "barrier.h"
#include "util.h"
#include "simpletest.h"
#include "test.h"

pthread_barrier_t barrier0 ;
pthread_barrier_t barrier1 ;
pthread_barrier_t barrier2 ;
extern struct rpmsg_fds remote_fd ;

inline int mod(int n)
{
	if(n<0)
		return -n ;
	else
		return n ;
}

void parallel_floyd_warshall(int tid, int* path, 
			int start_indx, int end_indx, struct buffer buf)
{
	int i, j, k ;

	printf("Computing FW from %d to %d\n", start_indx, end_indx) ;

	for(i=start_indx ; i<end_indx ; i++)
		for(j=0 ; j<N ; j++)
			path[i*N+j] = mod(i-j);

	for (k = 0; k < N; k++)
	{
		if(tid==0) {
			call_barrier(1) ;
			if(k > 0) {
				if(remote_fd.sysm3 >= 0) {
					detach_buffer(buf, remote_fd.sysm3, 0) ;
					//attach_buffer(buf, remote_fd.sysm3, 0) ;
				}	
				if(remote_fd.dsp >= 0) {
					detach_buffer(buf, remote_fd.dsp, 0) ;
					//attach_buffer(buf, remote_fd.dsp, 0) ;
				}
			}
			call_barrier(2) ;
		}
		pthread_barrier_wait(&barrier1);
		for(i = start_indx ; i < end_indx ; i++)
			for (j = 0; j < N; j++)
				path[i*N+j] = path[i*N+j] < path[i*N+k] + path[k*N+j] ? 
					path[i*N+j] : path[i*N+k] + path[k*N+j] ;
	}
}

void serial_floyd_warshall(int path[N][N])
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

void set_buffer_properties(struct buffer* bufs, int nbuf)
{
	int i ;
	for(i=0 ; i<nbuf ; i++) {
		//bo_flags |= OMAP_BO_UNCACHED ;
		//bo_flags |= OMAP_BO_WC ;
		//bo_flags |= OMAP_BO_CACHED;
		//bufs[i].cache_type = OMAP_BO_UNCACHED ;
		bufs[i].cache_type = OMAP_BO_CACHED ;
		bufs[i].privatize = false ;
		bufs[i].rd_only = false ;
		bufs[i].width = N ;
		bufs[i].height = N ;
		bufs[i].nbo = 1 ;
	}
}

void init_buffers(struct buffer* bufs, int nbuf)
{
	int i, j, k ;
	for(i=0 ; i<nbuf ; i++) {
		for(j=0 ; j<N ; j++) {
			for(k=0 ; k<N ; k++) {
				((int*)bufs[i].bo[0]->map)[j*N+k] = 0 ;
			}
		}
	}
}

int local_path[N][N] ;
int verify_result(struct buffer* buffers)
{
	int(*path)[N] =  buffers[0].bo[0]->map ;

	serial_floyd_warshall(local_path) ;

	int i, j ;
	for(i = 0; i < N; i++)
		for (j = 0; j < N; j++)
			if(path[i][j] != local_path[i][j])
				return 0 ;
	
	return 1 ;
}

void a9_compute(int tid, struct buffer* buffers, int start_indx, int end_indx)
{
	int* path =  buffers[0].bo[0]->map ;
	int i ;
	for(i=0 ; i<NUM_ITER ; i++)
	{
		if(tid == 0) {
			call_barrier(0) ;
		}
		pthread_barrier_wait(&barrier0) ;
		parallel_floyd_warshall(tid, path, start_indx, end_indx, buffers[0]) ;
	}
}

int find_remaining_size(size_others)
{
	return N - size_others ;
}
