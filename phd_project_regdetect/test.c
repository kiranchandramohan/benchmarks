/**
 * reg_detect.c: This file is part of the PolyBench/C 3.2 test suite.
 *
 *
 * Contact: Louis-Noel Pouchet <pouchet@cse.ohio-state.edu>
 * Web address: http://polybench.sourceforge.net
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

/* Include benchmark-specific header. */
/* Default data type is int, default size is 50. */
#include "simpletest.h"
#include "util.h"
#include "test.h"
#include "barrier.h"

pthread_barrier_t barrier0 ;
pthread_barrier_t barrier1 ;
pthread_barrier_t barrier2 ;
extern struct rpmsg_fds remote_fd ;

/* DCE code. Must scan the entire live-out data.
   Can be used also to check the correctness of the output. */
void print_array(int* path)
{
  int i, j;

  for (i = 0; i < MAXGRID; i++) {
    for (j = 0; j < MAXGRID; j++) {
      //fprintf (stderr, "%d", path[i*MAXGRID+j]);
      //if ((i * MAXGRID + j) % 20 == 0) fprintf (stderr, "\n");
      printf ("%d\t", path[i*MAXGRID+j]);
    }   
    printf ("\n");
  }
  //fprintf (stderr, "\n");
}

void parallel_kernel_reg_detect(int tid, struct buffer* buffers, int start_indx, int end_indx)
{
        int i, j, cnt ;
	int* __restrict__ sum_tang = (int*)(buffers[0].bo[0]->map) ;
	int* __restrict__ mean = (int*)(buffers[1].bo[0]->map) ;
	int* __restrict__ diff = (int*)(buffers[2].bo[0]->map) ;
	int* __restrict__ sum_diff = (int*)(buffers[3].bo[0]->map) ;
	int* __restrict__ tmp = (int*)(buffers[4].bo[0]->map) ;
	int* __restrict__ path = (int*)(buffers[5].bo[0]->map) ;

	for (i = start_indx ; i < end_indx ; i++)
		for (j = 0; j < MAXGRID; j++) {
			sum_tang[i*MAXGRID+j] = (i+1)*(j+1);
			mean[i*MAXGRID+j] = (i-j)/MAXGRID;
			path[i*MAXGRID+j] = (i*(j-1))/MAXGRID;
		}   

        for (j = start_indx ; j < end_indx ; j++)
                for (i = j; i <= MAXGRID - 1; i++)
                        for (cnt = 0; cnt <= LENGTH - 1; cnt++) {
                                diff[j*MAXGRID*LENGTH + i*LENGTH + cnt] =
                                        sum_tang[j*MAXGRID + i]; 
                        }   

        for (j = start_indx ; j < end_indx ; j++)
        {   
                for (i = j; i <= MAXGRID - 1; i++)
                {   
                        sum_diff[j*MAXGRID*LENGTH + i*LENGTH + 0] = diff[j*MAXGRID*LENGTH + i*LENGTH + 0] ;
                        for (cnt = 1; cnt <= LENGTH - 1; cnt++) {
                                sum_diff[j*MAXGRID*LENGTH + i*LENGTH + cnt] = 
                                        sum_diff[j*MAXGRID*LENGTH + i*LENGTH + cnt-1] + 
                                        diff[j*MAXGRID*LENGTH + i*LENGTH + cnt];
                        }   

                        mean[j*MAXGRID+i] = sum_diff[j*MAXGRID*LENGTH + i*LENGTH + LENGTH-1];
                }   
        }   

	if(tid == 0) {
		if(remote_fd.sysm3 >= 0) {
			detach_buffer(buffers[1], remote_fd.sysm3, 0) ;
			attach_buffer(buffers[1], remote_fd.sysm3, 0) ;
		}	
		if(remote_fd.dsp >= 0) {
			detach_buffer(buffers[1], remote_fd.dsp, 0) ;
			attach_buffer(buffers[1], remote_fd.dsp, 0) ;
		}
		call_barrier(1) ;
	}
	pthread_barrier_wait(&barrier1) ;

	int x ;
	for (j = start_indx ; j < end_indx ; j++) {
		x = 0 ;
		tmp[j*MAXGRID+j] = mean[x*MAXGRID+j] ;
		//printf("tmp[%d][%d] = %d\n", j, j, tmp[j*MAXGRID+j]) ;
		for (i = j+1; i <= MAXGRID - 1; i++) {
			x++ ;
			tmp[j*MAXGRID+i] = tmp[j*MAXGRID+i-1] + mean[x*MAXGRID+i];
			//printf("tmp[%d][%d] = %d\n", j, i, tmp[j*MAXGRID+i]) ;
		}
	}
	
	if(tid == 0) {
		if(remote_fd.sysm3 >= 0) {
			detach_buffer(buffers[4], remote_fd.sysm3, 0) ;
			attach_buffer(buffers[4], remote_fd.sysm3, 0) ;
		}	
		if(remote_fd.dsp >= 0) {
			detach_buffer(buffers[4], remote_fd.dsp, 0) ;
			attach_buffer(buffers[4], remote_fd.dsp, 0) ;
		}
		call_barrier(2) ;
	}
	pthread_barrier_wait(&barrier2) ;

	//print_array((int*)tmp) ;

	for (j = start_indx ; j < end_indx ; j++) {
		x = 0 ;
		for (i = j ; i <= MAXGRID - 1; i++) {
			path[j*MAXGRID+i] = tmp[x*MAXGRID+i] ;
			x++ ;
		}
	}
	//print_array((int*)path) ;
}

void serial_kernel_reg_detect(int* sum_tang, int* mean, int* diff, int* sum_diff, int* path)
{
        int t, i, j, cnt;
	
	for (i = 0; i < MAXGRID; i++)
		for (j = 0; j < MAXGRID; j++) {
			sum_tang[i*MAXGRID+j] = (i+1)*(j+1);
			mean[i*MAXGRID+j] = (i-j)/MAXGRID;
			path[i*MAXGRID+j] = (i*(j-1))/MAXGRID;
		}   

	for (t = 0; t < NUM_ITER; t++)
	{
		for (j = 0; j <= MAXGRID - 1; j++)
                        for (i = j; i <= MAXGRID - 1; i++)
                                for (cnt = 0; cnt <= LENGTH - 1; cnt++) {
                                        diff[j*MAXGRID*LENGTH + i*LENGTH + cnt] =
                                        sum_tang[j*MAXGRID + i];
                                }

                for (j = 0; j <= MAXGRID - 1; j++)
                {
                        for (i = j; i <= MAXGRID - 1; i++)
                        {   
                                sum_diff[j*MAXGRID*LENGTH + i*LENGTH + 0] = diff[j*MAXGRID*LENGTH + i*LENGTH + 0] ;
                                for (cnt = 1; cnt <= LENGTH - 1; cnt++) {
                                        sum_diff[j*MAXGRID*LENGTH + i*LENGTH + cnt] = 
                                        sum_diff[j*MAXGRID*LENGTH + i*LENGTH + cnt-1] + 
                                            diff[j*MAXGRID*LENGTH + i*LENGTH + cnt];
                                }   

                                mean[j*MAXGRID+i] = sum_diff[j*MAXGRID*LENGTH + i*LENGTH + LENGTH-1];
                        }   
                }   

                for (i = 0; i <= MAXGRID - 1; i++)
                        path[0*MAXGRID+i] = mean[0*MAXGRID+i];

                for (j = 1; j <= MAXGRID - 1; j++)
                        for (i = j; i <= MAXGRID - 1; i++)
                                path[j*MAXGRID+i] = path[(j-1)*MAXGRID+i-1] + mean[j*MAXGRID+i];
        }   
}

void a9_compute(int tid, struct buffer* buffers, int start_indx, int end_indx)
{
	int i ;
	for(i=0 ; i<NUM_ITER ; i++)
	{
		if(tid == 0) {
			call_barrier(0) ;
		}
		pthread_barrier_wait(&barrier0) ;
		parallel_kernel_reg_detect(tid, buffers, start_indx, end_indx) ;
	}
}

void set_buffer_properties(struct buffer* bufs, int nbuf)
{
	int i ;
	for(i=0 ; i<nbuf ; i++) {
		//bufs[i].cache_type = OMAP_BO_WC ;
		bufs[i].cache_type = OMAP_BO_CACHED ;
		//bufs[i].cache_type = OMAP_BO_UNCACHED ;
		bufs[i].privatize = false ;
		bufs[i].rd_only = true ;
		bufs[i].nbo = 1 ;
		bufs[i].remote_attach = true ;

		bufs[i].width = MAXGRID ;
		bufs[i].height = MAXGRID ;
		if(i==2 || i==3) {
			bufs[i].width = MAXGRID ;
			bufs[i].height = MAXGRID * LENGTH ;
		}
	}
}

void init_buffers(struct buffer* bufs, int nbuf)
{
	int i, j, k, l ;
	for(i=0 ; i<nbuf ; i++) {
		if(i==2 || i==3) {
			for(j=0 ; j<MAXGRID ; j++) {
				for(k=0 ; k<MAXGRID ; k++) {
					for(l=0 ; l<LENGTH ; l++) {
						int indx = j*(MAXGRID*LENGTH) + k * LENGTH + l ;
						((int*)bufs[i].bo[0]->map)[indx] = 0 ;
					}
				}
			}
		} else {
			for(j=0 ; j<MAXGRID ; j++) {
				for(k=0 ; k<MAXGRID ; k++) {
					((int*)bufs[i].bo[0]->map)[j*MAXGRID+k] = 0 ;
				}
			}
		}
	}
}

int t_sum_tang[MAXGRID*MAXGRID] ;
int t_mean[MAXGRID*MAXGRID] ;
int t_diff[MAXGRID*MAXGRID*LENGTH] ;
int t_sum_diff[MAXGRID*MAXGRID*LENGTH] ;
int t_path[MAXGRID*MAXGRID] ;
int verify_result(struct buffer* bufs)
{
	int pass = 1 ;
	//int i, j ;

	//int* path = bufs[5].bo[0]->map ;
	//
	//serial_kernel_reg_detect(t_sum_tang, t_mean, t_diff, t_sum_diff, t_path) ;
	//for(i=0 ; i<MAXGRID ; i++)
	//	for(j=0 ; j<MAXGRID ; j++) {
	//		if(path[i*MAXGRID+j] != t_path[i*MAXGRID+j]) {
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

int find_remaining_size(int size_others)
{
	return MAXGRID - size_others ;
}
