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

#ifndef INCLUDE_NHCAMERA_NHCAMERAMANAGER_H_
#define INCLUDE_NHCAMERA_NHCAMERAMANAGER_H_

#ifndef __cplusplus
#   error ERROR: This file requires C++ compilation(use a .cpp suffix)
#endif

#include "ncore/NCTypesDefine.h"
#include "ncore/NCSyncObj.h"


namespace nutshell {

/**
 * extern class declare
*/
class NCSyncObj;
class NCWaitObj;
class NHCameraGpuDispThread;

class NHCameraManager {

    public:
    /*************************************************************************/
    /**
        Instance

        @param none

        @return camerad packer instance
    */
    /*************************************************************************/
    static NHCameraManager* Instance();


    /*************************************************************************/
    /**
        Destroy

        @param  none

        @return none
    */
    /*************************************************************************/
    static void destroy();


    /*************************************************************************/
    /**
        onAwake

        @param  none

        @retval none
    */
    /*************************************************************************/
    void onAwake();


     private:
    /*************************************************************************/
    /**
        constructor

        @param  none

        @retval none
    */
    /*************************************************************************/
    NHCameraManager();


    /*************************************************************************/
    /**
        destructor

        @param  none

        @retval none
    */
    /*************************************************************************/
    virtual ~NHCameraManager();


    /**
     * copy constructor
     *
     * forbidden
     *
     * @param const NHCameraManager& src [IN]
     *
     * @return none
     */
    NHCameraManager(const NHCameraManager& src);

    const NHCameraManager& operator=(const NHCameraManager& src);


    private:
    static NHCameraManager    *m_UCameraManagerInstance;  // !< unique instance in process
    static NCSyncObj    s_cSync;        // !< sync object
    NHCameraGpuDispThread* m_uCameraGpuDisp;

};


}

#endif