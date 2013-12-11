#ifndef TEST1_H
#define TEST1_H

#define IMG_SIZE 2048   // size of image matrix
#define HISTO_SIZE 256
#define NBUF 3

void* compute_a9_thread1(void *x) ;
void* compute_a9_thread2(void *x) ;
void* compute_image_a9_thread1(void *x) ;
void* compute_image_a9_thread2(void *x) ;
void init_histo(int* histogram) ;
void compute_gray_level_mapping(int* histogram, int* gray_level_mapping) ;
void serial_histo(int* image, int* histogram, int* gray_level_mapping) ;

#endif
