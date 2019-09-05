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

#include <stdlib.h>
#include <stdio.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/input-event-codes.h>
#include <unistd.h>
#include <cstring>
#include <log/log.h>

#include "ncore/NCLog.h"
#include "ncore/NCAutoSync.h"
#include "ncore/NCThread.h"
#include "ncore/NCWaitObj.h"
#include "ncore/NCTime.h"
#include "gpudisp/gpu_render.h"
#include "gpudisp/drm_display.h"
#include "gpudisp/osal.h"
#include "gpudisp/NHCameraManager.h"
#include "gpudisp/NHCameraGpuDispThread.h"
#include "base/CameradThreadName.h"

#undef LOG_TAG
#define LOG_TAG "SYS_HWH_CAMERA_NHCAMERA"

namespace nutshell {
#define NHTHREAD_PRI_HIGH  (-16)

// instance
NHCameraManager* NHCameraManager::m_UCameraManagerInstance = NULL;
NCSyncObj NHCameraManager::s_cSync;

/******************************************************************************

    Constructor

*******************************************************************************/
NHCameraManager::NHCameraManager()
    : m_uCameraGpuDisp(NULL) {
    if (m_uCameraGpuDisp == NULL) {
            m_uCameraGpuDisp = new NHCameraGpuDispThread();
            if (NULL == m_uCameraGpuDisp) {
                SLOGE("NHRtapManager: m_uCameraGpuDisp is NULL!");
            }
        }
}

/******************************************************************************

    Destructor

*******************************************************************************/
NHCameraManager::~NHCameraManager() {
    if (m_uCameraGpuDisp != NULL) {
        delete m_uCameraGpuDisp;
        m_uCameraGpuDisp = NULL;
    }
}

/******************************************************************************

    Instance

******************************************************************************/
NHCameraManager*
NHCameraManager::Instance() {
    SLOGE("NHCameraManager::Instance");
    s_cSync.syncStart();
    if (NULL == m_UCameraManagerInstance) {
        m_UCameraManagerInstance = new NHCameraManager();
        if (NULL == m_UCameraManagerInstance) {
            SLOGE("NHCameraManager::Instance m_UCameraManagerInstance is NULL");
        }
    }
    s_cSync.syncEnd();
    return m_UCameraManagerInstance;

}

/******************************************************************************

    destroy

******************************************************************************/
VOID
NHCameraManager::destroy() {
    s_cSync.syncStart();
    if (m_UCameraManagerInstance != NULL) {
        delete m_UCameraManagerInstance;
        m_UCameraManagerInstance = NULL;
    }
    s_cSync.syncEnd();
}

/******************************************************************************

    onAwake

******************************************************************************/
VOID
NHCameraManager::onAwake() {
    SLOGD("NHCameraManager::onAwake");
     // start gpudisp thread
    if (m_uCameraGpuDisp != NULL) {
        m_uCameraGpuDisp->setPriorityExt(NHTHREAD_PRI_HIGH);
        m_uCameraGpuDisp->startThread(CAMERAD_NHCAMERA_GPUDISP_THREAD);
    } else {
        ALOGE("NHCameraManager::onAwake m_uCameraGpuDisp is NULL!");
    }

}

}  // namespace nutshell
// ================================================================= End of File
