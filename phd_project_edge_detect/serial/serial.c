#include <stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include "timer.h"
#define SIZE 2048
#define NUM_ITER 100
#define         K       3
#define         N       SIZE
#define         T       127

#define GAUSSIAN 1
#define VERTICAL_SOBEL 2
#define HORIZONTAL_SOBEL 3

int IB1[SIZE][SIZE], IB2[SIZE][SIZE], IB3[SIZE][SIZE] ;

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

	  set_filter(filter, GAUSSIAN) ;
	  convolve2d(image_buffer1, filter, image_buffer3, modified_start_indx, modified_end_indx);

	  set_filter(filter, VERTICAL_SOBEL) ;
	  convolve2d(image_buffer3, filter, image_buffer1, modified_start_indx, modified_end_indx);

	  set_filter(filter, HORIZONTAL_SOBEL) ;
	  convolve2d(image_buffer3, filter, image_buffer2, modified_start_indx, modified_end_indx);

	  apply_threshold(image_buffer1, image_buffer2, image_buffer3, start_indx, end_indx) ;
  }

  printf("Ending edge detection %d -> %d\n", start_indx, end_indx) ;
}

int main()
{
	struct timer tm ;
	init_timer(&tm) ;

	start_timer(&tm) ;
	edge_detect(IB1, IB2, IB3, 0, SIZE, /*synchronize=*/0, /*thread=*/0) ;
	stop_timer(&tm) ; 

	print_time(&tm) ;

	return 0 ;
}
