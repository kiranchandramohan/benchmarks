#include <stdlib.h>
#include <stdio.h>

#include "simpletest.h"
#include "test1.h"

void* compute_a9_thread1(void *x) 
{
	int i, j ; 
	int img_start_indx = 0 ;
	int img_end_indx = IMG_SIZE/4 ;

        struct message* mesg = (struct message *)x ;
        struct buffer** buffers = mesg->bufs ;
        int* image = (int*)(buffers[0]->bo[0]->map) ;
        int* histogram = (int*)(buffers[1]->bo[0]->map) ;

	for (i = img_start_indx ; i < img_end_indx ; i++) {
		for (j = 0; j < IMG_SIZE; ++j) {
			image[i*IMG_SIZE+j] = (i*j) % 255 ;
		}   
	}   

	/* Compute the image's histogram */
	for (i = img_start_indx ; i < img_end_indx ; i++) {
		for (j = 0; j < IMG_SIZE; ++j) {
			int pix = image[i*IMG_SIZE+j] ;
			histogram[0*HISTO_SIZE+pix] += 1;
		}   
	}   

	return NULL ;
}

void* compute_a9_thread2(void *x) 
{
	int i, j ; 
	int img_start_indx = IMG_SIZE/4 ;
	int img_end_indx = IMG_SIZE/2 ;

        struct message* mesg = (struct message *)x ;
        struct buffer** buffers = mesg->bufs ;
        int* image = (int*)(buffers[0]->bo[0]->map) ;
        int* histogram = (int*)(buffers[1]->bo[0]->map) ;

	for (i = img_start_indx ; i < img_end_indx ; i++) {
		for (j = 0; j < IMG_SIZE; ++j) {
			image[i*IMG_SIZE+j] = (i*j) % 255 ;
		}   
	}   

	/* Compute the image's histogram */
	for (i = img_start_indx ; i < img_end_indx ; i++) {
		for (j = 0; j < IMG_SIZE; ++j) {
			int pix = image[i*IMG_SIZE+j] ;
			histogram[0*HISTO_SIZE+pix] += 1;
		}   
	}   

	return NULL ;
}

void* compute_image_a9_thread1(void *x) 
{
	int i, j ;
	int img_start_indx = 0 ;
	int img_end_indx = IMG_SIZE/4 ;

        struct message* mesg = (struct message *)x ;
        struct buffer** buffers = mesg->bufs ;
        int* image = (int*)(buffers[0]->bo[0]->map) ;
        int* gray_level_mapping = (int*)(buffers[2]->bo[0]->map) ;

	/* Map the old gray levels in the original image to the new gray levels. */
	for (i = img_start_indx ; i < img_end_indx ; i++) {
		for (j = 0; j < IMG_SIZE; ++j) {
			image[i*IMG_SIZE+j] = gray_level_mapping[image[i*IMG_SIZE+j]];
		}
	}

	return NULL ;
}

void* compute_image_a9_thread2(void *x) 
{
	int i, j ;
	int img_start_indx = IMG_SIZE/4 ;
	int img_end_indx = IMG_SIZE/2 ;

        struct message* mesg = (struct message *)x ;
        struct buffer** buffers = mesg->bufs ;
        int* image = (int*)(buffers[0]->bo[0]->map) ;
        int* gray_level_mapping = (int*)(buffers[2]->bo[0]->map) ;

	/* Map the old gray levels in the original image to the new gray levels. */
	for (i = img_start_indx ; i < img_end_indx ; i++) {
		for (j = 0; j < IMG_SIZE; ++j) {
			image[i*IMG_SIZE+j] = gray_level_mapping[image[i*IMG_SIZE+j]];
		}
	}

	return NULL ;
}

void init_histo(int* histogram)
{
	int histo_start_indx = 0 ;
	int histo_end_indx = HISTO_SIZE ;
	int i ;
	for (i = histo_start_indx ; i < histo_end_indx ; i++)
		histogram[0*HISTO_SIZE+i] = 0;
}

void compute_gray_level_mapping(int* histogram, int* gray_level_mapping)
{
	int i ;
	float cdf, pixels ;
	for (i = 0; i < HISTO_SIZE/2; i++)
		histogram[0*HISTO_SIZE+i] += histogram[1*HISTO_SIZE+i] ;

	/* Compute the mapping from the old to the new gray levels */
	cdf = 0.0;
	pixels = (float) (IMG_SIZE * IMG_SIZE);
	for (i = 0; i < HISTO_SIZE; i++) {
		cdf += ((float)(histogram[0*HISTO_SIZE+i])) / pixels;
		gray_level_mapping[i] = (int)(255.0 * cdf);
	}
}

void serial_histo(int* image, int* histogram, int* gray_level_mapping)
{
	int i, j ;
	float cdf, pixels ;

	int nIter = 0 ;
	for(nIter=0 ; nIter<100 ; nIter++) {
		/* Initialize the histogram array. */

		for (i = 0 ; i < IMG_SIZE ; i++) {
			for (j = 0; j < IMG_SIZE; ++j) {
				image[i*IMG_SIZE+j] = (i*j) % 255 ;
			}   
		}   

		for (i = 0 ; i < HISTO_SIZE ; i++)
			histogram[i] = 0;

		/* Compute the image's histogram */
		for (i = 0; i < IMG_SIZE ; i++) {
			for (j = 0; j < IMG_SIZE; ++j) {
				int pix = image[i*IMG_SIZE+j] ;
				histogram[pix] += 1;
			}   
		}   

		/* Compute the mapping from the old to the new gray levels */

		cdf = 0.0;
		pixels = (float) (IMG_SIZE * IMG_SIZE);
		for (i = 0; i < HISTO_SIZE; i++) {
			cdf += ((float)(histogram[i])) / pixels;
			gray_level_mapping[i] = (int)(255.0 * cdf);
		}

		/* Map the old gray levels in the original image to the new gray levels. */

		for (i = 0 ; i < IMG_SIZE; i++) {
			for (j = 0; j < IMG_SIZE; ++j) {
				image[i*IMG_SIZE+j] = gray_level_mapping[image[i*IMG_SIZE+j]];
			}
		}

	}
}
