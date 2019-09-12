/**
 * Copyright @ 2019 iAUTO(Shanghai) Co., Ltd.
 * All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are NOT permitted except as agreed by
 * iAUTO(Shanghai) Co., Ltd.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */
/**
 * @file NHCameraGpuDispThread.cpp
 * @brief Implementation file of class NHCameraGpuDispThread.
 */


#include "ncore/NCLog.h"
#include "log/log.h"
#include "cutils/log.h"
#include "gpudisp/NHCameraGpuDispThread.h"


#undef LOG_TAG
#define LOG_TAG "SYS_HWH_CAMERA_NHCAMERA"

namespace nutshell {
/**
 * @brief Constructor.
 */
NHCameraGpuDispThread::NHCameraGpuDispThread()
    : NCThread() {
    fourcc = DRM_FORMAT_ARGB8888;
    memset(&disp, 0, sizeof(struct mtk_display));
    drm_init(&disp);


}

/**
 * @brief Destructor.
 */
NHCameraGpuDispThread::~NHCameraGpuDispThread() {
}

/**
 * @brief Thread start function.
 */
VOID
NHCameraGpuDispThread::run() {
    ALOGE("NHCameraGpuDispThread::run!");

    ALOGE("test info");
    usleep(200000);
    ALOGE("create gpu render handle");
    gpu_handle = gpu_render_init(disp.fd, 4);
    ALOGE("create input buffer");
    drm_alloc_gem(disp.fd, 1920, 720, fourcc, &input_tex);
    input_gpu_tex = gpu_render_get_tex(gpu_handle, &input_tex, 0);

    ALOGE("create output buffer");
    drm_alloc_gem(disp.fd, 1920, 720, fourcc, &output_tex[0]);
    output_gpu_tex[0] = gpu_render_get_tex(gpu_handle, &output_tex[0], 1);
    drm_buffer_prepare(disp.fd, &output_tex[0]);

    drm_alloc_gem(disp.fd, 1920, 720, fourcc, &output_tex[1]);
    output_gpu_tex[1] = gpu_render_get_tex(gpu_handle, &output_tex[1], 1);
    drm_buffer_prepare(disp.fd, &output_tex[1]);

    ALOGE("set display mode");
    display_set_drm_mode(&disp, 0, 1);

    memset(input_tex.texbuf, 200, input_tex.size);
    LOG_INFO("start drawing");
    int i = 0;
    // display guideline
    while (NC_TRUE) {
        coord.tex_w = input_tex.width;
        coord.tex_h = input_tex.height;
        coord.src_x = 0;
        coord.src_y = 0;
        coord.src_w = input_tex.width;
        coord.src_h = input_tex.height;
        coord.x = 0;
        coord.y = 0;
        coord.w = input_tex.width;
        coord.h = input_tex.height;
        coord.tex_type = i%5 + 1;
        pcoord[0] = &coord;
        gpu_render_2d_overlay(gpu_handle, 1920, 720,
        &input_gpu_tex, pcoord, 1, output_gpu_tex[i%2]);
        drm_add_plane_set(&disp, &output_tex[i%2], 0, 0, 0, 2);
        drm_display_flush(&disp, 0);
        sleep(1);
    }
}

/**
 * @brief display.
 */
NC_BOOL
NHCameraGpuDispThread::DisplayGuideLine() {
    SLOGI("NHCameraGpuDispThread: DisplayGuideLine !");

    return NC_TRUE;
}

VOID
NHCameraGpuDispThread::display_set_drm_mode(struct mtk_display *display, int screen_idx, int enable)
{
	drm_add_mode_set(display, screen_idx, enable);
	drm_display_flush(display, 1);
	return;
}


}  // namespace nutshell
/* EOF */
