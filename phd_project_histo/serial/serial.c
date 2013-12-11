#include<stdio.h>
#include "timer.h"
#define IMG_SIZE 1024
#define HISTO_SIZE 256
#define NUM_ITER 30

int image[IMG_SIZE*IMG_SIZE] ;
int histogram[HISTO_SIZE] ;
int gray_level_mapping[HISTO_SIZE] ;

void serial_histo(int* image, int* histogram, int* gray_level_mapping)
{
        int i, j ; 
        float cdf, pixels ;

        int nIter = 0 ; 
        for(nIter=0 ; nIter<NUM_ITER ; nIter++) {
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

	start_timer(&tm) ;
	serial_histo(image, histogram, gray_level_mapping) ;
	stop_timer(&tm) ; 
	print_time(&tm) ;

	return 0 ;
}
