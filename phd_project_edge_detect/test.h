#ifndef TEST1_H
#define TEST1_H

#define NUM_ITER 10
#define NBUF 3
#define CNT  500
#define SIZE 2048
#define         K       3
#define         N       SIZE
#define         T       127

#define GAUSSIAN 1
#define VERTICAL_SOBEL 2
#define HORIZONTAL_SOBEL 3

#define SYNC 1

void convolve2d(int* __restrict__ input_image, int* __restrict__ kernel, int* __restrict__ output_image, int start_indx, int end_indx);
void set_filter(int* __restrict__ filter, int type) ;
void apply_threshold(int* __restrict__ image_buffer1, int* __restrict__ image_buffer2, int* __restrict__ image_buffer3, int start_indx, int end_indx) ;
void edge_detect(int* __restrict__ image_buffer1, int* __restrict__ image_buffer2, int* __restrict__ image_buffer3, int start_indx, int end_indx, int synch, int tid) ;
void verify_edge_detect(int* __restrict__ image_buffer1, int* __restrict__ image_buffer2, int* __restrict__ image_buffer3, int start_indx, int end_indx) ;
void initialize(int* __restrict__ image_buffer1, int* __restrict__ image_buffer2, int* __restrict__ image_buffer3, int start_indx, int end_indx) ;
int verify_result(struct buffer* bufs) ;
void set_buffer_properties(struct buffer* bufs, int nbuf) ;
void init_buffers(struct buffer* bufs, int nbuf) ;
int find_remaining_size(int size_others) ;
void a9_compute(int tid, struct buffer* buffers, int start_indx, int end_indx) ;

#endif
