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
#include "cutils/log.h"
#include "rvc/CameradRvcImpl.h"
#include "cameradmanager/CameradManager.h"
#include "gpudisp/NHCameraManager.h"

#undef LOG_TAG
#define LOG_TAG "SYS_HWH_CAMERA_MANAGER"

namespace nutshell {

android::sp<CameradManager> CameradManager::s_instance = NULL;

/*************************************************************************/
/**
    CameradManager instance
*/
/*************************************************************************/
CameradManager* CameradManager::instance() {
    if (s_instance == NULL) {
        s_instance = new CameradManager;
    }
    return s_instance.get();
}

/*************************************************************************/
/**
    CameradManager Constructor
*/
/*************************************************************************/
CameradManager::CameradManager()
    : m_uCameraManager(NULL)
    , m_rvcImpl(NULL) {
    m_uCameraManager = NHCameraManager::Instance();
    m_rvcImpl = new CameradRvcImpl();
}

/*************************************************************************/
/**
    CameradManager Destructor
*/
/*************************************************************************/
CameradManager::~CameradManager() {
   if (m_rvcImpl != NULL) {
        m_rvcImpl = NULL;
    }
    NHCameraManager::destroy();
}

/*************************************************************************/
/**
    CameradManager init
*/
/*************************************************************************/
void CameradManager::init() {
    if (m_uCameraManager != NULL) {
       m_uCameraManager->onAwake();
    }
    if (m_rvcImpl != NULL) {
        m_rvcImpl->onAwake();
        android::status_t cameradstatus = m_rvcImpl->registerAsService();
        if (cameradstatus == android::OK) {
          ALOGD("HIDL:rvc registerAsService success");
        } else {
          ALOGE("HIDL:rvc registerAsService fail");
        }
    }
}

}  // namespace nutshell
