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
#include "timer.h"

/* Include benchmark-specific header. */
/* Default data type is int, default size is 50. */
#define MAXGRID 512
#define LENGTH 16
#define NUM_ITER 100

int sum_tang[MAXGRID][MAXGRID] ;
int mean[MAXGRID][MAXGRID] ;
int path[MAXGRID][MAXGRID] ;
int diff[MAXGRID][MAXGRID][LENGTH] ;
int sum_diff[MAXGRID][MAXGRID][LENGTH] ;

int t_sum_tang[MAXGRID][MAXGRID] ;
int t_mean[MAXGRID][MAXGRID] ;
int t_path[MAXGRID][MAXGRID] ;
int t_diff[MAXGRID][MAXGRID][LENGTH] ;
int t_sum_diff[MAXGRID][MAXGRID][LENGTH] ;

int tmp[MAXGRID][MAXGRID] ;

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

void serial_kernel_reg_detect(int* sum_tang, int* mean, int* diff, int* sum_diff, int* path)
{
	int t, i, j, cnt;

	for (i = 0; i < MAXGRID; i++)
		for (j = 0; j < MAXGRID; j++) {
			sum_tang[i*MAXGRID+j] = (i+1)*(j+1);
			mean[i*MAXGRID+j] = (i-j)/MAXGRID;
			path[i*MAXGRID+j] = (i*(j-1))/MAXGRID;
		}   

	for (j = 0; j <= MAXGRID - 1; j++)
		for (i = j; i <= MAXGRID - 1; i++)
			for (cnt = 0; cnt <= LENGTH - 1; cnt++) {
				diff[j*MAXGRID*LENGTH + i*LENGTH + cnt] =
					sum_tang[j*LENGTH + i];
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

int main()
{
	int i ;
	struct timer tm ;
	init_timer(&tm) ;

	start_timer(&tm) ;
	for(i=0 ; i<NUM_ITER ; i++)
		serial_kernel_reg_detect((int*)t_sum_tang, (int*)t_mean, (int*)t_diff, 
				(int*)t_sum_diff, (int*)t_path) ;
	stop_timer(&tm) ; 

	print_time(&tm) ;

	return 0 ;
}
