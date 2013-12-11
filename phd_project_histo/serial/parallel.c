#include<stdio.h>
#include<pthread.h>
#include "timer.h"
#define IMG_SIZE 1024
#define HISTO_SIZE 256
#define NUM_ITER 30

int image[IMG_SIZE*IMG_SIZE] ;
int histo1[HISTO_SIZE] ;
int histo2[HISTO_SIZE] ;
int gray_level_mapping[HISTO_SIZE] ;
int image_s[IMG_SIZE*IMG_SIZE] ;
int histogram_s[HISTO_SIZE] ;
int gray_level_mapping_s[HISTO_SIZE] ;

pthread_barrier_t barrier1 ;
pthread_barrier_t barrier2 ;

void* compute_a9_thread1(int* image, int* histogram, int img_start_indx, int img_end_indx) 
{
	int i, j ; 
	int histo_start_indx = 0 ;
	int histo_end_indx = HISTO_SIZE ;

	for (i = histo_start_indx ; i < histo_end_indx ; i++) {
		histogram[0*HISTO_SIZE+i] = 0;
	}

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

void* compute_image_a9_thread1(int* image, int* gray_level_mapping, int img_start_indx, int img_end_indx) 
{
	int i, j ;

	/* Map the old gray levels in the original image to the new gray levels. */
	for (i = img_start_indx ; i < img_end_indx ; i++) {
		for (j = 0; j < IMG_SIZE; ++j) {
			image[i*IMG_SIZE+j] = gray_level_mapping[image[i*IMG_SIZE+j]];
		}
	}

	return NULL ;
}

void compute_gray_level_mapping(int* histo1, int* histo2, int* gray_level_mapping)
{
	int i, j ;
	float cdf, pixels ;

	for (i = 0; i < HISTO_SIZE; i++) {
		histo1[i] += histo2[i] ;
	}

	/* Compute the mapping from the old to the new gray levels */
	cdf = 0.0;
	pixels = (float) (IMG_SIZE * IMG_SIZE);
	for (i = 0; i < HISTO_SIZE; i++) {
		cdf += ((float)(histo1[0*HISTO_SIZE+i])) / pixels;
		gray_level_mapping[i] = (int)(255.0 * cdf);
	}
}

void* compute(void* tid)
{
	int i ;
	for(i=0 ; i<NUM_ITER ; i++) {
		pthread_barrier_init(&barrier1,NULL,2);
		pthread_barrier_init(&barrier2,NULL,2);
		if(tid == 0) {
			compute_a9_thread1(image, histo1, 0, IMG_SIZE/2) ;
			pthread_barrier_wait(&barrier1);
			compute_gray_level_mapping(histo1, histo2, gray_level_mapping) ;
			pthread_barrier_wait(&barrier2);
			compute_image_a9_thread1(image, gray_level_mapping, 0, IMG_SIZE/2) ;
		} else {
			compute_a9_thread1(image, histo2, IMG_SIZE/2, IMG_SIZE) ;
			pthread_barrier_wait(&barrier1);
			pthread_barrier_wait(&barrier2);
			compute_image_a9_thread1(image, gray_level_mapping, IMG_SIZE/2, IMG_SIZE) ;
		}
	}
}

void serial_histo(int* image, int* histogram, int* gray_level_mapping)
{
        int i, j ; 
        float cdf, pixels ;

        int nIter = 0 ; 
        for(nIter=0 ; nIter<1 ; nIter++) {
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

int main()
{
	struct timer tm ;
	init_timer(&tm) ;
	pthread_t thread1, thread2;
	pthread_barrier_init(&barrier1,NULL,2);
	pthread_barrier_init(&barrier2,NULL,2);
	start_timer(&tm) ;
	int ret1 = pthread_create( &thread1, NULL, compute, (void*) 0);
	int ret2 = pthread_create( &thread2, NULL, compute, (void*) 1);
	pthread_join(thread1, NULL) ;
	pthread_join(thread2, NULL) ;
	stop_timer(&tm) ; 

	print_time(&tm) ;

	//serial_histo(image_s, histogram_s, gray_level_mapping_s) ;
	//int i, j ;
	//int pass = 1 ;
	//for(i=0 ; i<IMG_SIZE ; i++) {
	//	for(j=0 ; j<IMG_SIZE ; j++) {
	//		if(image_s[i*IMG_SIZE+j] != image[i*IMG_SIZE+j])
	//			pass = 0 ;
	//	}
	//}

	//if(pass)
	//	printf("Verification : SUCCESS\n") ;
	//else
	//	printf("Verification : FAILURE\n") ;

	return 0 ;
}
