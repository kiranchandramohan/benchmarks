#ifndef TEST1_H
#define TEST1_H

#define NBUF 6
#define NUM_ITER 30
#define MAXGRID 64
#define LENGTH 2048


void parallel_kernel_reg_detect(int tid, struct buffer* buffers, int start_indx, int end_indx) ;
void serial_kernel_reg_detect(int* sum_tang, int* mean, int* diff, int* sum_diff, int* path) ;
void a9_compute(int tid, struct buffer* buffers, int start_indx, int end_indx) ;
void set_buffer_properties(struct buffer* bufs, int nbuf) ;
void init_buffers(struct buffer* bufs, int nbuf) ;
int verify_result(struct buffer* bufs) ;
int find_remaining_size(int size_others) ;

#endif
