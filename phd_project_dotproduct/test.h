#ifndef TEST1_H
#define TEST1_H

#define NBUF 3
#define NUM_ITER 100
#define SIZE 2048   // Size by SIZE matrices

void dot_product(int* A, int* B, int* result, int start, int end) ;
int serial_dot_product(int* A, int* B) ;
void a9_compute(int tid, struct buffer* buffers, int start_indx, int end_indx) ;
void set_buffer_properties(struct buffer* bufs, int nbuf) ;
void init_buffers(struct buffer* bufs, int nbuf) ;
void a9_compute(int tid, struct buffer* buffers, int start_indx, int end_indx) ;
int verify_result(struct buffer* bufs) ;
int find_remaining_size(int size_others) ;

#endif
