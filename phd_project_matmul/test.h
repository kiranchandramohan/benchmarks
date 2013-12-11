#ifndef TEST1_H
#define TEST1_H

#define NUM_ITER 1
//#define SIZE 2048   // Size by SIZE matrices
#define SIZE 1024   // Size by SIZE matrices
#define NBUF 3

//#define NEED_BARRIER 1

void multiply(int* A, int* B, int* C, int start_indx, int end_indx) ;
int sum(int size, int beta) ;
void a9_compute(int tid, struct buffer* buffers, int start_indx, int end_indx) ;
void set_buffer_properties(struct buffer* bufs, int nbuf) ;
void init_buffers(struct buffer* bufs, int nbuf) ;
int find_remaining_size(int size_others) ;
int verify_result(struct buffer* bufs) ;

#endif
