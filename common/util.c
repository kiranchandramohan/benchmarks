/*
 * Copyright (C) 2011 Texas Instruments
 * Author: Rob Clark <rob.clark@linaro.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "util.h"

#include <drm.h>
#include <xf86drmMode.h>

struct connector {
	uint32_t id;
	char mode_str[64];
	drmModeModeInfo *mode;
	drmModeEncoder *encoder;
	int crtc;
	int pipe;
};

#define to_display_kms(x) container_of(x, struct display_kms, base)
struct display_kms {
	struct display base;

	uint32_t connectors_count;
	struct connector connector[10];
	drmModePlane *ovr[10];

	int scheduled_flips, completed_flips;
	uint32_t bo_flags;
	drmModeResPtr resources;
	drmModePlaneRes *plane_resources;
	struct buffer *current;
};

#define to_buffer_kms(x) container_of(x, struct buffer_kms, base)
struct buffer_kms {
	struct buffer base;
	uint32_t fb_id;
};

static void alloc_buffer(struct display *disp, struct buffer* buf)
{
	uint32_t bo_flags ;
	int i ;

	bo_flags = (to_display_kms(disp))->bo_flags ;
	bo_flags |= buf->cache_type ;
	
	int size = buf->width * buf->height * 4 ;
	#ifdef DEBUG
	printf("Asking for memory(width=%d,height=%d) of size=%d\n", buf->width, buf->height, size) ;
	#endif

	for(i=0 ; i<buf->nbo ; i++) {
		buf->bo[i] = omap_bo_new(disp->dev, size, bo_flags);
		if (buf->bo[i]) {
			buf->pitches[2] = buf->width * 4 ;
		} else {
			perror("bo allocation failed\n") ;
		}
	}
}

void alloc_buffers(struct display *disp, struct buffer* bufs, uint32_t n)
{
	uint32_t i ;
	for (i = 0; i < n; i++) {
		alloc_buffer(disp, &bufs[i]);
	}
}

struct display* disp_kms_open(int argc, char **argv)
{
	struct display_kms *disp_kms = NULL;
	struct display *disp;

	disp_kms = calloc(1, sizeof(*disp_kms));
	if (!disp_kms) {
		ERROR("allocation failed");
		goto fail;
	}
	disp = &disp_kms->base;

	disp->fd = drmOpen("omapdrm", NULL);
	if (disp->fd < 0) {
		ERROR("could not open drm device: %s (%d)", strerror(errno), errno);
		goto fail;
	}

	disp->dev = omap_device_new(disp->fd);
	if (!disp->dev) {
		ERROR("couldn't create device");
		goto fail;
	}

	disp_kms->resources = drmModeGetResources(disp->fd);
	if (!disp_kms->resources) {
		ERROR("drmModeGetResources failed: %s", strerror(errno));
		goto fail;
	}

	disp_kms->plane_resources = drmModeGetPlaneResources(disp->fd);
	if (!disp_kms->plane_resources) {
		ERROR("drmModeGetPlaneResources failed: %s", strerror(errno));
		goto fail;
	}

	disp_kms->bo_flags |= OMAP_BO_SCANOUT;

	disp->width = 0;
	disp->height = 0;
	#ifdef DEBUG
	printf("Opened KMS display = %p\n", disp) ;
	#endif

	return disp;
fail:
	// XXX cleanup
	return NULL;
}
