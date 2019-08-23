/*
 * Copyright (c) 2016 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>
#include "drm_display.h"
#include "osal.h"

static const char * const mtk_crtc_prop_names[] = {
    [WDRM_CRTC_MODE_ID] = "MODE_ID",
    [WDRM_CRTC_ACTIVE] = "ACTIVE",
    [WDRM_CRTC_BACKGROUND] = "background",
};
static const char * const mtk_connector_prop_names[] = {
    [WDRM_CONNECTOR_CRTC_ID] = "CRTC_ID",
};

static const char * const mtk_plane_prop_names[] = {
    [WDRM_PLANE_TYPE] = "type",
    [WDRM_PLANE_SRC_X] = "SRC_X",
    [WDRM_PLANE_SRC_Y] = "SRC_Y",
    [WDRM_PLANE_SRC_W] = "SRC_W",
    [WDRM_PLANE_SRC_H] = "SRC_H",
    [WDRM_PLANE_CRTC_X] = "CRTC_X",
    [WDRM_PLANE_CRTC_Y] = "CRTC_Y",
    [WDRM_PLANE_CRTC_W] = "CRTC_W",
    [WDRM_PLANE_CRTC_H] = "CRTC_H",
    [WDRM_PLANE_FB_ID] = "FB_ID",
    [WDRM_PLANE_CRTC_ID] = "CRTC_ID",
    [WDRM_PLANE_ALPHA] = "alpha",
    [WDRM_PLANE_COLORKEY] = "colorkey",
};

static int drm_connector_get_mode(int fd, struct mtk_screen *screen,
	drmModeConnector *connector, char * mode)
{
    int32_t width = 0;
    int32_t height = 0;
    int i, ret;
    int preferred_flag = 0;
    int configured_flag = 0;
    drmModeModeInfoPtr preferred = NULL;
    drmModeModeInfoPtr configured = NULL;
    drmModeModeInfoPtr best = NULL;
    drmModeModeInfoPtr crtc_mode = NULL;

    if (!connector || !connector->count_modes)
        return -1;

    if((mode != NULL) && (sscanf(mode, "%dx%d", &width, &height) == 2))
        configured_flag = 1;
    else
        preferred_flag = 1;

    for (i = 0; i < connector->count_modes; i++) {
        if (width == connector->modes[i].hdisplay &&
            height == connector->modes[i].vdisplay &&
            NULL == configured)
            configured = &connector->modes[i];

        if (connector->modes[i].type & DRM_MODE_TYPE_PREFERRED)
            preferred = &connector->modes[i];

        best =  &connector->modes[i];
    }

    if ((configured_flag) && (NULL != configured))
        crtc_mode = configured;
    else if ((preferred_flag) && (NULL != preferred))
        crtc_mode = preferred;
    else
        crtc_mode = best;

    ret = drmModeCreatePropertyBlob(fd, crtc_mode, sizeof(*crtc_mode), &screen->blob_id);

    if (ret < 0) {
        LOG_ERR( "failed to create property blob ");
        return -1;
    }

    return 0;
}

int drm_alloc_gem(int fd, int width, int height, int fourcc, struct raw_texture * raw_data)
{
    void *map = NULL;
    struct drm_mode_create_dumb create_arg;
    struct drm_mode_map_dumb map_arg;
    struct drm_prime_handle prime_arg;
    int i, ret;
    unsigned int alloc_size;

    memset(&create_arg, 0, sizeof(create_arg));
    memset(&map_arg, 0, sizeof(map_arg));
    memset(&prime_arg, 0, sizeof(prime_arg));

    raw_data->width = width;
    raw_data->height = height;
    raw_data->fourcc = fourcc;

    for(i = 0; i < MTK_MAX_PLANE; i ++ )
        raw_data->fds[i] = -1;

    switch (raw_data->fourcc) {
    case DRM_FORMAT_NV12:
        raw_data->bpp = 12;
        raw_data->plane_nums = 2;
        break;

    case DRM_FORMAT_NV16:
        raw_data->bpp = 16;
        raw_data->plane_nums = 2;
        break;

    case DRM_FORMAT_RGB565:
    case DRM_FORMAT_YUYV:
    case DRM_FORMAT_UYVY:
        raw_data->bpp = 16;
        raw_data->plane_nums = 1;
        break;

    case DRM_FORMAT_ARGB8888:
    case DRM_FORMAT_XRGB8888:
        raw_data->bpp = 32;
        raw_data->plane_nums = 1;
        break;
    case DRM_FORMAT_RGB888:
    case DRM_FORMAT_BGR888:
        raw_data->bpp = 24;
        raw_data->plane_nums = 1;
        break;
    case DRM_FORMAT_YUV420:
    case DRM_FORMAT_YVU420:
        raw_data->bpp = 12;
        raw_data->plane_nums = 3;
        break;

    default:
        fprintf(stderr, "unsupported format 0x%08x\n",  raw_data->fourcc);
        return -1;
    }

    if (raw_data->plane_nums == 3) {
        if (raw_data->bpp == 12) {
            raw_data->pitch[0] = DRM_ALIGN(raw_data->width, 16);
            raw_data->pitch[1] = raw_data->pitch[0] / 2;
            raw_data->pitch[2] = raw_data->pitch[0] / 2;
            raw_data->offset[0] = 0;
            raw_data->offset[1] = raw_data->pitch[0] * raw_data->height;
            raw_data->offset[2] = raw_data->offset[1] + raw_data->pitch[1] * raw_data->height / 2;
            alloc_size = raw_data->offset[2] + raw_data->pitch[2] * raw_data->height / 2;
        } else {
            fprintf(stderr,"debug: please add new format 0x%x\n", raw_data->fourcc);
            return -1;
        }
    } else if (raw_data->plane_nums == 2) {
        raw_data->pitch[0] = DRM_ALIGN(raw_data->width, 16);
        raw_data->offset[0] = 0;
        if (raw_data->bpp == 16) {
            raw_data->pitch[1] = raw_data->pitch[0];
            raw_data->offset[1] = raw_data->pitch[0] * raw_data->height;
            alloc_size = raw_data->offset[1] + raw_data->pitch[1] * raw_data->height;
            fprintf(stderr,"debug:  %s %d alloc_size = %d o/p [%d %d]\n",
                __FUNCTION__, __LINE__, alloc_size, raw_data->offset[1], raw_data->pitch[1]);
        }
        else if (raw_data->bpp == 12) {
            raw_data->pitch[1] = raw_data->pitch[0];
            raw_data->offset[1] = raw_data->pitch[0] * raw_data->height;
            alloc_size = raw_data->offset[1] + raw_data->pitch[1] * raw_data->height;
        } else {
            fprintf(stderr,"debug: please add new format 0x%x\n", raw_data->fourcc);
            return -1;
        }
    } else {
        raw_data->pitch[0] = DRM_ALIGN(raw_data->width * raw_data->bpp / 8, 16);
        raw_data->offset[0] = 0;
        alloc_size = raw_data->pitch[0] * raw_data->height;
    }

    create_arg.bpp = 8;
    create_arg.width = alloc_size;
    create_arg.height = 1;

    ret = drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_arg);
    if (ret) {
            LOG_ERR("error: drmIoctl %d DRM_IOCTL_MODE_CREATE_DUMB fail %d", fd, ret);
            return -1;
    }

    map_arg.handle = create_arg.handle;

    ret = drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &map_arg);
    if (ret) {
            LOG_ERR("error: drmIoctl DRM_IOCTL_MODE_MAP_DUMB fail %d", ret);
            return -1;
    }

    map = mmap(0, create_arg.size, PROT_WRITE|PROT_READ , MAP_SHARED, fd, map_arg.offset);
    if (map == MAP_FAILED) {
            LOG_ERR("error: mmap fail :%p", map);
            return -1;
    }

    prime_arg.handle = create_arg.handle;
    prime_arg.flags = DRM_CLOEXEC;
    ret = drmIoctl(fd, DRM_IOCTL_PRIME_HANDLE_TO_FD, &prime_arg);
    if (ret || prime_arg.fd == -1) {
            LOG_ERR("error: drmIoctl DRM_IOCTL_PRIME_HANDLE_TO_FD fail %d fd=%d",
                    ret,prime_arg.fd);
            return -1;
    }

    for (i = 0; i < raw_data->plane_nums; i++) {
        raw_data->fds[i] = prime_arg.fd;
        raw_data->handle[i] = create_arg.handle;
    }
    raw_data->texbuf = map;
    raw_data->size = create_arg.size;

    LOG_INFO( "alloc drm dma buffer %x size %d", map, create_arg.size);

    return 0;
}

int drm_free_gem(int fd, struct raw_texture * raw_data)
{
    struct drm_mode_destroy_dumb arg;
    int ret = 0;

    if (raw_data->texbuf){
        LOG_INFO( "free drm dma buffer %x size %d", raw_data->texbuf, raw_data->size);
        munmap(raw_data->texbuf, raw_data->size);
        raw_data->texbuf = NULL;

	close(raw_data->fds[0]);
        memset(&arg, 0, sizeof(arg));
        arg.handle = raw_data->handle[0];

        ret = drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &arg);
        if (ret)
            LOG_ERR( "failed to destroy dumb buffer: %s",
                strerror(errno));
    }

    return ret;
}

int drm_buffer_release(int fd, struct raw_texture *raw_data)
{
    int ret = 0;

    if (raw_data->fb_id){
        drmModeRmFB(fd, raw_data->fb_id);
        raw_data->fb_id = 0;
    }

    return ret;
}

int drm_buffer_prepare(int fd, struct raw_texture *raw_data)
{
    int ret = 0;

    if (drmModeAddFB2(fd, raw_data->width, raw_data->height,
                            raw_data->fourcc, raw_data->handle,
                            raw_data->pitch, raw_data->offset,
                            &raw_data->fb_id, 0)) {
        LOG_ERR( "failed to add raw_data fb: %s", strerror(errno));
        return -1;
    }
    return ret;
}

static int drm_find_crtc_for_connector(struct mtk_display * disp,
            drmModeRes *resources, drmModeConnector *connector)
{
	drmModeEncoder *encoder;
	uint32_t possible_crtcs;
	int i, j;

	for (j = 0; j < connector->count_encoders; j++) {
		encoder = drmModeGetEncoder(disp->fd, connector->encoders[j]);
		if (encoder == NULL) {
			LOG_ERR( "Failed to get encoder.");
			return -1;
		}
		possible_crtcs = encoder->possible_crtcs;
		drmModeFreeEncoder(encoder);

		for (i = 0; i < resources->count_crtcs; i++) {
			if (possible_crtcs & (1 << i))
			    return resources->crtcs[i];
		}
	}

	return -1;
}

static int drm_find_plane_screen_idx(struct mtk_display * disp,
	drmModeRes *resources, drmModePlane *plane)
{
	uint32_t possible_crtcs = plane->possible_crtcs;
	int i, j, crtc_id;

	for (i = 0; i < resources->count_crtcs; i++){
		crtc_id = -1;
		if (possible_crtcs & (1 << i))
		    crtc_id = resources->crtcs[i];

		for(j = 0; j < disp->screen_num; j ++){
			if(disp->screen[j].crtc_id == crtc_id)
				return j;
		}
	}

	return -1;
}

static int drm_get_resource(struct mtk_display * disp)
{
	drmModeConnector *connector;
	drmModeRes *resources;
	drmModePropertyPtr property;
	drmModePlaneRes *plane_res;
	drmModePlane *plane;
	drmModeObjectProperties *props;
	int i, j, k, src_idx;
	struct mtk_screen *screen = NULL;
	char *screen_mode = NULL;
	struct mtk_plane * plane_info = NULL;

	disp->screen_num = 0;

	resources = drmModeGetResources(disp->fd);
	if (!resources) {
		LOG_ERR( "drmModeGetResources failed");
		return -1;
	}

	for (i = 0; i < resources->count_connectors; i++) {
		connector = drmModeGetConnector(disp->fd,
			resources->connectors[i]);
		if (connector == NULL)
			continue;

		if (connector->connection == DRM_MODE_CONNECTED){
			screen = &disp->screen[disp->screen_num];
			screen_mode = disp->screen_mode[disp->screen_num];
			screen->crtc_id = drm_find_crtc_for_connector(disp, resources, connector);
			if (screen->crtc_id < 0) {
				LOG_ERR( "No usable crtc/encoder pair for connector.");
				return -1;
			}

			if (drm_connector_get_mode(disp->fd, screen, connector, screen_mode) < 0){
				LOG_ERR( "Connector get mode error");
				return -1;
			}

			screen->con_id = connector->connector_id;
			for (j = 0; j < connector->count_props; j++) {
				property = drmModeGetProperty(disp->fd, connector->props[j]);
				if (!property)
					continue;
				for(k = 0; k < WDRM_CONNECTOR__COUNT; k ++) {
					if (0 == strcmp(property->name, mtk_connector_prop_names[k])) {
						screen->con_prop_id[k] = property->prop_id;
						LOG_DBG( "connector prop name %s prop id %d",
							property->name, property->prop_id);
						break;
					}
				}
				drmModeFreeProperty(property);
			}

			if(screen->crtc_id > 0) {
				props = drmModeObjectGetProperties(disp->fd,
					screen->crtc_id, DRM_MODE_OBJECT_CRTC);
				for (j = 0; j < props->count_props; j++) {
					property = drmModeGetProperty(disp->fd, props->props[j]);
					if (!property)
						continue;
					for(k = 0; k < WDRM_CRTC__COUNT; k ++) {
						if (0 == strcmp(property->name, mtk_crtc_prop_names[k])) {
							screen->crtc_prop_id[k] = property->prop_id;
							LOG_DBG( "crtc %d prop name %s prop id %d",
								screen->crtc_id, property->name, property->prop_id);
							break;
						}
					}
					drmModeFreeProperty(property);
				}
				drmModeFreeObjectProperties(props);
			}

			disp->screen_num++;
		}

		drmModeFreeConnector(connector);
		if(disp->screen_num == MTK_MAX_SCREEN)
			break;
	}

	plane_res = drmModeGetPlaneResources(disp->fd);
	if (!plane_res) {
		LOG_ERR( "failed to get plane resources: %s",
			strerror(errno));
		return -1;
	}

	LOG_DBG("plane num %d ", plane_res->count_planes);

	for (i = 0; i < plane_res->count_planes; i++) {
		plane = drmModeGetPlane(disp->fd, plane_res->planes[i]);
		if (!plane)
			continue;

		src_idx = drm_find_plane_screen_idx(disp, resources, plane);
		if(src_idx < 0)
			continue;

		screen = &disp->screen[src_idx];
		plane_info = &screen->plane[screen->plane_num];

		LOG_DBG( "creating new plane %d on screen idx %d plane_num %d",
			plane_res->planes[i], src_idx, screen->plane_num);

		props = drmModeObjectGetProperties(disp->fd,
			plane->plane_id, DRM_MODE_OBJECT_PLANE);
		for (j = 0; j < props->count_props; j++) {
			property = drmModeGetProperty(disp->fd, props->props[j]);
			if (!property)
				continue;
			for(k = 0; k < WDRM_PLANE__COUNT; k ++) {
				if (0 == strcmp(property->name, mtk_plane_prop_names[k])) {
					plane_info->prop_id[k] = property->prop_id;
					LOG_DBG( "plane %d prop name %s prop id %d",
						plane->plane_id, property->name, property->prop_id);
					break;
				}
			}
			drmModeFreeProperty(property);
		}
		LOG_DBG( "plane id %d info OK", plane->plane_id);
		plane_info->plane_id = plane->plane_id;
		screen->plane_num ++;
		drmModeFreeObjectProperties(props);
		drmModeFreePlane(plane);
	}

	drmModeFreePlaneResources(plane_res);
	drmModeFreeResources(resources);
	return 0;
}

int drm_init(struct mtk_display * disp)
{
	int ret = 0;
	int fd = -1;
	fd = drmOpen(MTK_DRM_DRV_NAME, NULL);

	if (fd < 0) {
		LOG_ERR("failed to open device.");
		return -1;
	}

	LOG_INFO("Open disp->fd = %d", fd);
	ret = drmSetClientCap(fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
	if (ret) {
		LOG_ERR("driver doesn't support universal planes setting");
		return -1;
	}

	LOG_INFO("test info");

	ret = drmSetClientCap(fd, DRM_CLIENT_CAP_ATOMIC, 1);
	if (ret) {
		LOG_ERR("driver doesn't support atomic display setting");
		return -1;
	}

	LOG_INFO("test info");

	ret = drmDropMaster(fd);
	if (ret)
		LOG_DBG("no master display user");

	LOG_INFO("test info");

	disp->fd = fd;

	ret = drm_get_resource(disp);
	if (ret) {
		disp->fd = -1;
		LOG_ERR("driver doesn't support atomic display setting");
		return -1;
	}

	LOG_INFO("test info");

	return 0;
}

void drm_deinit(struct mtk_display * disp)
{
	drmClose(disp->fd);
	return;
}

int drm_add_mode_set(struct mtk_display * disp, int screen_idx, int enable)
{
	struct mtk_screen *screen = &disp->screen[screen_idx];
	struct mtk_plane *plane_info = NULL;

	LOG_DBG( "screen %d crtc_id %d con_id %d ", screen_idx, screen->crtc_id, screen->con_id);

	if(disp->req == NULL)
		disp->req = drmModeAtomicAlloc();

	if (!disp->req) {
		return -1;
	}

	drmModeAtomicAddProperty(disp->req, screen->con_id,
		screen->con_prop_id[WDRM_CONNECTOR_CRTC_ID],
		enable ? screen->crtc_id : 0);

	drmModeAtomicAddProperty(disp->req, screen->crtc_id,
		screen->crtc_prop_id[WDRM_CRTC_MODE_ID],
		enable ? screen->blob_id : 0);

	drmModeAtomicAddProperty(disp->req, screen->crtc_id,
		screen->crtc_prop_id[WDRM_CRTC_ACTIVE], enable);

	disp->flags |= DRM_MODE_ATOMIC_ALLOW_MODESET;

	return 0;
}

int drm_add_plane_set(struct mtk_display * disp,
	struct raw_texture * raw_data, int x, int y, int screen_idx, int plane_idx)
{
	struct mtk_screen *screen = &disp->screen[screen_idx];
	struct mtk_plane *plane_info = &screen->plane[plane_idx];

	if(disp->req == NULL){
		disp->req = drmModeAtomicAlloc();
		LOG_DBG(" allocate new req %p ", disp->req);
	}

	if (!disp->req) {
		return -1;
	}
	
	LOG_DBG("set fb_id %d to plane %d crtc %d",
		raw_data != NULL ? raw_data->fb_id : 0, plane_info->plane_id, screen->crtc_id);

	drmModeAtomicAddProperty(disp->req, plane_info->plane_id,
		plane_info->prop_id[WDRM_PLANE_FB_ID],
		raw_data != NULL ? raw_data->fb_id : 0);

	drmModeAtomicAddProperty(disp->req, plane_info->plane_id,
		plane_info->prop_id[WDRM_PLANE_CRTC_ID],
		raw_data != NULL ? screen->crtc_id : 0);

	drmModeAtomicAddProperty(disp->req, plane_info->plane_id,
		plane_info->prop_id[WDRM_PLANE_SRC_X], 0);

	drmModeAtomicAddProperty(disp->req, plane_info->plane_id,
		plane_info->prop_id[WDRM_PLANE_SRC_Y], 0);

	drmModeAtomicAddProperty(disp->req, plane_info->plane_id,
		plane_info->prop_id[WDRM_PLANE_SRC_W],
		raw_data != NULL ? (raw_data->width << 16) : 0);

	drmModeAtomicAddProperty(disp->req, plane_info->plane_id,
		plane_info->prop_id[WDRM_PLANE_SRC_H],
		raw_data != NULL ? (raw_data->height << 16) : 0);

	drmModeAtomicAddProperty(disp->req, plane_info->plane_id,
		plane_info->prop_id[WDRM_PLANE_CRTC_X],
		raw_data != NULL ? x : 0);

	drmModeAtomicAddProperty(disp->req, plane_info->plane_id,
		plane_info->prop_id[WDRM_PLANE_CRTC_Y],
		raw_data != NULL ? y : 0);

	drmModeAtomicAddProperty(disp->req, plane_info->plane_id,
		plane_info->prop_id[WDRM_PLANE_CRTC_W],
		raw_data != NULL ? raw_data->width : 0);

	drmModeAtomicAddProperty(disp->req, plane_info->plane_id,
		plane_info->prop_id[WDRM_PLANE_CRTC_H],
		raw_data != NULL ? raw_data->height : 0);

	return 0;
}

int drm_display_flush(struct mtk_display * disp, int block)
{
	int ret = 0;

	if(disp->req == NULL)
		return 0;

	if(block == 0)
		disp->flags |= DRM_MODE_ATOMIC_NONBLOCK | DRM_MODE_PAGE_FLIP_EVENT;

	LOG_DBG("commit req %p flags %x start ", disp->req, disp->flags);

	ret = drmModeAtomicCommit(disp->fd, disp->req, disp->flags, disp);
	if (ret < 0)
		LOG_ERR( "atomic commit failed %s", strerror(errno));

	LOG_DBG("commit req %p flags %x  done ", disp->req, disp->flags);

	drmModeAtomicFree(disp->req);
	disp->req = NULL;
	disp->flags = 0;

	return ret;
}
