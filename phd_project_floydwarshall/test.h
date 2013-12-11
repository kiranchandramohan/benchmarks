#ifndef TEST1_H
#define TEST1_H

#define NBUF 1
#define NUM_ITER 10
#define NEED_BARRIER 1
#define N 512

void serial_floyd_warshall(int path[N][N]) ;
void set_buffer_properties(struct buffer* bufs, int nbuf) ;
void init_buffers(struct buffer* bufs, int nbuf) ;
void a9_compute(int tid, struct buffer* buffers, int start_indx, int end_indx) ;
void parallel_floyd_warshall(int tid, int* path, 
			int start_indx, int end_indx, struct buffer buf) ;
int verify_result(struct buffer* buffers) ;
int find_remaining_size(int size_others) ;
#endif
