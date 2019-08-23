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

#include "gpu_render.h"
#include "drm_display.h"
#include "osal.h"
#include "ncore/NCLog.h"
#include "ncore/NCAutoSync.h"
#include "ncore/NCThread.h"
#include "ncore/NCWaitObj.h"
#include "ncore/NCTime.h"

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
NHCameraManager::NHCameraManager() {
    if (m_UCameraManagerInstance == NULL) {
            m_UCameraManagerInstance = new NHCameraGpuDispThread();
            if (NULL == m_UCameraManagerInstance) {
                ALOGE("NHRtapManager: m_UCameraManagerInstance is NULL!");
            }
        }
}

/******************************************************************************

    Destructor

*******************************************************************************/
NHCameraManager::~NHCameraManager() {

}

/******************************************************************************

    Instance

******************************************************************************/
NHCameraManager*
NHCameraManager::Instance(VOID) {
    ALOGE("NHCameraManager::Instance");
    s_cSync.syncStart();
    if (NULL == m_UCameraManagerInstance) {
        m_UCameraManagerInstance = new NHCameraManager();
        if (NULL == m_UCameraManagerInstance) {
            ALOGE("NHCameraManager::Instance m_UCameraManagerInstance is NULL");
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
    ALOGD("NHCameraManager::onAwake");

}

}  // namespace nutshell
// ================================================================= End of File
