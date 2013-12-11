#ifndef TEST1_H
#define TEST1_H

#define NBUF 6
#define NUM_ITER 1
#define IMG_SIZE 4096   // size of image matrix
#define HISTO_SIZE 256
#define NEED_BARRIER 1
#define NUM_TOTAL_THREADS 5

void* compute_a9_thread1(struct buffer* buffers, int img_start_indx, int img_end_indx) ;
void* compute_a9_thread2(struct buffer* buffers, int img_start_indx, int img_end_indx) ;
void* compute_image_a9_thread1(struct buffer* buffers, int img_start_indx, int img_end_indx) ;
void* compute_image_a9_thread2(struct buffer* buffers, int img_start_indx, int img_end_indx) ;
void compute_gray_level_mapping() ;
void serial_histo(int* image, int* histogram, int* gray_level_mapping) ;
void a9_compute(int tid, struct buffer* buffers, int start_indx, int end_indx) ;
void set_buffer_properties(struct buffer* bufs, int nbuf) ;
void init_buffers(struct buffer* bufs, int nbuf) ;
int verify_result(struct buffer* bufs) ;
int find_remaining_size(int size_others) ;
#endif
