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
 * @file NHCameraReceiveThread.cpp
 * @brief Implementation file of class NHCameraReceiveThread.
 */

#include <stdlib.h>
#include <stdio.h>

#include "ncore/NCLog.h"
#include "log/log.h"
#include "cutils/log.h"


#undef LOG_TAG
#define LOG_TAG "SYS_HWH_CAMERA_NHCAMERA"

namespace nutshell {
/**
 * @brief Constructor.
 */
NHCameraReceiveThread::NHCameraReceiveThread()
    : NCThread() {
}

/**
 * @brief Destructor.
 */
NHCameraReceiveThread::~NHCameraReceiveThread() {
}

/**
 * @brief Thread start function.
 */
VOID
NHCameraReceiveThread::run() {

    // display guideline
    while (NC_TRUE) {
        // // checkQuit
        // if (FALSE != checkQuit()) {
        //     break;
        // }
        // // receive data
        // if (NC_FALSE == ReceiveData()) {
        //     break;
        // }
    }
}

/**
 * @brief display.
 */
NC_BOOL
NHCameraReceiveThread::DisplayGuideLine() {

}

/**
 * @brief Read data
 */
NC_BOOL
NHCameraReceiveThread::read(BYTE* data, UINT32 dataNum, INT* retNum) {
    NC_BOOL ret = NC_FALSE;
    if ((NULL == data) || (NULL == retNum) || (NULL == m_uCameraDevice)) {
        ALOGE("NHCameraReceiveThread::read invalid parameter!");
        return ret;
    } else {
        // poll, block read
        ret = m_uCameraDevice->read(data, dataNum, retNum);
        return ret;
    }
}

}  // namespace nutshell
/* EOF */
