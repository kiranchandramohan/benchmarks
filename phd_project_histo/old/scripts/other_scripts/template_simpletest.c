#include <errno.h>
#include "util.h"
#include <pthread.h>
#include <unistd.h>

#define NBUF 3
#define CNT  500
#define SIZE 1024


#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/eventfd.h>

#include <sys/mman.h>
#include <xf86drm.h>
#include <sys/time.h>

#include"rpmsg_omx.h"

#include "omx_packet.h"

#define MY_MACIG 'G'
#define READ_IOCTL _IOR(MY_MACIG, 0, int)
#define WRITE_IOCTL _IOW(MY_MACIG, 1, int)
#define ATTACH_IOCTL _IOW(MY_MACIG, 2, int)
#define DETACH_IOCTL _IOW(MY_MACIG, 3, int)

struct message
{
	int fd ;
	struct buffer** bufs ;
	int start_indx ;
	int end_indx ;
} ;

struct fds {
	int dma_fd ;
	int rpmsg_omx_fd ;
} ;

struct omap_device {
	        int fd; 
};

struct omap_bo {
	struct omap_device	*dev;
	void		*map;		/* userspace mmap'ing (if there is one) */
	uint32_t	size;
	uint32_t	handle;
	uint32_t	name;		/* flink global handle (DRI2 name) */
	uint64_t	offset;		/* offset to mmap() */
	int		fd;		/* dmabuf handle */
	uint32_t	phy_addr;
};

struct timezone tz ;
struct timeval tv_start, tv_end ;
struct timeval tv_m3_start, tv_m3_end ;
struct timeval tv_dsp_start, tv_dsp_end ;
struct timeval tv_a9_start, tv_a9_end ;

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


void  change_to_read_only_mmap(struct buffer** bufs)
{
	munmap(bufs[0]->bo[0]->map, bufs[0]->bo[0]->size) ;
	
        bufs[0]->bo[0]->map = mmap(0, bufs[0]->bo[0]->size, PROT_READ, MAP_SHARED, 
			bufs[0]->bo[0]->dev->fd, bufs[0]->bo[0]->offset);
        if (bufs[0]->bo[0]->map == MAP_FAILED) {
		printf("Mapping failed\n") ;
                bufs[0]->bo[0]->map = NULL;
        }
	   
	munmap(bufs[1]->bo[0]->map, bufs[1]->bo[0]->size) ;
	
        bufs[1]->bo[0]->map = mmap(0, bufs[1]->bo[0]->size, PROT_READ, MAP_SHARED, 
			bufs[1]->bo[0]->dev->fd, bufs[1]->bo[0]->offset);
        if (bufs[1]->bo[0]->map == MAP_FAILED) {
		printf("Mapping failed\n") ;
                bufs[1]->bo[0]->map = NULL;
        }
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
	fd.rpmsg_omx_fd = rpmsg_fd ;

	struct buffer* buf = bufs[0] ;
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

void print_array(int(*arr)[SIZE])
{
	printf("----------------------------- ARRAY -----------------------------\n") ;
	int i, j ;
	for(i=0; i < SIZE ; i++) {
		for(j=0; j < SIZE ; j++) {
			printf("%d ", arr[i][j]) ;
		}
		printf("\n") ;
	}
}

void multiply(int a[][SIZE], int b[][SIZE], int c[][SIZE], int start, int end)
{
  int i,j,k;
  for (i = start ; i < end; i++)
  {
    for (j = 0; j < SIZE; j++)
    {   
      //c[i][j] = 0;
      for ( k = 0; k < SIZE; k++)
        c[i][j] += a[i][k]*b[k][j];
    }   
  }
}

typedef struct {
    unsigned int a;
    unsigned int b;
    unsigned int c;
    unsigned int start_indx;
    unsigned int end_indx;
} fxn_double_args;

/* The map_info_type enum is defined in the rpmsg_omx driver, and is defined
 * elsewhere in userspace, but define it here for this sample. The data portion
 * of the packet being written is expected to start with this.
 */
typedef enum {
    RPC_OMX_MAP_INFO_NONE       = 0,
    RPC_OMX_MAP_INFO_ONE_BUF    = 1,
    RPC_OMX_MAP_INFO_TWO_BUF    = 2,
    RPC_OMX_MAP_INFO_THREE_BUF  = 3,
    RPC_OMX_MAP_INFO_MAX        = 0x7FFFFFFF
} map_info_type;

/* Note: Set bit 31 to indicate static function indicies:
 * This function order will be hardcoded on BIOS side, hence preconfigured:
 * See src/examples/srvmgr/test_omx.c.
 */
#define FXN_IDX_FXNDOUBLE            (3 | 0x80000000)

int exec_cmd(int fd, char *msg, int len, char *reply_msg, int *reply_len)
{
    printf("Executing remote command via rpmsg\n") ;
    int ret = 0;

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

void test_exec_call(int fd, unsigned int p_addrA, unsigned int p_addrB, unsigned int p_addrC,
		    int start_indx, int end_indx)
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
    long              elapsed = 0, delta;
    map_info_type     map_info = RPC_OMX_MAP_INFO_NONE;

    int num_iterations = 1 ;

    for (i = num_iterations; i <= num_iterations; i++) {

        /* Set Packet Header for the RCMServer, synchronous execution: */
        init_omx_packet(packet, OMX_DESC_MSG);

        /* Set OMX Function Index to call, with data: */
        packet->fxn_idx = FXN_IDX_FXNDOUBLE;

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

           printf ("omx_benchmarkex: called fxnDouble(%d)), result = %d\n",
                        fxn_args->a, rtn_packet->result);
        }
        else {
           printf("omx_benchmark: Failed to execute fxnDouble: server status: %d\n",
                server_status);
        }

    }
}


int attach_buffers(struct buffer** bufs, int rpmsg_fd, int cdev_fd)
{
	int retval = get_buffer_address(bufs[0], rpmsg_fd, cdev_fd) ;
	if(retval < 0)
		return retval ;
	retval = get_buffer_address(bufs[1], rpmsg_fd, cdev_fd) ;
	if(retval < 0)
		return retval ;
	retval = get_buffer_address(bufs[2], rpmsg_fd, cdev_fd) ;
	if(retval < 0)
		return retval ;

	struct omap_bo* bo = bufs[0]->bo[0] ;
	printf("buffer A (id=%d, u_addr=%p, p_addr=%x)\n", bo->fd, bo->map, bo->phy_addr) ;
	bo = bufs[1]->bo[0] ;
	printf("buffer B (id=%d, u_addr=%p, p_addr=%x)\n", bo->fd, bo->map, bo->phy_addr) ;
	bo = bufs[2]->bo[0] ;
	printf("buffer C (id=%d, u_addr=%p, p_addr=%x)\n", bo->fd, bo->map, bo->phy_addr) ;

	return 0 ;
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

int A[SIZE][SIZE], B[SIZE][SIZE], C[SIZE][SIZE] ;
void init_matrices(struct buffer** buffers)
{
	int(*arra)[SIZE] =  (int(*)[SIZE])(buffers[0]->bo[0]->map) ;
	int(*arrb)[SIZE] =  (int(*)[SIZE])(buffers[1]->bo[0]->map) ;
	int(*arrc)[SIZE] =  (int(*)[SIZE])(buffers[2]->bo[0]->map) ;

	if(!arra || !arrb || !arrc)
		return ;

	printf("Beginning initialization %p, %p, %p\n", arra, arrb, arrc) ;
	int i, j ;
	for(i=0; i <SIZE ; i++)
		for(j=0; j <SIZE ; j++) {
			if(i<256) {
				arra[i][j] = 1 ;
				A[i][j] = 1 ;
			} else if(i<256+128) {
				arra[i][j] = 2 ;
				A[i][j] = 2 ;
			} else {
				arra[i][j] = 3 ;
				A[i][j] = 3 ;
			}
			arrb[i][j] = 3 ;
			B[i][j] = 3 ;
			arrc[i][j] = 0 ;
			C[i][j] = 0 ;
		}
	printf("Finishing initialization\n") ;
/*
	int ret = msync(&(arra[0][0]),SIZE*SIZE*4,MS_SYNC) ;
	if (ret < 0) {
		perror("Can't sync A");
	}
	ret = msync(&(arrb[0][0]),SIZE*SIZE*4,MS_SYNC) ;
	if(ret < 0) {
		perror("Can't sync B");
	}
	ret = msync(&(arrc[0][0]),SIZE*SIZE*4,MS_SYNC) ;
	if(ret < 0) {
		perror("Can't sync C");
	}
*/
	//print_arrays(buffers) ;  
}

#INSERT_MPY1

#INSERT_MPY2

void *local_compute(void* msg)
{
	struct message* mesg = (struct message *)msg ;
    	struct buffer** buffers = mesg->bufs ;

	if(mesg->fd == 0) {
		multiply1(buffers[0]->bo[0]->map, buffers[1]->bo[0]->map, buffers[2]->bo[0]->map) ;
	} else {
		multiply2(buffers[0]->bo[0]->map, buffers[1]->bo[0]->map, buffers[2]->bo[0]->map) ;
	}

	return NULL ;
}

int verify_result(struct buffer** buffers)
{
	int(*arra)[SIZE] =  (int(*)[SIZE])(buffers[0]->bo[0]->map) ;
	int(*arrb)[SIZE] =  (int(*)[SIZE])(buffers[1]->bo[0]->map) ;
	int(*arrc)[SIZE] =  (int(*)[SIZE])(buffers[2]->bo[0]->map) ;
	
	multiply(A, B, C, 0, SIZE) ;

	int i, j ;
	int pass = 1 ;
	for(i=0 ; i<SIZE ; i++) {
		for(j=0 ; j<SIZE ; j++) {
			if(C[i][j] != arrc[i][j]) {
				printf("Not equal [%d][%d] %d != %d\n",i,j,C[i][j],arrc[i][j]) ;
				pass = 0 ;
			}
		}
	}

	return pass ;
}

void *remote_compute_m3(void* msg)
{
	struct message* mesg = (struct message *)msg ;
    	struct buffer** buffers = mesg->bufs ;
	unsigned int p_addrA = buffers[0]->bo[0]->phy_addr ;
	unsigned int p_addrB = buffers[1]->bo[0]->phy_addr ;
	unsigned int p_addrC = buffers[2]->bo[0]->phy_addr ;

        gettimeofday(&tv_m3_start, &tz) ;
    	int rpmsg_fd = mesg->fd ;
	test_exec_call(rpmsg_fd, p_addrA, p_addrB, p_addrC, mesg->start_indx, mesg->end_indx) ;
        gettimeofday(&tv_m3_end, &tz) ;

	return NULL ;
}

void *remote_compute_dsp(void* msg)
{
	struct message* mesg = (struct message *)msg ;
    	struct buffer** buffers = mesg->bufs ;
	unsigned int p_addrA = buffers[0]->bo[0]->phy_addr ;
	unsigned int p_addrB = buffers[1]->bo[0]->phy_addr ;
	unsigned int p_addrC = buffers[2]->bo[0]->phy_addr ;

        gettimeofday(&tv_dsp_start, &tz) ;
    	int rpmsg_fd = mesg->fd ;
	test_exec_call(rpmsg_fd, p_addrA, p_addrB, p_addrC, mesg->start_indx, mesg->end_indx) ;
        gettimeofday(&tv_dsp_end, &tz) ;

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

struct rpsmg_fds
{
	int sysm3 ;
	int appm3 ;
	int dsp ;
} ;

void split_string(char* str, char delim, char** rproc, int* val)
{
	int len = strlen(str) ;
	printf("split_string : (str:%s,len=%d)\n", str, len) ;

	int delim_indx = -1 ;
	int i ;
	for(i=0 ; i<=len ; i++)
		if(str[i] == delim) 
			delim_indx = i ;

	int str_length = delim_indx ;
	*rproc = (char *)malloc(str_length+1) ;
	strncpy(*rproc, str, str_length) ;
	(*rproc)[str_length] = '\0' ;

	int val_str_length = len - delim_indx ;
	char* val_str = (char *)malloc(val_str_length) ;
	strncpy(val_str, &(str[delim_indx+1]), val_str_length) ;
	*val = atoi(val_str) ;

	printf("split_string : (rproc:%s,val=%d)\n", *rproc, *val) ;
}

void process_rproc_string(char* rprocs[3], int rprocs_val[3], char* argv_str)
{
	rprocs[0] = rprocs[1] = rprocs[2] = NULL ;
	rprocs_val[0] = rprocs_val[1] = rprocs_val[2] = 0 ;

	int len = strlen(argv_str) ;
	printf("rproc_string : %s %d\n", argv_str, len) ;
	int i ;
	int trail_pointer = 0 ;
	int num_rproc = 0 ;
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

struct rpmsg_fds
{
	int sysm3 ;
	int appm3 ;
	int dsp ;
} ;

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
    	struct omx_conn_req connreq = { .name = "OMX" } ;

	rpmsg_fd->sysm3 = -1 ;
	rpmsg_fd->appm3 = -1 ;
	rpmsg_fd->dsp = -1 ;

	int i ;
	for(i=0 ; i<argc ; i++)
	{
		if(!argv[i] || strncmp("-r", argv[i],2))
			continue ;

		printf("Found rproc option %s\n", argv[i]) ;
		argv[i++] = NULL ;
		char* rprocs[3] ;
		int tmp_rprocs_val[3] ;
		process_rproc_string(rprocs, tmp_rprocs_val, argv[i]) ;

		int j ;
		for(j=0 ; j<3 ; j++)
		{
			if(!rprocs[j]) continue ;

			int ret = -1 ;
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

void print_time_summary()
{
	long int total_time = 
	((tv_end.tv_sec * 1000000 + tv_end.tv_usec) - (tv_start.tv_sec * 1000000 + tv_start.tv_usec))/1000000 ;
	long int a9_time =
	((tv_a9_end.tv_sec * 1000000 + tv_a9_end.tv_usec) - (tv_a9_start.tv_sec * 1000000 + tv_a9_start.tv_usec))/1000000 ;
	long int m3_time =
	((tv_m3_end.tv_sec * 1000000 + tv_m3_end.tv_usec) - (tv_m3_start.tv_sec * 1000000 + tv_m3_start.tv_usec))/1000000 ;
	long int dsp_time =
	((tv_dsp_end.tv_sec * 1000000 + tv_dsp_end.tv_usec) - (tv_dsp_start.tv_sec * 1000000 + tv_dsp_start.tv_usec))/1000000 ;

	printf("Time<total,a9,m3,dsp> = <%ld,%ld,%ld,%ld>\n", total_time,a9_time,m3_time,dsp_time) ;
}

int main(int argc, char *argv[])
{
    gettimeofday(&tv_start, &tz) ;

    pthread_t thread1, thread2, thread3, thread4 ;
    int ret = 0;

    int cdev_fd ;
    if ((cdev_fd = open("/dev/cdev_example", O_RDWR)) < 0) {
	    perror("open");
	    return -1;
    }

    struct buffer** bufs ;
    ret = open_display_and_allocate_buffers(&bufs, argc, argv) ;
    if(ret < 0) {
	    printf("Could not display and allocate buffers\n") ;
    }

    init_matrices(bufs) ;

    //change_to_read_only_mmap(bufs) ; //Change A and B to read only

    struct rpmsg_fds rpmsg_fd ;
    int rprocs_val[3] ;
    rprocs_val[0] = rprocs_val[1] = rprocs_val[2] = 0 ;
    open_connection_to_remote_procs(&rpmsg_fd, rprocs_val, argc, argv) ;

    int i ;
    int num_threads = 0 ;
    int size_others = 0 ;
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

    struct message message1, message2, message3, message4[2] ;
    if(rpmsg_fd.sysm3 >= 0) {
	    printf("SYSM3\n") ;
	    message1.fd = rpmsg_fd.sysm3 ;
	    message1.bufs = bufs ;
	    message1.start_indx = 0 ;
	    message1.end_indx = rprocs_val[0] ;
	    printf("Calling multiply from %d to %d\n", message1.start_indx, message1.end_indx) ;
	    int ret1 = pthread_create( &thread1, NULL, remote_compute_m3, (void*) &message1);
    }

    if(rpmsg_fd.appm3 >= 0) {
	    printf("APPM3\n") ;
	    message2.fd = rpmsg_fd.appm3 ;
	    message2.bufs = bufs ;
	    int ret2 = pthread_create( &thread2, NULL, remote_compute_m3, (void*) &message2);
    }

    if(rpmsg_fd.dsp >= 0) {
	    printf("DSP\n") ;
	    message3.fd = rpmsg_fd.dsp ;
	    message3.bufs = bufs ;
	    message3.start_indx = rprocs_val[0]  ;
	    message3.end_indx = (rprocs_val[0] + rprocs_val[2]) ;
	    printf("Calling multiply from %d to %d\n", message3.start_indx, message3.end_indx) ;
	    int ret3 = pthread_create( &thread3, NULL, remote_compute_dsp, (void*) &message3);
    }

    if(num_threads) {
	    if(size_others == 0) {
		    size_others = rprocs_val[0] + rprocs_val[2] ;
	    }
	    int remaining_size=SIZE - size_others ;
	    int previous_proc_stop_point = size_others ;

    	    pthread_t thread[num_threads] ;
            int ret4[num_threads] ;
	    int per_thread_load = remaining_size/num_threads ;
	    printf("Spawning %d thread(s) on the A9\n", num_threads) ;
    	    gettimeofday(&tv_a9_start, &tz) ;
	    int j ;
	    for(j=0 ; j<num_threads ; j++) {
		    message4[j].fd = j ;
		    message4[j].bufs = bufs ;
		    message4[j].start_indx = previous_proc_stop_point ;
		    message4[j].end_indx = previous_proc_stop_point + per_thread_load ;
		    previous_proc_stop_point = message4[j].end_indx ;
		    printf("Calling multiply from %d to %d\n", message4[j].start_indx, message4[j].end_indx) ;
		    ret4[j] = pthread_create( &thread[j], NULL, local_compute, (void*) &message4[j]);
	    }

	    for(j=0 ; j<num_threads ; j++) {
		    pthread_join(thread[j], NULL) ;
	    }
    	    gettimeofday(&tv_a9_end, &tz) ;
    }

    if(rpmsg_fd.sysm3 >= 0) { pthread_join( thread1, NULL) ; }
    if(rpmsg_fd.appm3 >= 0) { pthread_join( thread2, NULL) ; }
    if(rpmsg_fd.dsp >= 0) { pthread_join( thread3, NULL) ; }

    if(rpmsg_fd.sysm3 >= 0) { detach_buffers(bufs, rpmsg_fd.sysm3, cdev_fd) ; }
    if(rpmsg_fd.appm3 >= 0) { detach_buffers(bufs, rpmsg_fd.appm3, cdev_fd) ; }
    if(rpmsg_fd.dsp >= 0) { detach_buffers(bufs, rpmsg_fd.dsp, cdev_fd) ; }

    close_connection_to_remote_procs(&rpmsg_fd) ;

    
    //if(verify_result(bufs)) {
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

    gettimeofday(&tv_end, &tz) ;

    print_time_summary() ;
    return 0;
}
