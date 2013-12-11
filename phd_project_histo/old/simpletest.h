#ifndef SIMPLTEST_H
#define SIMPLETEST_H

#include <errno.h>
#include "util.h"
#include <pthread.h>
#include <unistd.h>

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

#define READ_IOCTL _IOR(MY_MACIG, 0, int)
#define WRITE_IOCTL _IOW(MY_MACIG, 1, int)
#define BARRIER_IOCTL _IOW(MY_MACIG, 2, struct barrier*)
#define LOCK_IOCTL _IO(MY_MACIG, 3)
#define UNLOCK_IOCTL _IO(MY_MACIG, 4)
#define DEBUG_IOCTL _IO(MY_MACIG, 5)
#define INIT_BARRIER_IOCTL _IOW(MY_MACIG, 6, int*)
#define FINALIZE_BARRIER_IOCTL _IO(MY_MACIG, 7)

struct message
{
	int fd ;
	struct buffer** bufs ;
	int start_indx ;
	int end_indx ;
	int barrier_fd ;
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
#define FXN_TEST1 (3 | 0x80000000)

struct rpmsg_fds
{
	int sysm3 ;
	int appm3 ;
	int dsp ;
} ;

#endif
