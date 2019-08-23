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

#ifndef _DRM_DISPLAY_H
#define _DRM_DISPLAY_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MTK_DRM_DRV_NAME "mediatek"

#define MTK_MAX_PLANE                4

#define MTK_MAX_SCREEN               3

#define DRM_ALIGN(X,bit) ((X + bit-1) & (~(bit-1)))

/**
 * List of properties attached to DRM connectors
 */
enum wdrm_connector_property {
    WDRM_CONNECTOR_CRTC_ID = 0,
    WDRM_CONNECTOR__COUNT
};

/**
 * List of properties attached to DRM CRTCs
 */
enum wdrm_crtc_property {
    WDRM_CRTC_MODE_ID = 0,
    WDRM_CRTC_ACTIVE,
    WDRM_CRTC_BACKGROUND,
    WDRM_CRTC__COUNT
};

/**
 * List of properties attached to DRM PLANEs
 */
enum wdrm_plane_property {
    WDRM_PLANE_TYPE = 0,
    WDRM_PLANE_SRC_X,
    WDRM_PLANE_SRC_Y,
    WDRM_PLANE_SRC_W,
    WDRM_PLANE_SRC_H,
    WDRM_PLANE_CRTC_X,
    WDRM_PLANE_CRTC_Y,
    WDRM_PLANE_CRTC_W,
    WDRM_PLANE_CRTC_H,
    WDRM_PLANE_FB_ID,
    WDRM_PLANE_CRTC_ID,
    WDRM_PLANE_ALPHA,
    WDRM_PLANE_COLORKEY,
    WDRM_PLANE__COUNT
};

struct raw_texture {
    /* input */
    int width;
    int height;
    int fourcc;
    int bpp;
    int plane_nums;

    int pitch[MTK_MAX_PLANE];
    int offset[MTK_MAX_PLANE];
    int fds[MTK_MAX_PLANE];
    int handle[MTK_MAX_PLANE];

    void *texbuf;
    int size;

    int fb_id;
    void *tex_id;

    void *android_buf;
};

struct mtk_plane {
    uint32_t plane_id;
    uint32_t prop_id[WDRM_PLANE__COUNT];
};

struct mtk_screen {
    uint32_t crtc_id;
    uint32_t crtc_prop_id[WDRM_CRTC__COUNT];

    uint32_t con_id;
    uint32_t con_prop_id[WDRM_CONNECTOR__COUNT];

    uint32_t blob_id;

    struct mtk_plane plane[MTK_MAX_PLANE];
    int plane_num;
};

struct mtk_display {
    int fd;
    struct mtk_screen screen[MTK_MAX_SCREEN];
    char *screen_mode[MTK_MAX_SCREEN];
    int screen_num;

    void *req;
    uint32_t flags;
};

/* Function Statement */
#ifdef __cplusplus
extern "C"{
#endif

int drm_alloc_gem(int fd, int width, int height, int fourcc, struct raw_texture * raw_data);
int drm_free_gem(int fd, struct raw_texture * raw_data);

int drm_buffer_release(int fd, struct raw_texture *raw_data);

int drm_buffer_prepare(int fd, struct raw_texture *raw_data);

int drm_init(struct mtk_display * disp);

void drm_deinit(struct mtk_display * disp);

int drm_add_mode_set(struct mtk_display * disp, int screen_idx, int enable);

int drm_add_plane_set(struct mtk_display * disp,
	struct raw_texture * raw_data, int x, int y, int screen_idx, int plane_idx);

int drm_display_flush(struct mtk_display * disp, int block);

#ifdef __cplusplus
}
#endif

#endif
