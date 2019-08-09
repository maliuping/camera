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
 * @file CameradRvcImpl.h
 * @brief Declaration file of class CameradRvcImpl
 */

#ifndef INCLUDE_RVC_CAMERADRVCIMPL_H_
#define INCLUDE_RVC_CAMERADRVCIMPL_H_

#ifndef __cplusplus
#    error ERROR: This file requires C++ compilation(use a .cpp suffix)
#endif

/*********************************************************************************
                                include files
*********************************************************************************/
//#include <iauto/hardware/camerad/rvc/1.0/IRvc.h>

namespace nutshell {
/*********************************************************************************
                                Class Declaration
*********************************************************************************/
class CameradRvcMessageHandler;

/*********************************************************************************
                        using element in other namespace
*********************************************************************************/
using ::iauto::hardware::camerad::rvc::V1_0::IRvc;
using ::iauto::hardware::camerad::rvc::V1_0::IRvcListener;
using ::iauto::hardware::camerad::rvc::V1_0::EnRvcFuncResult;
using ::iauto::hardware::camerad::rvc::V1_0::EnCameradRvcListenerID;
using ::iauto::hardware::camerad::rvc::V1_0::EnNotifyGuideLineStatus;
using ::android::hardware::Return;
using ::android::sp;

/*********************************************************************************
    struct name : CameradRvcImpl
    purpose     : IRvc binder implement method
*********************************************************************************/
class CameradRvcImpl : public IRvc {
 public:
    /*************************************************************************/
    /**
       CameradRvcImpl class constructor
    */
    /*************************************************************************/
    CameradRvcImpl();

    /*************************************************************************/
    /**
        CameradRvcImpl class destructor
    */
    /*************************************************************************/
    virtual ~CameradRvcImpl();

 public:
    /*************************************************************************/
    /**
        onAwake

        @param  none

        @retval none
    */
    /*************************************************************************/
    void
    onAwake();

 public:
    /*************************************************************************/
    /**
        registerListener

        @param  listener : [IN]  listener for register
        @param  serviceId : [IN]  service ID
        @param  eventIds : [IN]  event ID

        @retval EnRvcFuncResult
    */
    /*************************************************************************/
    Return<EnRvcFuncResult>
    registerListener(const sp<IRvcListener>& listener, EnCameradRvcListenerID serviceId, uint64_t eventIds) override;

    /*************************************************************************/
    /**
        unRegisterListener

        @param  serviceId : [IN]  service ID

        @retval EnRvcFuncResult
    */
    /*************************************************************************/
    Return<EnRvcFuncResult>
    unRegisterListener(EnCameradRvcListenerID serviceId) override;

    /*************************************************************************/
    /**
        configCameraItem

        @param  dispStatus : [IN]  data for configing guideline on/off 

        @retval EnRvcFuncResult
    */
    /*************************************************************************/
    Return<EnRvcFuncResult>
    configCameraItem(const EnNotifyGuideLineStatus&  dispStatus) override;

 private:
    /*********************************************************************************
        purpose     : creat message handler class
    *********************************************************************************/
    sp<CameradRvcMessageHandler> m_handler;
};
}  // namespace nutshell

#endif  // INCLUDE_RVC_RTAPDRVCIMPL_H_
