#include "util.h"
#include "simpletest.h"
#include "test.h"
#include "timer.h"
#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include "barrier.h"

extern int num_threads_barrier ;
extern pthread_barrier_t barrier0 ;
extern pthread_barrier_t barrier1 ;
extern pthread_barrier_t barrier2 ;
extern struct rpmsg_fds remote_fd ;

extern struct timer tm_a9_0 ;
extern struct timer tm_a9_1 ;

int abs(int n)
{
	return (n<0) ? (-n) : n ;
}

int get_start_indx(int n)
{
        if(n==0)
                return n ; 
        else
                return n-1 ;
}

int get_end_indx(int n)
{
        if(n==SIZE)
                return n ; 
        else
                return n+1 ;
}

void edge_detect(int image_buffer1[N][N], int image_buffer2[N][N], int image_buffer3[N][N],
		int start_indx, int end_indx, int synch, int tid)
{
  int filter[K][K];

  int modified_start_indx, modified_end_indx ;
  modified_start_indx = get_start_indx(start_indx) ;
  modified_end_indx = get_end_indx(end_indx) ;

  printf("Starting edge detection %d -> %d\n", start_indx, end_indx) ;

  int i ;
  for(i=0 ; i<NUM_ITER ; i++) 
  {
	  initialize(image_buffer1, image_buffer2, image_buffer3, modified_start_indx, modified_end_indx) ;
	  if(synch) {
		  pthread_barrier_wait(&barrier0);
		  if(tid == 0)
			  call_barrier(0) ;
	  }

	  set_filter(filter, GAUSSIAN) ;
	  convolve2d(image_buffer1, filter, image_buffer3, modified_start_indx, modified_end_indx);
	  if(synch) {
		  pthread_barrier_wait(&barrier1);
		  if(tid == 0)
			  call_barrier(1) ;
	  }

	  set_filter(filter, VERTICAL_SOBEL) ;
	  convolve2d(image_buffer3, filter, image_buffer1, modified_start_indx, modified_end_indx);

	  set_filter(filter, HORIZONTAL_SOBEL) ;
	  convolve2d(image_buffer3, filter, image_buffer2, modified_start_indx, modified_end_indx);
	  if(synch) {
		  pthread_barrier_wait(&barrier2);
		  if(tid == 0)
			  call_barrier(2) ;
	  }

	  apply_threshold(image_buffer1, image_buffer2, image_buffer3, start_indx, end_indx) ;
  }

  printf("Ending edge detection %d -> %d\n", start_indx, end_indx) ;
}

//void edge_detect(int image_buffer1[N][N], int image_buffer2[N][N], int image_buffer3[N][N],
//		int start_indx, int end_indx, int synch, int tid)
//{
//  int filter[K][K];
//
//  int modified_start_indx, modified_end_indx ;
//  modified_start_indx = get_start_indx(start_indx) ;
//  modified_end_indx = get_end_indx(end_indx) ;
//
//  printf("Starting edge detection %d -> %d\n", start_indx, end_indx) ;
//
//  //if(synch)
//  //	  start_timer((tid == 0) ? &tm_a9_0 : &tm_a9_1) ;
//  initialize(image_buffer1, image_buffer2, image_buffer3, start_indx, end_indx) ;
//  //if(synch)
//  //	  stop_timer((tid == 0) ? &tm_a9_0 : &tm_a9_1) ;
//  #if SYNC
//  // Synchronization point
//  if(synch)
//	  call_barrier(0) ;
//  #endif
//
//  //if(synch)
//  //	  start_timer((tid == 0) ? &tm_a9_0 : &tm_a9_1) ;
//  /* Set the values of the filter matrix to a Gaussian kernel.
//     This is used as a low-pass filter which blurs the image so as to
//     de-emphasize the response of some isolated points to the edge
//     detection (Sobel) kernels. */
//  set_filter(filter, GAUSSIAN) ;
//
//  /* Perform the Gaussian convolution. */
//  convolve2d(image_buffer1, filter, image_buffer3, modified_start_indx, modified_end_indx);
//  //if(synch)
//  //	  stop_timer((tid == 0) ? &tm_a9_0 : &tm_a9_1) ;
//  #if SYNC
//  // Synchronization point
//  if(synch)
//	  call_barrier(1) ;
//  #endif
//
//  //if(synch)
//  //	  start_timer((tid == 0) ? &tm_a9_0 : &tm_a9_1) ;
//  /* Set the values of the filter matrix to the vertical Sobel operator. */
//  set_filter(filter, VERTICAL_SOBEL) ;
//
//  /* Convolve the smoothed matrix with the vertical Sobel kernel. */
//  convolve2d(image_buffer3, filter, image_buffer1, modified_start_indx, modified_end_indx);
//  //if(synch)
//  // 	  stop_timer((tid == 0) ? &tm_a9_0 : &tm_a9_1) ;
//
//  //if(synch)
//  //	  start_timer((tid == 0) ? &tm_a9_0 : &tm_a9_1) ;
//  /* Set the values of the filter matrix to the horizontal Sobel operator. */
//  set_filter(filter, HORIZONTAL_SOBEL) ;
//
//  /* Convolve the smoothed matrix with the horizontal Sobel kernel. */
//  convolve2d(image_buffer3, filter, image_buffer2, modified_start_indx, modified_end_indx);
//  //if(synch)
//  //	  stop_timer((tid == 0) ? &tm_a9_0 : &tm_a9_1) ;
//  #if SYNC
//  // Synchronization point
//  if(synch)
//	  call_barrier(2) ;
//  #endif 
//
//  //if(synch)
//  //	  start_timer((tid == 0) ? &tm_a9_0 : &tm_a9_1) ;
//  /* Take the larger of the magnitudes of the horizontal and vertical
//     matrices. Form a binary image by comparing to a threshold and
//     storing one of two values. */
//  apply_threshold(image_buffer1, image_buffer2, image_buffer3, start_indx, end_indx) ;
//  //if(synch)
//  //	  stop_timer((tid == 0) ? &tm_a9_0 : &tm_a9_1) ;
//
//  printf("Ending edge detection %d -> %d\n", start_indx, end_indx) ;
//}

void verify_edge_detect(int image_buffer1[N][N], int image_buffer2[N][N], int image_buffer3[N][N],
		int start_indx, int end_indx)
{
  int filter[K][K];

  printf("Starting edge detection %d -> %d\n", start_indx, end_indx) ;

  initialize(image_buffer1, image_buffer2, image_buffer3, start_indx, end_indx) ;

  /* Set the values of the filter matrix to a Gaussian kernel.
     This is used as a low-pass filter which blurs the image so as to
     de-emphasize the response of some isolated points to the edge
     detection (Sobel) kernels. */
  set_filter(filter, GAUSSIAN) ;
  /* Perform the Gaussian convolution. */
  convolve2d(image_buffer1, filter, image_buffer3, start_indx, end_indx);

  /* Set the values of the filter matrix to the vertical Sobel operator. */
  set_filter(filter, VERTICAL_SOBEL) ;
  /* Convolve the smoothed matrix with the vertical Sobel kernel. */
  convolve2d(image_buffer3, filter, image_buffer1, start_indx, end_indx);

  /* Set the values of the filter matrix to the horizontal Sobel operator. */
  set_filter(filter, HORIZONTAL_SOBEL) ;
  /* Convolve the smoothed matrix with the horizontal Sobel kernel. */
  convolve2d(image_buffer3, filter, image_buffer2, start_indx, end_indx);

  /* Take the larger of the magnitudes of the horizontal and vertical
     matrices. Form a binary image by comparing to a threshold and
     storing one of two values. */
  apply_threshold(image_buffer1, image_buffer2, image_buffer3, start_indx, end_indx) ;

  printf("Ending edge detection %d -> %d\n", start_indx, end_indx) ;
}

void initialize(int image_buffer1[N][N], int image_buffer2[N][N], int image_buffer3[N][N], 
			int start_indx, int end_indx)
{
  int i, j ;

  printf("Beginning initialization\n") ;

  /* Read input image. */
  //input_dsp(image_buffer1, N*N, 1);

  /* Initialize image_buffer2 and image_buffer3 */
  for (i = start_indx; i < end_indx; i++) {
    for (j = 0; j < N; ++j) {
       image_buffer1[i][j] = i+j ;

       image_buffer2[i][j] = 0;

       image_buffer3[i][j] = 0;
     }
  }
  printf("Finishing initialization\n") ;
}

/* This function convolves the input image by the kernel and stores the result
   in the output image. */
void convolve2d(int input_image[N][N], int kernel[K][K], int output_image[N][N],
		int start_indx, int end_indx)
{
  int i;
  int j;
  int c;
  int r;
  int normal_factor;
  int sum;
  int dead_rows;
  int dead_cols;

  /* Set the number of dead rows and columns. These represent the band of rows
     and columns around the edge of the image whose pixels must be formed from
     less than a full kernel-sized compliment of input image pixels. No output
     values for these dead rows and columns since  they would tend to have less
     than full amplitude values and would exhibit a "washed-out" look known as
     convolution edge effects. */

  dead_rows = K / 2;
  dead_cols = K / 2;

  /* Calculate the normalization factor of the kernel matrix. */

  normal_factor = 0;
  for (r = 0; r < K; r++) {
    for (c = 0; c < K; c++) {
      normal_factor += abs(kernel[r][c]);
    }
  }

  if (normal_factor == 0)
    normal_factor = 1;

  /* Convolve the input image with the kernel. */
  for (r = start_indx; r < end_indx - K + 1; r++) {
    for (c = 0; c < N - K + 1; c++) {
      sum = 0;
      for (i = 0; i < K; i++) {
        for (j = 0; j < K; j++) {
          sum += input_image[r+i][c+j] * kernel[i][j];
        }
      }
      output_image[r+dead_rows][c+dead_cols] = (sum / normal_factor);
    }
  }
}

void set_filter(int filter[K][K], int type)
{
	if(type == GAUSSIAN) {
		filter[0][0] = 1;
		filter[0][1] = 2;
		filter[0][2] = 1;
		filter[1][0] = 2;
		filter[1][1] = 4;
		filter[1][2] = 2;
		filter[2][0] = 1;
		filter[2][1] = 2;
		filter[2][2] = 1;
	} else if(type == VERTICAL_SOBEL) {
		filter[0][0] =  1;
		filter[0][1] =  0;
		filter[0][2] = -1;
		filter[1][0] =  2;
		filter[1][1] =  0;
		filter[1][2] = -2;
		filter[2][0] =  1;
		filter[2][1] =  0;
		filter[2][2] = -1;
	} else if(type == HORIZONTAL_SOBEL) {
		filter[0][0] =  1;
		filter[0][1] =  2;
		filter[0][2] =  1;
		filter[1][0] =  0;
		filter[1][1] =  0;
		filter[1][2] =  0;
		filter[2][0] = -1;
		filter[2][1] = -2;
		filter[2][2] = -1;
	}
}


void apply_threshold(int input_image1[N][N], int input_image2[N][N], int output_image[N][N],
	int start_indx, int end_indx)
{
  /* Take the larger of the magnitudes of the horizontal and vertical
     matrices. Form a binary image by comparing to a threshold and
     storing one of two values. */
  int i, j ;
  int temp1, temp2, temp3 ;

  for (i = start_indx; i < end_indx; i++) {
    for (j = 0; j < N; ++j) {
       temp1 = abs(input_image1[i][j]);
       temp2 = abs(input_image2[i][j]);
       temp3 = (temp1 > temp2) ? temp1 : temp2;
       output_image[i][j] = (temp3 > T) ? 255 : 0;
     }
  }
}

void a9_compute(int tid, struct buffer* buffers, int start_indx, int end_indx)
{
	if(tid == 0) {
		edge_detect(buffers[0].bo[0]->map, buffers[1].bo[0]->map, buffers[2].bo[0]->map, 
				start_indx, end_indx, /*synchronize=*/1, /*thread=*/0) ;
	} else {
		edge_detect(buffers[0].bo[0]->map, buffers[1].bo[0]->map, buffers[2].bo[0]->map,
				start_indx, end_indx, /*synchronize=*/1, /*thread=*/1) ;
	}
}

int IB1[SIZE][SIZE], IB2[SIZE][SIZE], IB3[SIZE][SIZE] ;
int verify_result(struct buffer* buffers)
{
	int i, j ;
	int pass ;

	verify_edge_detect(IB1, IB2, IB3, 0, N) ;
	
	int (*image_buffer1)[SIZE] = buffers[0].bo[0]->map ;
	int (*image_buffer2)[SIZE] = buffers[1].bo[0]->map ;
	int (*image_buffer3)[SIZE] = buffers[2].bo[0]->map ;

	pass = 1 ;
	for(i=0 ; i<SIZE ; i++) {
		for(j=0 ; j<SIZE ; j++) {
			if(IB1[i][j] != image_buffer1[i][j]) {
				//printf("Difference in first buffer i=%d,j=%d : %d, %d\n",i,j,IB1[i][j],image_buffer1[i][j]) ;
				pass = 0 ;                                             
			}
		}
	}

	for(i=0 ; i<SIZE ; i++) {
		for(j=0 ; j<SIZE ; j++) {
			if(IB2[i][j] != image_buffer2[i][j]) {                         
				//printf("Difference in second buffer i=%d,j=%d : %d, %d\n",i,j,IB2[i][j],image_buffer2[i][j]) ;
				pass = 0 ;                                             
			}                                                              
		}
	}

	for(i=0 ; i<SIZE ; i++) {
		for(j=0 ; j<SIZE ; j++) {
			if(IB3[i][j] != image_buffer3[i][j]) {
				//printf("Difference in third buffer i=%d,j=%d : %d, %d\n",i,j,IB3[i][j],image_buffer3[i][j]) ;
				pass = 0 ;
			}
		}
	}

	return pass ;
}

void set_buffer_properties(struct buffer* bufs, int nbuf)
{
	int i ;
	for(i=0 ; i<nbuf ; i++) {
		//bo_flags |= OMAP_BO_UNCACHED ;
		//bo_flags |= OMAP_BO_WC ;
		//bo_flags |= OMAP_BO_CACHED;
		bufs[i].cache_type = OMAP_BO_CACHED ;
		bufs[i].privatize = false ;
		bufs[i].rd_only = false ;
		bufs[i].width = SIZE ;
		bufs[i].height = SIZE ;
		if(i==0) {
			bufs[i].nbo = 1 ;
		} else {
			bufs[i].nbo = 2 ;
			if(i==1) {
				bufs[i].privatize = true ;
				if(remote_fd.sysm3 >= 0)
					bufs[i].nbo ++ ;
				if(remote_fd.dsp >= 0)
					bufs[i].nbo = 4 ;
			}
		}
	}
}

void init_buffers(struct buffer* bufs, int nbuf)
{
	int i, j, k ;
	for(i=0 ; i<nbuf ; i++) {
		if(i==0) {
			for(j=0 ; j<SIZE ; j++) {
				for(k=0 ; k<SIZE ; k++) {
					((int*)bufs[i].bo[0]->map)[j*SIZE+k] = 0 ;
				}
			}
		} else {
			for(j=0 ; j<SIZE ; j++) {
				for(k=0 ; k<bufs[i].nbo ; k++) {
					((int*)bufs[i].bo[k]->map)[j] = 0 ;
				}
			}
		}
	}
}

int find_remaining_size(int size_others)
{
	return SIZE - size_others ;
}
