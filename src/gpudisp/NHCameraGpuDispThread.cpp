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

#include <stdlib.h>
#include <stdio.h>

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
NHCameraGpuDispThread::DisplayGuideLine() {
    SLOGI("NHCameraGpuDispThread: DisplayGuideLine !");

    return NC_TRUE;
}


}  // namespace nutshell
/* EOF */
