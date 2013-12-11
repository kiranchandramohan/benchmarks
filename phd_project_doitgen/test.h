#ifndef TEST1_H
#define TEST1_H

#define NUM_ITER 100
#define NBUF 5
#define NQ 64
#define NR 64
#define NP 64

void doitgen(int* A, int* sum, int* C4, int start_indx, int end_indx) ;
void serial_doitgen(int* A, int* sum, int* C4) ;
int verify_result(struct buffer* buffers) ;
void set_buffer_properties(struct buffer* bufs, int nbuf) ;
void init_buffers(struct buffer* bufs, int nbuf) ;
void a9_compute(int tid, struct buffer* buffers, int start_indx, int end_indx) ;
int find_remaining_size(int size_others) ;

#endif
