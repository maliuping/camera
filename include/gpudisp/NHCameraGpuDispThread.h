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
 * @file NHCameraGpuDispThread.h
 * @brief Declaration file of class NHCameraGpuDispThread.
 */

#ifndef INCLUDE_NHCAMERA_NHCAMERAGPUDISPTHREAD_H_
#define INCLUDE_NHCAMERA_NHCAMERAGPUDISPTHREAD_H_

#ifndef __cplusplus
#   error ERROR: This file requires C++ compilation(use a .cpp suffix)
#endif
#include <stdlib.h>
#include <stdio.h>
#include <drm_fourcc.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include "ncore/NCTypesDefine.h"
#include "ncore/NCThread.h"
#include "gpudisp/gpu_render.h"
#include "gpudisp/drm_display.h"
#include "gpudisp/osal.h"

namespace nutshell {
// class NHCameraDevice;
// class NHCameraRxServer;

/**
 * @brief Class of NHCameraGpuDispThread.
 *
 * Thread for receive data from device.
 */
class NHCameraGpuDispThread : public NCThread {
 public:
    /**
     * @brief Constructor.
     */
    NHCameraGpuDispThread();

    /**
     * @brief Destructor.
     */
    virtual ~NHCameraGpuDispThread();

    /**
     * @brief Thread start function.
     */
    virtual VOID run();


 private:
    /**
     * @brief Receive data.
     *
     * @return NC_TRUE indicates success, other value indicates failure.
     *
     * @attention Asynchronous I/F.
     */
    NC_BOOL
    DisplayGuideLine();

    /**
     * @brief display_set_drm_mode
     *
     * @return
     *
     * @attention
     */
    void display_set_drm_mode(struct mtk_display *display, int screen_idx, int enable);


    /**
     * copy constructor(forbidden)
     *
     * @param const NHCameraGpuDispThread& src [IN]
     *
     * @return none
     */
    NHCameraGpuDispThread(const NHCameraGpuDispThread& src);

    /**
     * copy operator(forbidden)
     *
     * @param const NHCameraGpuDispThread& src [IN]
     *
     * @return const NHCameraGpuDispThread&
     */
    const NHCameraGpuDispThread& operator=(const NHCameraGpuDispThread& src);


    private:
    int fourcc;
    struct mtk_display disp;
    void * gpu_handle;
    struct raw_texture input_tex;
    void * input_gpu_tex;
    struct raw_texture output_tex[2];
    void * output_gpu_tex[2];
    REND_COORD_T coord;
	REND_COORD_T *pcoord[2];
};
}  // namespace nutshell
#endif  // INCLUDE_NHCAMERA_NHCameraGpuDispThread_H_
