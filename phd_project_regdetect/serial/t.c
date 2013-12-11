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
#define MAXGRID 512
#define LENGTH 16
#define NUM_ITER 1

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
pthread_barrier_t barrier1 ;
pthread_barrier_t barrier2 ;

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

void parallel_kernel_reg_detect(int* sum_tang, int* mean, int* diff, int* sum_diff, int* path, int start_indx, int end_indx)
{
        int i, j, cnt ;
	
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
                                        sum_tang[j*LENGTH + i]; 
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

	for (i = 0; i <= MAXGRID - 1; i++)
		path[0*MAXGRID+i] = mean[0*MAXGRID+i];

	for (j = 1; j <= MAXGRID - 1; j++)
		for (i = j; i <= MAXGRID - 1; i++)
			path[j*MAXGRID+i] = path[(j-1)*MAXGRID+i-1] + mean[j*MAXGRID+i];
}

void mod_parallel_kernel_reg_detect(int* sum_tang, int* mean, int* diff, int* sum_diff, int* path, int start_indx, int end_indx)
{
        int i, j, cnt ;

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
                                        sum_tang[j*LENGTH + i]; 
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

	pthread_barrier_wait(&barrier1) ;

	int x ;
	for (j = start_indx ; j < end_indx ; j++) {
		x = 0 ;
		tmp[j][j] = mean[x*MAXGRID+j] ;
		//printf("tmp[%d][%d] = %d\n", j, j, tmp[j][j]) ;
		for (i = j+1; i <= MAXGRID - 1; i++) {
			x++ ;
			tmp[j][i] = tmp[j][i-1] + mean[x*MAXGRID+i];
			//printf("tmp[%d][%d] = %d\n", j, i, tmp[j][i]) ;
		}
	}
	
	pthread_barrier_wait(&barrier2) ;

	//print_array((int*)tmp) ;

	for (j = start_indx ; j < end_indx ; j++) {
		x = 0 ;
		for (i = j ; i <= MAXGRID - 1; i++) {
			path[j*MAXGRID+i] = tmp[x][i] ;
			x++ ;
		}
	}
	//print_array((int*)path) ;
}

void* par_wrapper(void *x)
{
	if(x == 0) {
		mod_parallel_kernel_reg_detect((int*)sum_tang, (int*)mean, (int*)diff, 
				(int*)sum_diff, (int*)path, 0, MAXGRID/2) ;
	} else {
		mod_parallel_kernel_reg_detect((int*)sum_tang, (int*)mean, (int*)diff, 
				(int*)sum_diff, (int*)path, MAXGRID/2, MAXGRID) ;
	}
	return NULL ;
}

void wrapper_parallel_kernel_reg_detect()
{
	pthread_t thread1, thread2 ;
	int ret1, ret2 ;
	ret1 = pthread_create( &thread1, NULL, par_wrapper, (void *)0) ;
	ret2 = pthread_create( &thread2, NULL, par_wrapper, (void *)1) ;
	pthread_join(thread1, NULL) ;
	pthread_join(thread2, NULL) ;
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
}

int main()
{
      pthread_barrier_init(&barrier1,NULL,2);
      pthread_barrier_init(&barrier2,NULL,2);

      //mod_parallel_kernel_reg_detect((int*)sum_tang, (int*)mean, (int*)diff, 
      //					(int*)sum_diff, (int*)path, 0, 64) ;
      wrapper_parallel_kernel_reg_detect() ;
      
      serial_kernel_reg_detect((int*)t_sum_tang, (int*)t_mean, (int*)t_diff, 
      				(int*)t_sum_diff, (int*)t_path) ;

      int pass = 1 ;
      int i, j ;
      for(i=0 ; i<MAXGRID ; i++)
	      for(j=0 ; j<MAXGRID ; j++) {
		      if(path[i][j] != t_path[i][j]) {
			      printf("Difference in i=%d,j=%d\n", i, j) ;
			      pass = 0 ;
			      break ;
		      }

	      }
	
      if(pass) {
	      printf("SUCCESS\n") ;
      } else {
	      printf("FAIL\n") ;
	      //print_array(mean) ;
	      //printf("-------------------------\n") ;
	      //print_array(t_mean) ;
	      //printf("-------------------------\n") ;
	      //print_array(path) ;
	      //printf("-------------------------\n") ;
	      //print_array(t_path) ;
      }
      
      return 0 ;
}
