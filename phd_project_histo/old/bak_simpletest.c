#include "timer.h"
#include "simpletest.h"
#include "barrier.h"
#include "test1.h"

int IB1[SIZE][SIZE], IB2[SIZE][SIZE], IB3[SIZE][SIZE] ;
int M3_FD, DSP_FD ;

struct timer tm_main ;
struct timer tm_m3 ;
struct timer tm_dsp ;
struct timer tm_a9 ;
struct timer tm_a9_0 ;
struct timer tm_a9_1 ;

int num_threads_barrier ;

static void
usage(char *name)
{
	MSG("Usage: %s [OPTION]...", name);
	MSG("Simple page-flip test, similar to 'modetest' but with option to use tiled buffers.");
	MSG("");
	disp_usage();
}


/* get buffer info */
int local_get_buffer_info(struct omap_bo *bo)
{
        struct drm_omap_gem_info req = {
                        .handle = bo->handle,
        };
        int ret = drmCommandWriteRead(bo->dev->fd, DRM_OMAP_GEM_INFO,
                        &req, sizeof(req));
        if (ret) {
                return ret;
        }

        /* really all we need for now is mmap offset */
        bo->offset = req.offset;
        bo->size = req.size;

        return 0;
}

void * local_omap_bo_map(struct omap_bo *bo)
{
        if (!bo->map) {
                if (!bo->offset) {
                        local_get_buffer_info(bo);
                }

                bo->map = mmap(0, bo->size, PROT_READ | PROT_WRITE,
                                MAP_SHARED, bo->dev->fd, bo->offset);
                if (bo->map == MAP_FAILED) {
                        printf("Mapping failed\n") ;
                        bo->map = NULL;
                } else {
                        printf("bo size = %d\n", bo->size) ;
                }
        }
        return bo->map;
}

int get_buffer_address(struct buffer* buf, int rpmsg_omx_fd, int cdev_fd)
{
        struct fds fd ;
        if(buf->bo[0]->fd == 0) {
                omap_bo_dmabuf(buf->bo[0]) ;
        }
        fd.dma_fd = buf->bo[0]->fd ;
        fd.rpmsg_omx_fd = rpmsg_omx_fd ;

        if(ioctl(cdev_fd, ATTACH_IOCTL, (void*)&fd) < 0)
                perror("ATTACH IOCTL");
        if(ioctl(cdev_fd, READ_IOCTL, (void*)&(buf->bo[0]->phy_addr)) < 0)
                perror("READ IOCTL");

        return 0 ;
}

void print_buffer_info(struct buffer* buf)
{
        printf("Height = %u, Width = %u, fourcc = %u, nbo = %d\n",
                buf->height, buf->width, buf->fourcc, buf->nbo) ;
        printf("map = %p, size = %u\n", (unsigned int*)(buf->bo[0]->map), buf->bo[0]->size) ;
}

int detach_buffers(struct buffer** bufs, int rpmsg_fd, int cdev_fd)
{
        struct fds fd ;
        struct buffer* buf ;
        fd.rpmsg_omx_fd = rpmsg_fd ;

        buf = bufs[0] ;
        fd.dma_fd = buf->bo[0]->fd ;
        if(ioctl(cdev_fd, DETACH_IOCTL, (void*)&fd) < 0)
                perror("DETACH IOCTL");

        buf = bufs[1] ;
        fd.dma_fd = buf->bo[0]->fd ;
        if(ioctl(cdev_fd, DETACH_IOCTL, (void*)&fd) < 0)
                perror("DETACH IOCTL");

        buf = bufs[2] ;
        fd.dma_fd = buf->bo[0]->fd ;
        if(ioctl(cdev_fd, DETACH_IOCTL, (void*)&fd) < 0)
                perror("DETACH IOCTL");

        return 0 ;
}

void deallocate_buffers(struct buffer** buf)
{
        printf("Begin deallocate_buffers\n") ;
        omap_bo_del(buf[0]->bo[0]) ;
        close(buf[0]->bo[0]->fd) ;
        omap_bo_del(buf[1]->bo[0]) ;
        close(buf[1]->bo[0]->fd) ;
        omap_bo_del(buf[2]->bo[0]) ;
        close(buf[2]->bo[0]->fd) ;
        printf("Finish deallocate_buffers\n") ;
}

int open_display_and_allocate_buffers(struct buffer*** buffers,
                                        int argc, char* argv[])
{

        struct display *disp;
        //struct buffer *framebuf;
        uint32_t fourcc, width, height;

        MSG("Opening Display..");
        disp = disp_open(argc, argv);
        if (!disp) {
                printf("simpletest : Opening display failed\n");
                usage(argv[0]);
                return -1; 
        } else {
                printf("simpletest : Opening display succeeded\n");
	}

	//framebuf = disp_get_fb(disp);
	fourcc = 0 ;
	width = SIZE ;
	height = SIZE ;
	*buffers = disp_get_vid_buffers(disp, NBUF, fourcc, width, height);
	if (!*buffers) {
		return -1;
	}

	local_omap_bo_map((*buffers)[0]->bo[0]) ;
	local_omap_bo_map((*buffers)[1]->bo[0]) ;
	local_omap_bo_map((*buffers)[2]->bo[0]) ;

	//print_buffer_info((*buffers)[0]) ;
	//print_buffer_info((*buffers)[1]) ;
	//print_buffer_info((*buffers)[2]) ;

	return 0 ;
}

int exec_cmd(int fd, char *msg, int len, char *reply_msg, int *reply_len)
{
    int ret = 0;
    printf("Executing remote command via rpmsg\n") ;

    ret = write(fd, msg, len);
    if (ret < 0) {
         perror("Can't write to OMX instance");
         return -1;
    }

    /* Now, await normal function result from OMX service: */
    // Note: len should be max length of response expected.
    ret = read(fd, reply_msg, len);
    if (ret < 0) {
         perror("Can't read from OMX instance");
         return -1;
    }
    else {
          *reply_len = ret;
    }
    return(0);
}

void init_omx_packet(omx_packet *packet, uint16_t desc)
{
    /* initialize the packet structure */
    packet->desc  |= desc << OMX_DESC_TYPE_SHIFT;
    packet->msg_id  = 0;
    packet->flags  = OMX_POOLID_JOBID_NONE;
    packet->fxn_idx = OMX_INVALIDFXNIDX;
    packet->result = 0;
}

void get_remote_fn_name(unsigned int fxn_idx, char* name)
{
	if(fxn_idx == FXN_TEST1) {
		strncpy(name, "fxnTest1", 9) ;
	} else {
		strncpy(name, "fxnMisc", 8) ;
	}
}

void test_exec_call(int fd, unsigned int p_addrA, unsigned int p_addrB, unsigned int p_addrC,
		int start_indx, int end_indx, int fxn_idx)
{
    int               i;
    uint16_t          server_status;
    int               packet_len;
    int               reply_len;
    char              packet_buf[512] = {0};
    char              return_buf[512] = {0};
    omx_packet        *packet = (omx_packet *)packet_buf;
    omx_packet        *rtn_packet = (omx_packet *)return_buf;
    fxn_double_args   *fxn_args = NULL;
    map_info_type     map_info = RPC_OMX_MAP_INFO_NONE;

    int num_iterations = 1 ;

    char fn_name[25] ;
    get_remote_fn_name(fxn_idx, fn_name) ;

    for (i = num_iterations; i <= num_iterations; i++) {

        /* Set Packet Header for the RCMServer, synchronous execution: */
        init_omx_packet(packet, OMX_DESC_MSG);

        /* Set OMX Function Index to call, with data: */
        packet->fxn_idx = fxn_idx ;

        /* Set data for the OMX function: */
        memcpy(packet->data, &map_info, sizeof(map_info));
        packet->data_size = sizeof(fxn_double_args) + sizeof(map_info) ;
        fxn_args = (fxn_double_args *)((char *)packet->data + sizeof(map_info));
        fxn_args->a = p_addrA ;
        fxn_args->b = p_addrB ;
        fxn_args->c = p_addrC ;
	fxn_args->start_indx = start_indx ;
	fxn_args->end_indx = end_indx ;

        /* Exec command: */
        packet_len = sizeof(omx_packet) + packet->data_size;
        exec_cmd(fd, (char *)packet, packet_len, (char *)rtn_packet, &reply_len);

        /* Decode reply: */
        server_status = (OMX_DESC_TYPE_MASK & rtn_packet->desc) >>
                OMX_DESC_TYPE_SHIFT;
        if (server_status == OMXSERVER_STATUS_SUCCESS)  {
           printf ("omx_benchmarkex: called %s(%d)), result = %d\n",
	   		fn_name, fxn_args->a, rtn_packet->result);
        }
        else {
           printf("omx_benchmark: Failed to execute %s : server status: %d\n",
	   	fn_name, server_status) ;
        }

    }
}


int attach_buffers(struct buffer** bufs, int rpmsg_fd, int cdev_fd)
{
	struct omap_bo* bo ;

	int retval = get_buffer_address(bufs[0], rpmsg_fd, cdev_fd) ;
	if(retval < 0)
		return retval ;
	retval = get_buffer_address(bufs[1], rpmsg_fd, cdev_fd) ;
	if(retval < 0)
		return retval ;
	retval = get_buffer_address(bufs[2], rpmsg_fd, cdev_fd) ;
	if(retval < 0)
		return retval ;

	bo = bufs[0]->bo[0] ;
	printf("buffer A (id=%d, u_addr=%p, p_addr=%x)\n", bo->fd, bo->map, bo->phy_addr) ;
	bo = bufs[1]->bo[0] ;
	printf("buffer B (id=%d, u_addr=%p, p_addr=%x)\n", bo->fd, bo->map, bo->phy_addr) ;
	bo = bufs[2]->bo[0] ;
	printf("buffer C (id=%d, u_addr=%p, p_addr=%x)\n", bo->fd, bo->map, bo->phy_addr) ;

	return 0 ;
}

void print_array(int array[SIZE][SIZE])
{
	int i, j ;
	for(i=0 ; i<SIZE ; i++) {
		for(j=0 ; j<SIZE ; j++) {
			printf("array[%d][%d] = %d\n", i, j, array[i][j]) ;
		}
	}
}

void print_arrays(struct buffer** buffers)
{
	int(*arra)[SIZE] =  (int(*)[SIZE])(buffers[0]->bo[0]->map) ;
	int(*arrb)[SIZE] =  (int(*)[SIZE])(buffers[1]->bo[0]->map) ;
	int(*arrc)[SIZE] =  (int(*)[SIZE])(buffers[2]->bo[0]->map) ;

	if(!arra || !arrb || !arrc)
		return ;

	print_array(arra) ;
	print_array(arrb) ;
	print_array(arrc) ;
}

void *local_compute(void* msg)
{
	struct message* mesg = (struct message *)msg ;
    	struct buffer** buffers = mesg->bufs ;

	if(mesg->fd == 0) {
		init_histo(histogram) ;
		call_barrier(0) ;
		compute_a9_thread1(void *x) ;
		call_barrier(1) ;
		compute_gray_level_mapping(histogram) ;
		call_barrier(2) ;
		compute_image_a9_thread1(void *x) ;
	} else {
		call_barrier(0) ;
		compute_a9_thread2(void *x) ;
		call_barrier(1) ;
		call_barrier(2) ;
		compute_image_a9_thread2(void *x) ;
	}

	return NULL ;
}

int verify_result(int buffer1[SIZE][SIZE], int buffer2[SIZE][SIZE], int buffer3[SIZE][SIZE])
{
	int i, j ;
	int pass ;

	multiply((int*)IB1, (int*)IB2, (int*)IB3, 0, SIZE) ;

	pass = 1 ;
	for(i=0 ; i<SIZE ; i++) {
		for(j=0 ; j<SIZE ; j++) {
			if(IB1[i][j] != buffer1[i][j]) {
				printf("Difference in first buffer i=%d,j=%d : %d, %d\n",i,j,IB1[i][j],buffer1[i][j]) ;
				pass = 0 ;                                             
			}
		}
	}

	for(i=0 ; i<SIZE ; i++) {
		for(j=0 ; j<SIZE ; j++) {
			if(IB2[i][j] != buffer2[i][j]) {                         
				printf("Difference in second buffer i=%d,j=%d : %d, %d\n",i,j,IB2[i][j],buffer2[i][j]) ;
				pass = 0 ;                                             
			}                                                              
		}
	}

	for(i=0 ; i<SIZE ; i++) {
		for(j=0 ; j<SIZE ; j++) {
			if(IB3[i][j] != buffer3[i][j]) {
				printf("Difference in third buffer i=%d,j=%d : %d, %d\n",i,j,IB3[i][j],buffer3[i][j]) ;
				pass = 0 ;
			}
		}
	}

	return pass ;
}

void wrapper_start_timer(int fd)
{
	if(fd == M3_FD)
		start_timer(&tm_m3) ;
	else if(fd == DSP_FD)
		start_timer(&tm_dsp) ;
	else
		assert(0) ;
}

void wrapper_stop_timer(int fd)
{
	if(fd == M3_FD)
		stop_timer(&tm_m3) ;
	else if(fd == DSP_FD)
		stop_timer(&tm_dsp) ;
	else
		assert(0) ;
}

void *remote_compute(void* msg)
{
	struct message* mesg = (struct message *)msg ;
    	int rpmsg_fd = mesg->fd ;
    	struct buffer** buffers = mesg->bufs ;
	unsigned int p_buffer1 = buffers[0]->bo[0]->phy_addr ;
	unsigned int p_buffer2 = buffers[1]->bo[0]->phy_addr ;
	unsigned int p_buffer3 = buffers[2]->bo[0]->phy_addr ;

	wrapper_start_timer(rpmsg_fd) ;
	test_exec_call(rpmsg_fd, p_buffer1, p_buffer2, p_buffer3, mesg->start_indx, mesg->end_indx, FXN_TEST1) ;
	wrapper_stop_timer(rpmsg_fd) ;

	return NULL ;
}

int open_rpmsg_dev(const char* rpmsg_dev)
{
    int rpmsg_fd = open(rpmsg_dev, O_RDWR);
    if (rpmsg_fd < 0) {
        perror("Can't open OMX device");
    } else {
	printf("Successfully opened %s with fd = %d\n", rpmsg_dev, rpmsg_fd) ;
    }

    return rpmsg_fd ;
}

void split_string(char* str, char delim, char** rproc, int* val)
{
	int len ;
	int delim_indx ;
	int i ;
	int str_length ;
	int val_str_length ;
	char* val_str ;

	len = strlen(str) ;
	printf("split_string : (str:%s,len=%d)\n", str, len) ;

	delim_indx = -1 ;
	for(i=0 ; i<=len ; i++)
		if(str[i] == delim) 
			delim_indx = i ;

	str_length = delim_indx ;
	*rproc = (char *)malloc(str_length+1) ;
	strncpy(*rproc, str, str_length) ;
	(*rproc)[str_length] = '\0' ;

	val_str_length = len - delim_indx ;
	val_str = (char *)malloc(val_str_length) ;
	strncpy(val_str, &(str[delim_indx+1]), val_str_length) ;
	*val = atoi(val_str) ;

	printf("split_string : (rproc:%s,val=%d)\n", *rproc, *val) ;
}

void process_rproc_string(char* rprocs[3], int rprocs_val[3], char* argv_str)
{
	int len ;
	int i ;
	int trail_pointer ;
	int num_rproc ;

	rprocs[0] = rprocs[1] = rprocs[2] = NULL ;
	rprocs_val[0] = rprocs_val[1] = rprocs_val[2] = 0 ;

	len = strlen(argv_str) ;
	printf("rproc_string : %s %d\n", argv_str, len) ;
	trail_pointer = 0 ;
	num_rproc = 0 ;
	for(i=0 ; i<=len ; i++)
	{
		if(num_rproc >= 3) break ;

		if((argv_str[i] == ',') || (argv_str[i] == '\0')) {
			int str_length = (i - trail_pointer + 1) ;
			char* tmp = (char *)malloc(str_length+1) ;
			strncpy(tmp, &(argv_str[trail_pointer]), str_length-1) ;
			tmp[str_length] = '\0' ;

			split_string(tmp, '=', &(rprocs[num_rproc]), &(rprocs_val[num_rproc])) ;

			trail_pointer = i + 1 ;
			num_rproc++ ;
		}
	}
}

void close_connection_to_remote_procs(struct rpmsg_fds* rpmsg_fd)
{
    struct omx_conn_req connreq = { .name = "OMX" } ;

    int ret = -1 ;
    if(rpmsg_fd->sysm3 >= 0) {
	    ret = close(rpmsg_fd->sysm3);
	    if (ret < 0) {
		    perror("Can't close OMX fd ??");
	    }
    	    printf("omx_sample: Closed connection to %s sysm3 with fd = %d\n", connreq.name, rpmsg_fd->sysm3);
    }

    if(rpmsg_fd->appm3 >= 0) {
	    ret = close(rpmsg_fd->appm3);
	    if (ret < 0) {
		    perror("Can't close OMX fd ??");
	    }
    	    printf("omx_sample: Closed connection to %s appm3 with fd = %d\n", connreq.name, rpmsg_fd->appm3);
    }

    if(rpmsg_fd->dsp >= 0) {
	    ret = close(rpmsg_fd->dsp);
	    if (ret < 0) {
		    perror("Can't close OMX fd ??");
	    }
    	    printf("omx_sample: Closed connection to %s dsp with fd = %d\n", connreq.name, rpmsg_fd->dsp);
    }
}

void open_connection_to_remote_procs(struct rpmsg_fds* rpmsg_fd, int rprocs_val[3], int argc, char*argv[])
{
	int i, j ;
    	struct omx_conn_req connreq = { .name = "OMX" } ;

	rpmsg_fd->sysm3 = -1 ;
	rpmsg_fd->appm3 = -1 ;
	rpmsg_fd->dsp = -1 ;

	for(i=0 ; i<argc ; i++)
	{
		char* rprocs[3] ;
		int tmp_rprocs_val[3] ;
		if(!argv[i] || strncmp("-r", argv[i],2))
			continue ;

		printf("Found rproc option %s\n", argv[i]) ;
		argv[i++] = NULL ;
		process_rproc_string(rprocs, tmp_rprocs_val, argv[i]) ;

		for(j=0 ; j<3 ; j++)
		{
			int ret = -1 ;
			if(!rprocs[j]) continue ;

			if (!strncmp(rprocs[j], "sysm3", 5)) {
				rpmsg_fd->sysm3 = open_rpmsg_dev("/dev/rpmsg-omx1") ;
				ret = ioctl(rpmsg_fd->sysm3, OMX_IOCCONNECT, &connreq) ;
				rprocs_val[0] = tmp_rprocs_val[j] ;
			} else if (!strncmp(rprocs[j], "appm3", 5)) {
				rpmsg_fd->appm3 = open_rpmsg_dev("/dev/rpmsg-omx1") ;
				ret = ioctl(rpmsg_fd->appm3, OMX_IOCCONNECT, &connreq) ;
				rprocs_val[1] = tmp_rprocs_val[j] ;
			} else if (!strncmp(rprocs[j], "dsp", 3)) {
				rpmsg_fd->dsp = open_rpmsg_dev("/dev/rpmsg-omx2") ;
				ret = ioctl(rpmsg_fd->dsp, OMX_IOCCONNECT, &connreq) ;
				rprocs_val[2] = tmp_rprocs_val[j] ;
			} else {
				ERROR("invalid arg: %s", rprocs[j]);
			}

			if (ret < 0) {
				perror("Can't connect to OMX instance");
			} else {
				printf("simple_test: Connected to %s\n", connreq.name);
			}
		}
	}
}

void fire_threads(struct rpmsg_fds rpmsg_fd, int* rprocs_val,struct buffer*** bufs, 
			int num_threads, int size_others)
{
	pthread_t thread1, thread2, thread3 ;
	struct message message1, message2, message3, message4[2] ;
	if(rpmsg_fd.sysm3 >= 0) {
		int ret1 ;
		printf("SYSM3\n") ;
		message1.fd = rpmsg_fd.sysm3 ;
		M3_FD = rpmsg_fd.sysm3 ;
		message1.bufs = *bufs ;
		message1.start_indx = 0 ;
		message1.end_indx = rprocs_val[0] ;
		printf("Calling test1 from %d to %d\n", message1.start_indx, message1.end_indx) ;
		ret1 = pthread_create( &thread1, NULL, remote_compute, (void*) &message1);
		if(ret1 != 0)
			printf("Failure in call of m3 thread\n") ;
	}
	
	if(rpmsg_fd.appm3 >= 0) {
		int ret2 ;
		printf("APPM3\n") ;
		message2.fd = rpmsg_fd.appm3 ;
		M3_FD = rpmsg_fd.appm3 ;
		message2.bufs = *bufs ;
		ret2 = pthread_create( &thread2, NULL, remote_compute, (void*) &message2);
		if(ret2 != 0)
			printf("Failure in call of m3 thread\n") ;
	} 

	if(rpmsg_fd.dsp >= 0) {
		int ret3 ;
		printf("DSP\n") ;
		message3.fd = rpmsg_fd.dsp ;
		DSP_FD = rpmsg_fd.dsp ;
		message3.bufs = *bufs ;
		message3.start_indx = rprocs_val[0]  ;
		message3.end_indx = (rprocs_val[0] + rprocs_val[2]) ;
		printf("Calling test1 from %d to %d\n", message3.start_indx, message3.end_indx) ;
		ret3 = pthread_create( &thread3, NULL, remote_compute, (void*) &message3);
		if(ret3 != 0)
			printf("Failure in call of dsp thread\n") ;
	}

	if(num_threads) {
		int j ;
		int remaining_size ;
		int previous_proc_stop_point ;
		pthread_t* thread = (pthread_t*)malloc(num_threads*sizeof(pthread_t)) ;
		int* ret4 = (int*)malloc(num_threads*sizeof(int)) ;
		int per_thread_load ;

		if(size_others == 0) {
			size_others = rprocs_val[0] + rprocs_val[2] ;
		}
		remaining_size=SIZE - size_others ;
		previous_proc_stop_point = size_others ;

		per_thread_load = remaining_size/num_threads ;
		printf("Spawning %d thread(s) on the A9\n", num_threads) ;
		start_timer(&tm_a9) ;
		for(j=0 ; j<num_threads ; j++) {
			message4[j].fd = j ;
			message4[j].bufs = *bufs ;
			message4[j].start_indx = previous_proc_stop_point ;
			message4[j].end_indx = previous_proc_stop_point + per_thread_load ;
			previous_proc_stop_point = previous_proc_stop_point + per_thread_load ;
			printf("Calling test1 from %d to %d\n", message4[j].start_indx, message4[j].end_indx) ;
			ret4[j] = pthread_create( &thread[j], NULL, local_compute, (void*) &message4[j]);
			if(ret4[j] != 0)
				printf("Failure in call of a9 thread\n") ;
		}

		for(j=0 ; j<num_threads ; j++) {
			pthread_join(thread[j], NULL) ;
		}
		stop_timer(&tm_a9) ;
		free(ret4) ;
		free(thread) ;
	}

	if(rpmsg_fd.sysm3 >= 0) { pthread_join( thread1, NULL) ; }
	if(rpmsg_fd.appm3 >= 0) { pthread_join( thread2, NULL) ; }
	if(rpmsg_fd.dsp >= 0) { pthread_join( thread3, NULL) ; }
}

int main(int argc, char *argv[])
{
    int ret = 0;
    int cdev_fd ;
    struct buffer** bufs ;
    struct rpmsg_fds rpmsg_fd ;
    int rprocs_val[3] ;
    int i ;
    int num_threads = 0 ;
    int size_others = 0 ;
    int indx ;
    
    init_timer(&tm_main) ;
    init_timer(&tm_m3) ;
    init_timer(&tm_dsp) ;
    init_timer(&tm_a9) ;
    init_timer(&tm_a9_0) ;
    init_timer(&tm_a9_1) ;

    start_timer(&tm_main) ;

    if ((cdev_fd = open("/dev/cdev_example", O_RDWR)) < 0) {
	    perror("open");
	    return -1;
    }

    #ifdef NEED_BARRIER
    open_barrier() ;
    #endif

    ret = open_display_and_allocate_buffers(&bufs, argc, argv) ;
    if(ret < 0) {
	    printf("Could not display and allocate buffers\n") ;
    }
    
    rprocs_val[0] = rprocs_val[1] = rprocs_val[2] = 0 ;
    open_connection_to_remote_procs(&rpmsg_fd, rprocs_val, argc, argv) ;

    for(i=0 ; i<argc ; i++) {
	    if(!argv[i])
		    continue ;

	    if(!strncmp("-l", argv[i], 2)) {
		    argv[i++] = NULL ;
		    num_threads = atoi(argv[i]) ;
		    if(num_threads > 2) {
			    printf("Currently only supports 2 threads on the A9\n") ;
			    exit(-1) ;
		    }

	    } else if(!strncmp("-x", argv[i], 2)) {
		    argv[i++] = NULL ;
		    size_others = atoi(argv[i]) ;
	    }
    }

    if(rpmsg_fd.sysm3 >= 0) attach_buffers(bufs, rpmsg_fd.sysm3, cdev_fd) ; 
    if(rpmsg_fd.appm3 >= 0) attach_buffers(bufs, rpmsg_fd.appm3, cdev_fd) ; 
    if(rpmsg_fd.dsp >= 0) attach_buffers(bufs, rpmsg_fd.dsp, cdev_fd) ; 

    #ifdef NEED_BARRIER
    num_threads_barrier = num_threads ;
    if(rpmsg_fd.sysm3 >= 0) num_threads_barrier +=2 ; 
    if(rpmsg_fd.appm3 >= 0) num_threads_barrier++ ; 
    if(rpmsg_fd.dsp >= 0) num_threads_barrier++ ; 
    #endif
    
    for(indx=0 ; indx<1 ; indx++)
    {
	    #ifdef NEED_BARRIER
	    init_barrier(num_threads_barrier) ;
	    #endif

	    fire_threads(rpmsg_fd,rprocs_val, &bufs, num_threads, size_others) ;

	    #ifdef NEED_BARRIER
	    finalize_barrier() ;
	    #endif
    }

    if(rpmsg_fd.sysm3 >= 0) { detach_buffers(bufs, rpmsg_fd.sysm3, cdev_fd) ; }
    if(rpmsg_fd.appm3 >= 0) { detach_buffers(bufs, rpmsg_fd.appm3, cdev_fd) ; }
    if(rpmsg_fd.dsp >= 0) { detach_buffers(bufs, rpmsg_fd.dsp, cdev_fd) ; }

    close_connection_to_remote_procs(&rpmsg_fd) ;
    
    //if(verify_result(buffer1, buffer2, buffer3)) {
    //        printf("Verified PASS\n") ;
    //} else {
    //        printf("Verified FAIL\n") ;
    //}

    //print_arrays(bufs) ; 

    deallocate_buffers(bufs) ;

    if (close(cdev_fd) < 0) {
	    perror("close");
	    return -1;
    }

    #ifdef NEED_BARRIER
    close_barrier() ;
    #endif

    stop_timer(&tm_main) ;

    print_float_time_summary(&tm_main, &tm_a9, &tm_a9_0, &tm_a9_1, &tm_m3, &tm_dsp) ;

    return 0;
}
