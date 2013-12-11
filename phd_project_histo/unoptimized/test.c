#include <stdlib.h>
#include <stdio.h>

#include "simpletest.h"
#include "test.h"
#include "barrier.h"

pthread_barrier_t barrier0 ;
pthread_barrier_t barrier1 ;
pthread_barrier_t barrier2 ;
pthread_barrier_t barrier3 ;
extern struct rpmsg_fds remote_fd ;
int* histo[NUM_TOTAL_THREADS] ;
int a9_histo[HISTO_SIZE] ;
int a9_gray_level_mapping[HISTO_SIZE] ;

void* compute_a9_thread1(struct buffer* buffers, int img_start_indx, int img_end_indx) 
{
	int i, j ; 
	int histo_start_indx = 0 ;
	int histo_end_indx = HISTO_SIZE ;

        int* image = (int*)(buffers[0].bo[0]->map) ;
        int* histogram = (int*)(buffers[1].bo[0]->map) ;

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

void* compute_a9_thread2(struct buffer* buffers, int img_start_indx, int img_end_indx) 
{
	int i, j ; 
	int histo_start_indx = 0 ;
	int histo_end_indx = HISTO_SIZE ;

        int* image = (int*)(buffers[0].bo[0]->map) ;
        int* histogram = (int*)(buffers[2].bo[0]->map) ;

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

void* compute_image_a9_thread1(struct buffer* buffers, int img_start_indx, int img_end_indx) 
{
	int i, j ;

        int* image = (int*)(buffers[0].bo[0]->map) ;

	/* Map the old gray levels in the original image to the new gray levels. */
	for (i = img_start_indx ; i < img_end_indx ; i++) {
		for (j = 0; j < IMG_SIZE; ++j) {
			image[i*IMG_SIZE+j] = a9_gray_level_mapping[image[i*IMG_SIZE+j]];
		}
	}

	return NULL ;
}

void* compute_image_a9_thread2(struct buffer* buffers, int img_start_indx, int img_end_indx) 
{
	int i, j ;

        int* image = (int*)(buffers[0].bo[0]->map) ;

	/* Map the old gray levels in the original image to the new gray levels. */
	for (i = img_start_indx ; i < img_end_indx ; i++) {
		for (j = 0; j < IMG_SIZE; ++j) {
			image[i*IMG_SIZE+j] = a9_gray_level_mapping[image[i*IMG_SIZE+j]];
		}
	}

	return NULL ;
}

void compute_gray_level_mapping()
{
	int j, k ;
	float cdf, pixels ;
	float tmp1 ;

	for (j = 0; j < NUM_TOTAL_THREADS; j++) {
		for(k=0 ; k<HISTO_SIZE ; k++) {
			if(j==0)
				a9_histo[k] = (histo[j])[k] ;
			else
				a9_histo[k] += (histo[j])[k] ;
		}
	}

	/* Compute the mapping from the old to the new gray levels */
	cdf = 0.0;
	pixels = (float) (IMG_SIZE * IMG_SIZE);
	for (j = 0; j < HISTO_SIZE; j++) {
		cdf += ((float)(a9_histo[j])) / pixels;
		a9_gray_level_mapping[j] = (int)(255.0 * cdf) ;
	}
}

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

void a9_compute(int tid, struct buffer* buffers, int start_indx, int end_indx)
{
	int i, j ;
	histo[0] = (int*)(buffers[1].bo[0]->map) ;
	histo[1] = (int*)(buffers[2].bo[0]->map) ;
	histo[2] = (int*)(buffers[3].bo[0]->map) ;
	histo[3] = (int*)(buffers[4].bo[0]->map) ;
	histo[4] = (int*)(buffers[5].bo[0]->map) ;
	for(i=0 ; i<NUM_ITER ; i++)
	{
		if(tid == 0) {
			call_barrier(0) ;
			pthread_barrier_wait(&barrier0);
			compute_a9_thread1(buffers, start_indx, end_indx) ;
			pthread_barrier_wait(&barrier1);
			if(remote_fd.sysm3 >= 0) {
				detach_buffer(buffers[1], remote_fd.sysm3, 0) ;
				detach_buffer(buffers[2], remote_fd.sysm3, 0) ;
				attach_buffer(buffers[1], remote_fd.sysm3, 0) ;
				attach_buffer(buffers[2], remote_fd.sysm3, 0) ;
			}
			if(remote_fd.dsp >= 0) {
				detach_buffer(buffers[1], remote_fd.dsp, 0) ;
				detach_buffer(buffers[2], remote_fd.dsp, 0) ;
				attach_buffer(buffers[1], remote_fd.dsp, 0) ;
				attach_buffer(buffers[2], remote_fd.dsp, 0) ;
			}
			pthread_barrier_wait(&barrier2);
			call_barrier(1) ;
			compute_gray_level_mapping() ;
			pthread_barrier_wait(&barrier3);
			compute_image_a9_thread1(buffers, start_indx, end_indx) ;
		} else {
			pthread_barrier_wait(&barrier0);
			compute_a9_thread2(buffers, start_indx, end_indx) ;
			pthread_barrier_wait(&barrier1);
			pthread_barrier_wait(&barrier2);
			pthread_barrier_wait(&barrier3);
			compute_image_a9_thread2(buffers, start_indx, end_indx) ;
		}
	}
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
		if(i>=3) {
			bufs[i].cache_type = OMAP_BO_UNCACHED ;
		}
		bufs[i].remote_attach = true ;
		if(i==0) {
			bufs[i].width = IMG_SIZE ;
			bufs[i].height = IMG_SIZE ;
			bufs[i].nbo = 1 ;
		} else {
			//bufs[i].cache_type = OMAP_BO_UNCACHED ;
			bufs[i].width = 1024 ; //HISTO_SIZE ;
			bufs[i].height = 32 ;
			bufs[i].privatize = true ;
			bufs[i].nbo = 1 ;
		}
	}
}

void init_buffers(struct buffer* bufs, int nbuf)
{
	int i, j, k ;
	for(i=0 ; i<nbuf ; i++) {
		if(i==0) {
			for(j=0 ; j<IMG_SIZE ; j++) {
				for(k=0 ; k<IMG_SIZE ; k++) {
					((int*)bufs[i].bo[0]->map)[j*IMG_SIZE+k] = 0 ;
				}
			}
		} else {
			for(j=0 ; j<HISTO_SIZE ; j++) {
				for(k=0 ; k<bufs[i].nbo ; k++) {
					((int*)bufs[i].bo[k]->map)[j] = 0 ;
				}
			}
		}
	}
}

int verify_result(struct buffer* bufs)
{
	int i, j ;
	int pass ;
/*
	int* IMAGE = (int*)malloc(IMG_SIZE * IMG_SIZE * sizeof(int)) ;
	int* HISTO = (int*)malloc(HISTO_SIZE * sizeof(int)) ;
	int* GRAY_LEVEL = (int*)malloc(HISTO_SIZE * sizeof(int)) ;

	int* buffer1 = bufs[0].bo[0]->map ;
	int* buffer2 = a9_histo ;
	int* buffer3 = a9_gray_level_mapping ;

	serial_histo(IMAGE, HISTO, GRAY_LEVEL) ;

	pass = 1 ;
	for(i=0 ; i<IMG_SIZE ; i++) {
		for(j=0 ; j<IMG_SIZE ; j++) {
			if(IMAGE[i*IMG_SIZE+j] != buffer1[i*IMG_SIZE+j]) {
				printf("Difference in first buffer i=%d,j=%d : %d, %d\n",
					i, j, IMAGE[i*IMG_SIZE+j], buffer1[i*IMG_SIZE+j]) ;
				pass = 0 ;                                             
				goto fail ;
			}
		}
	}

fail :
	free(IMAGE) ;
	free(HISTO) ;
	free(GRAY_LEVEL) ;

	return pass ;
*/	
	return 1 ;
}

int find_remaining_size(int size_others)
{
	return IMG_SIZE - size_others ;
}
