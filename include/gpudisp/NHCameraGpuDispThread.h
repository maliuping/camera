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

#include "ncore/NCTypesDefine.h"
#include "ncore/NCThread.h"

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
    virtual VOID
    run();

 private:
    // NHCameraRxServer* m_recvServer;
    // NHCameraDevice* m_uCameraDevice;         // /< uCom device pointer

 private:
    /**
     * @brief Receive data.
     *
     * @return NC_TRUE indicates success, other value indicates failure.
     *
     * @attention Asynchronous I/F.
     */
    NC_BOOL
    ReceiveData();

    /**
     * @brief Read data from device.
     *
     * @param [in] data
     *             data buffer.
     * @param [in] dataNum
     *             receive data length.
     * @param [out] retNum
     *             Actual received data length.
     *
     * @return NC_TRUE indicates success, other value indicates failure.
     *
     * @attention Asynchronous I/F.
     */
    NC_BOOL
    read(BYTE* data, UINT32 dataNum, INT* retNum);

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
};
}  // namespace nutshell
#endif  // INCLUDE_NHCAMERA_NHCameraGpuDispThread_H_
