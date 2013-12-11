#include <stdio.h>
#define IMG_SIZE 1024
#define HISTO_SIZE 256

void* compute_a9_thread1(int* __restrict__ image, int* __restrict__ histogram,
			int img_start_indx, int img_end_indx) 
{
	int i, j ; 

	/* Compute the image's histogram */
	for (i = img_start_indx ; i < img_end_indx ; i++) {
		for (j = 0; j < IMG_SIZE; ++j) {
			int pix = image[i*IMG_SIZE+j] ;
			histogram[0*HISTO_SIZE+pix] += 1;
		}   
	}   

	return NULL ;
}

