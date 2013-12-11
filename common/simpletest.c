#include "timer.h"
#include "simpletest.h"
#include "barrier.h"
#include "test.h"
#include "util.h"
#include <sys/mman.h>
#include <unistd.h>

struct timer tm_main ;
struct timer tm_m3 ;
struct timer tm_dsp ;
struct timer tm_a9 ;
struct timer tm_a9_0 ;
struct timer tm_a9_1 ;
struct timer exp_runtime ;

int num_procs ;
pthread_barrier_t barrier0, barrier1, barrier2, barrier3 ;

struct rpmsg_fds remote_fd ;
int cdev_fd ;

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
			#ifdef DEBUG
				printf("bo size = %d\n", bo->size) ;
			#endif
                }
        }
        return bo->map;
}

void attach_buffer(struct buffer buf, int rpmsg_omx_fd, int nbo)
{
        struct fds fd ;
        fd.dma_fd = buf.bo[nbo]->fd ;
        fd.rpmsg_omx_fd = rpmsg_omx_fd ;

	if(buf.rd_only) {
		if(ioctl(cdev_fd, ATTACH_RDONLY_IOCTL, (void*)&fd) < 0)
			perror("ATTACH RDONLY IOCTL");
	} else {
		if(ioctl(cdev_fd, ATTACH_RDWRITE_IOCTL, (void*)&fd) < 0)
			perror("ATTACH RDWRITE IOCTL");
	}
}

int get_buffer_address(struct buffer* buf, int rpmsg_omx_fd)
{
	int i ;
	for(i=0 ; i<buf->nbo ; i++) {
		if(buf->bo[i]->fd == 0) {
			omap_bo_dmabuf(buf->bo[i]) ;
		}

		if(buf->remote_attach) {
			attach_buffer(*buf, rpmsg_omx_fd, i) ;
			
			if(ioctl(cdev_fd, READ_IOCTL, (void*)&(buf->bo[i]->phy_addr)) < 0)
				perror("READ IOCTL") ;
		}
	}

        return 0 ;
}

void print_buffer_info(struct buffer* buf)
{
        printf("Height = %u, Width = %u, fourcc = %u, nbo = %d\n",
                buf->height, buf->width, buf->fourcc, buf->nbo) ;
        printf("map = %p, size = %u\n", (unsigned int*)(buf->bo[0]->map), buf->bo[0]->size) ;
}

void detach_buffer(struct buffer buf, int rpmsg_omx_fd, int nbo)
{
        struct fds fd ;
        fd.dma_fd = buf.bo[nbo]->fd ;
	fd.rpmsg_omx_fd = rpmsg_omx_fd ;

	if(ioctl(cdev_fd, DETACH_IOCTL, (void*)&fd) < 0)
		perror("DETACH IOCTL");
}

int detach_buffers(struct buffer* bufs, int nbuf, int rpmsg_fd)
{
	int i, j ;
	for(i=0 ; i<nbuf ; i++) {
		for(j=0 ; j<bufs[i].nbo ; j++)
			detach_buffer(bufs[i], rpmsg_fd, j) ;
	}

        return 0 ;
}

void deallocate_buffers(struct buffer* buf, int nbuf)
{
	int i ;
	#ifdef DEBUG
        printf("Begin deallocate_buffers\n") ;
	#endif
	for(i=0 ; i<nbuf ; i++) {
		omap_bo_del(buf[i].bo[0]) ;
		close(buf[i].bo[0]->fd) ;
	}
	#ifdef DEBUG
	printf("Finish deallocate_buffers\n") ;
	#endif
}

int open_display_and_allocate_buffers(struct buffer* bufs, int nbuf, int argc, char* argv[])
{
        struct display *disp;
	int i,k ;

	#ifdef DEBUG
        MSG("Opening Display..");
	#endif
        disp = disp_kms_open(argc, argv);
        if (!disp) {
		#ifdef DEBUG
                printf("simpletest : Opening display failed\n");
		#endif
                return -1; 
        } else {
		#ifdef DEBUG
                printf("simpletest : Opening display succeeded\n");
		#endif
	}

	set_buffer_properties(bufs, nbuf) ;

	alloc_buffers(disp, bufs, nbuf);

	for(i=0 ; i<nbuf ; i++) {
		for(k=0 ; k<bufs[i].nbo ; k++) {
			local_omap_bo_map(bufs[i].bo[k]) ;
			//print_buffer_info(&bufs[i]) ;
		}
	}

	init_buffers(bufs, nbuf) ;

	return 0 ;
}

int exec_cmd(int fd, char *msg, int len, char *reply_msg, int *reply_len)
{
    int ret = 0;
    #ifdef DEBUG
    printf("Executing remote command via rpmsg\n") ;
    #endif

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
	unsigned int p_addrD, unsigned int p_addrE, unsigned int p_addrF, 
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
        fxn_args->d = p_addrD ;
        fxn_args->e = p_addrE ;
        fxn_args->f = p_addrF ;
	fxn_args->start_indx = start_indx ;
	fxn_args->end_indx = end_indx ;

        /* Exec command: */
        packet_len = sizeof(omx_packet) + packet->data_size;
        exec_cmd(fd, (char *)packet, packet_len, (char *)rtn_packet, &reply_len);

        /* Decode reply: */
        server_status = (OMX_DESC_TYPE_MASK & rtn_packet->desc) >>
                OMX_DESC_TYPE_SHIFT;
        if (server_status == OMXSERVER_STATUS_SUCCESS)  {
           #ifdef DEBUG
           printf ("omx_benchmarkex: called %s(%d)), result = %d\n",
	   		fn_name, fxn_args->a, rtn_packet->result);
	   #endif
        }
        else {
           printf("omx_benchmark: Failed to execute %s : server status: %d\n",
	   	fn_name, server_status) ;
        }

    }
}


int attach_buffers(struct buffer* bufs, int nbuf, int rpmsg_fd)
{
	struct omap_bo* bo ;
	int retval ;
	int i ;

	for(i=0 ; i<nbuf ; i++) {
		retval = get_buffer_address(&bufs[i], rpmsg_fd) ;
		if(retval < 0)
			return retval ;
		bo = bufs[i].bo[0] ;
		#ifdef DEBUG
		printf("buffer %c (id=%d, u_addr=%p, p_addr=%x)\n", 'A'+i, bo->fd, bo->map, bo->phy_addr) ;
		#endif
		if(bufs[i].rd_only) {
			#ifdef DEBUG
			printf("buffer %c-1 (id=%d, u_addr=%p)\n", 'A'+i, bo->fd, bo->map) ;
			#endif
		}
	}

	return 0 ;
}

void* get_a9_address(struct buffer buf)
{
	if(buf.rd_only) {
		return buf.bo[0]->map ; //KC : made change here
		//return buf.bo[1]->map ; 
	} else {
		return buf.bo[0]->map ;
	}
}

void wrapper_start_timer(int fd)
{
	if(fd == remote_fd.sysm3)
		start_timer(&tm_m3) ;
	else if(fd == remote_fd.dsp)
		start_timer(&tm_dsp) ;
	else
		assert(0) ;
}

void wrapper_stop_timer(int fd)
{
	if(fd == remote_fd.sysm3)
		stop_timer(&tm_m3) ;
	else if(fd == remote_fd.dsp)
		stop_timer(&tm_dsp) ;
	else
		assert(0) ;
}

void *remote_compute(void* msg)
{
	struct message* mesg = (struct message *)msg ;
    	int rpmsg_fd = mesg->fd ;
    	struct buffer* buffers = mesg->bufs ;
	unsigned int p_buffer1 = NBUF>0 ? buffers[0].bo[0]->phy_addr : 0 ;
	unsigned int p_buffer2 = NBUF>1 ? buffers[1].bo[0]->phy_addr : 0 ;
	unsigned int p_buffer3 = NBUF>2 ? buffers[2].bo[0]->phy_addr : 0 ;
	unsigned int p_buffer4 = NBUF>3 ? buffers[3].bo[0]->phy_addr : 0 ;
	unsigned int p_buffer5 = NBUF>4 ? buffers[4].bo[0]->phy_addr : 0 ;
	unsigned int p_buffer6 = NBUF>5 ? buffers[5].bo[0]->phy_addr : 0 ;
	
	#ifdef DEBUG
	printf("b1=%d,b2=%d,b3=%d,b4=%d,b5=%d,b6=%d\n",
		p_buffer1, p_buffer2, p_buffer3, p_buffer4, p_buffer5, p_buffer6) ;
	#endif

	wrapper_start_timer(rpmsg_fd) ;
	test_exec_call(rpmsg_fd, p_buffer1, p_buffer2, p_buffer3, p_buffer4, 
			p_buffer5, p_buffer6, mesg->start_indx, mesg->end_indx, FXN_TEST1) ;
	wrapper_stop_timer(rpmsg_fd) ;

	return NULL ;
}

void *local_compute(void* msg)
{
	struct message* mesg = (struct message *)msg ;
    	struct buffer* buffers = mesg->bufs ;
	int tid = mesg->fd ;

	a9_compute(tid, buffers, mesg->start_indx, mesg->end_indx) ;

	return NULL ;
}

int open_rpmsg_dev(const char* rpmsg_dev)
{
    int rpmsg_fd = open(rpmsg_dev, O_RDWR);
    if (rpmsg_fd < 0) {
        perror("Can't open OMX device");
    } else {
	#ifdef DEBUG
	printf("Successfully opened %s with fd = %d\n", rpmsg_dev, rpmsg_fd) ;
	#endif
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
	#ifdef DEBUG
	printf("split_string : (str:%s,len=%d)\n", str, len) ;
	#endif

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
	
	#ifdef DEBUG
	printf("split_string : (rproc:%s,val=%d)\n", *rproc, *val) ;
	#endif
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
	#ifdef DEBUG
	printf("rproc_string : %s %d\n", argv_str, len) ;
	#endif
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

void close_connection_to_remote_procs()
{
    struct omx_conn_req connreq = { .name = "OMX" } ;

    int ret = -1 ;
    if(remote_fd.sysm3 >= 0) {
	    ret = close(remote_fd.sysm3);
	    if (ret < 0) {
		    perror("Can't close OMX fd ??");
	    }
            #ifdef DEBUG
    	    printf("omx_sample: Closed connection to %s sysm3 with fd = %d\n", connreq.name, remote_fd.sysm3);
            #endif
    }

    if(remote_fd.appm3 >= 0) {
	    ret = close(remote_fd.appm3);
	    if (ret < 0) {
		    perror("Can't close OMX fd ??");
	    }
            #ifdef DEBUG
    	    printf("omx_sample: Closed connection to %s appm3 with fd = %d\n", connreq.name, remote_fd.appm3);
            #endif
    }

    if(remote_fd.dsp >= 0) {
	    ret = close(remote_fd.dsp);
	    if (ret < 0) {
		    perror("Can't close OMX fd ??");
	    }
            #ifdef DEBUG
    	    printf("omx_sample: Closed connection to %s dsp with fd = %d\n", connreq.name, remote_fd.dsp);
            #endif
    }
}

void open_connection_to_remote_procs(int rprocs_val[3], int argc, char*argv[])
{
	int i, j ;
    	struct omx_conn_req connreq = { .name = "OMX" } ;

	remote_fd.sysm3 = -1 ;
	remote_fd.appm3 = -1 ;
	remote_fd.dsp = -1 ;

	for(i=0 ; i<argc ; i++)
	{
		char* rprocs[3] ;
		int tmp_rprocs_val[3] ;
		if(!argv[i] || strncmp("-r", argv[i],2))
			continue ;

		#ifdef DEBUG
		printf("Found rproc option %s\n", argv[i]) ;
		#endif
		argv[i++] = NULL ;
		process_rproc_string(rprocs, tmp_rprocs_val, argv[i]) ;

		for(j=0 ; j<3 ; j++)
		{
			int ret = -1 ;
			if(!rprocs[j]) continue ;

			if (!strncmp(rprocs[j], "sysm3", 5)) {
				remote_fd.sysm3 = open_rpmsg_dev("/dev/rpmsg-omx1") ;
				ret = ioctl(remote_fd.sysm3, OMX_IOCCONNECT, &connreq) ;
				rprocs_val[0] = tmp_rprocs_val[j] ;
			} else if (!strncmp(rprocs[j], "appm3", 5)) {
				remote_fd.appm3 = open_rpmsg_dev("/dev/rpmsg-omx1") ;
				ret = ioctl(remote_fd.appm3, OMX_IOCCONNECT, &connreq) ;
				rprocs_val[1] = tmp_rprocs_val[j] ;
			} else if (!strncmp(rprocs[j], "dsp", 3)) {
				remote_fd.dsp = open_rpmsg_dev("/dev/rpmsg-omx2") ;
				ret = ioctl(remote_fd.dsp, OMX_IOCCONNECT, &connreq) ;
				rprocs_val[2] = tmp_rprocs_val[j] ;
			} else {
				ERROR("invalid arg: %s", rprocs[j]);
			}

			if (ret < 0) {
				perror("Can't connect to OMX instance");
			} else {
				#ifdef DEBUG
				printf("simple_test: Connected to %s\n", connreq.name);
				#endif
			}
		}
	}
}

void fire_threads(int* rprocs_val,struct buffer* bufs, 
			int num_a9_threads, int size_others)
{
	pthread_t thread1, thread2, thread3 ;
	struct message message1, message2, message3, message4[2] ;
	if(remote_fd.sysm3 >= 0) {
		int ret1 ;
		#ifdef DEBUG
		printf("SYSM3\n") ;
		#endif
		message1.fd = remote_fd.sysm3 ;
		message1.bufs = bufs ;
		message1.start_indx = 0 ;
		message1.end_indx = rprocs_val[0] ;
		#ifdef DEBUG
		printf("Calling test1 from %d to %d\n", message1.start_indx, message1.end_indx) ;
		#endif
		ret1 = pthread_create( &thread1, NULL, remote_compute, (void*) &message1);
		if(ret1 != 0)
			printf("Failure in call of m3 thread\n") ;
	}
	
	if(remote_fd.appm3 >= 0) {
		int ret2 ;
		#ifdef DEBUG
		printf("APPM3\n") ;
		#endif
		message2.fd = remote_fd.appm3 ;
		message2.bufs = bufs ;
		ret2 = pthread_create( &thread2, NULL, remote_compute, (void*) &message2);
		if(ret2 != 0)
			printf("Failure in call of m3 thread\n") ;
	} 

	if(remote_fd.dsp >= 0) {
		int ret3 ;
		#ifdef DEBUG
		printf("DSP\n") ;
		#endif
		message3.fd = remote_fd.dsp ;
		message3.bufs = bufs ;
		message3.start_indx = rprocs_val[0]  ;
		message3.end_indx = (rprocs_val[0] + rprocs_val[2]) ;
		#ifdef DEBUG
		printf("Calling test1 from %d to %d\n", message3.start_indx, message3.end_indx) ;
		#endif
		ret3 = pthread_create( &thread3, NULL, remote_compute, (void*) &message3);
		if(ret3 != 0)
			printf("Failure in call of dsp thread\n") ;
	}

	if(num_a9_threads) {
		int j ;
		int remaining_size ;
		int previous_proc_stop_point ;
		pthread_t* thread = (pthread_t*)malloc(num_a9_threads*sizeof(pthread_t)) ;
		int* ret4 = (int*)malloc(num_a9_threads*sizeof(int)) ;
		int per_thread_load ;


		if(size_others == 0) {
			size_others = rprocs_val[0] + rprocs_val[2] ;
		}
		remaining_size = find_remaining_size(size_others) ;
		previous_proc_stop_point = size_others ;

		per_thread_load = remaining_size/num_a9_threads ;
		#ifdef DEBUG
		printf("Spawning %d thread(s) on the A9\n", num_a9_threads) ;
		#endif
		start_timer(&tm_a9) ;
		for(j=0 ; j<num_a9_threads ; j++) {
			message4[j].fd = j ;
			message4[j].bufs = bufs ;
			message4[j].start_indx = previous_proc_stop_point ;
			message4[j].end_indx = previous_proc_stop_point + per_thread_load ;
			previous_proc_stop_point = previous_proc_stop_point + per_thread_load ;
			#ifdef DEBUG
			printf("Calling test1 from %d to %d\n", message4[j].start_indx, message4[j].end_indx) ;
			#endif
			ret4[j] = pthread_create( &thread[j], NULL, local_compute, (void*) &message4[j]);
			if(ret4[j] != 0)
				printf("Failure in call of a9 thread\n") ;
		}

		for(j=0 ; j<num_a9_threads ; j++) {
			pthread_join(thread[j], NULL) ;
		}
		stop_timer(&tm_a9) ;
		free(ret4) ;
		free(thread) ;
	}

	if(remote_fd.sysm3 >= 0) { pthread_join( thread1, NULL) ; }
	if(remote_fd.appm3 >= 0) { pthread_join( thread2, NULL) ; }
	if(remote_fd.dsp >= 0) { pthread_join( thread3, NULL) ; }
}

int main(int argc, char *argv[])
{
    sleep(15) ;
	    
    int ret = 0;
    struct buffer bufs[NBUF] ;
    int rprocs_val[3] ;
    int i ;
    int num_a9_threads = 0 ;
    int size_others = 0 ;
    int indx ;
    
    init_timer(&tm_main) ;
    init_timer(&tm_m3) ;
    init_timer(&tm_dsp) ;
    init_timer(&tm_a9) ;
    init_timer(&tm_a9_0) ;
    init_timer(&tm_a9_1) ;
    init_timer(&exp_runtime) ;

    start_timer(&tm_main) ;

    if ((cdev_fd = open("/dev/cdev_example", O_RDWR)) < 0) {
	    perror("open");
	    return -1;
    }

    #ifdef NEED_BARRIER
    open_barrier() ;
    #endif

    rprocs_val[0] = rprocs_val[1] = rprocs_val[2] = 0 ;
    open_connection_to_remote_procs(rprocs_val, argc, argv) ;


    ret = open_display_and_allocate_buffers(bufs, NBUF, argc, argv) ;
    if(ret < 0) {
	    printf("Could not display and allocate buffers\n") ;
    }

    for(i=0 ; i<argc ; i++) {
	    if(!argv[i])
		    continue ;

	    if(!strncmp("-l", argv[i], 2)) {
		    argv[i++] = NULL ;
		    #ifdef DEBUG
		    printf("Saw l, value =%s\n", argv[i]) ;
		    #endif
		    num_a9_threads = atoi(argv[i]) ;
		    if(num_a9_threads > 2) {
			    printf("Currently only supports 2 threads on the A9\n") ;
			    exit(-1) ;
		    }

	    } else if(!strncmp("-x", argv[i], 2)) {
		    argv[i++] = NULL ;
		    size_others = atoi(argv[i]) ;
	    }
    }
    #ifdef DEBUG
    printf("Finished processing options\n") ;
    #endif

    if(remote_fd.sysm3 >= 0) attach_buffers(bufs, NBUF, remote_fd.sysm3) ; 
    if(remote_fd.appm3 >= 0) attach_buffers(bufs, NBUF, remote_fd.appm3) ; 
    if(remote_fd.dsp >= 0) attach_buffers(bufs, NBUF, remote_fd.dsp) ; 

    #ifdef DEBUG
    printf("Finished attaching buffers\n") ;
    #endif
    
    //int mret = touch_all_buffers(bufs, NBUF) ;
    //printf("%d\n", mret) ;

    #ifdef NEED_BARRIER
    num_procs = num_a9_threads>0 ? 1 : 0 ;
    if((remote_fd.sysm3 >= 0) || (remote_fd.appm3 >= 0)) num_procs++ ; 
    if(remote_fd.dsp >= 0) num_procs++ ; 
    #endif

    for(indx=0 ; indx<NUM_ITER*3 ; indx++) {
	    //printf("Initializing barrier with num_a9_threads = %d\n", num_a9_threads) ;
	    pthread_barrier_init(&barrier0,NULL,num_a9_threads);
	    pthread_barrier_init(&barrier1,NULL,num_a9_threads);
	    pthread_barrier_init(&barrier2,NULL,num_a9_threads);
	    pthread_barrier_init(&barrier3,NULL,num_a9_threads);
    }
    
    for(indx=0 ; indx<1 ; indx++)
    {
	    #ifdef NEED_BARRIER
	    init_barrier(num_procs) ;
	    #endif

	    start_timer(&exp_runtime) ;
	    fire_threads(rprocs_val, bufs, num_a9_threads, size_others) ;
	    stop_timer(&exp_runtime) ;

	    #ifdef NEED_BARRIER
	    finalize_barrier() ;
	    #endif
    }

    if(remote_fd.sysm3 >= 0) { detach_buffers(bufs, NBUF, remote_fd.sysm3) ; }
    if(remote_fd.appm3 >= 0) { detach_buffers(bufs, NBUF, remote_fd.appm3) ; }
    if(remote_fd.dsp >= 0) { detach_buffers(bufs, NBUF, remote_fd.dsp) ; }

    close_connection_to_remote_procs() ;
    
    if(verify_result(bufs)) {
    	    #ifdef DEBUG
            printf("Verified PASS\n") ;
	    #endif
    } else {
    	    #ifdef DEBUG
            printf("Verified FAIL\n") ;
	    #endif
    }


    deallocate_buffers(bufs, NBUF) ;

    if (close(cdev_fd) < 0) {
	    perror("close");
	    return -1;
    }

    #ifdef NEED_BARRIER
    close_barrier() ;
    #endif

    stop_timer(&tm_main) ;

    print_float_time_summary(&tm_main, &exp_runtime, &tm_a9, &tm_a9_0, &tm_a9_1, &tm_m3, &tm_dsp) ;

    return 0;
}
