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
/*********************************************************************************
                                include files
*********************************************************************************/
#include "cutils/log.h"
#include "rvc/CameradRvcMessageHandler.h"
#include "rvc/CameradRvcImpl.h"

#undef LOG_TAG
#define LOG_TAG "SYS_HWH_CAMERA_RVC"

namespace nutshell {
/*********************************************************************************
    structname:  CameradRvcImpl
    funcname:   CameradRvcImpl
    contents:   CameradRvcImpl constructor
    parameter:  none
    return:     none
*********************************************************************************/
CameradRvcImpl::CameradRvcImpl()
    : m_handler(NULL) {
    ALOGD("CameradRvcImpl constructor");
    m_handler = new CameradRvcMessageHandler();
    if (m_handler == NULL) {
        ALOGE("CameradRvcImpl m_handler is NULL!");
    }
}

/*********************************************************************************
    structname:  CameradRvcImpl
    funcname:   ~CameradRvcImpl
    contents:   CameradRvcImpl distructor
    parameter:  none
    return:     none
*********************************************************************************/
CameradRvcImpl::~CameradRvcImpl() {
    if (m_handler != NULL) {
        m_handler = NULL;
    }
}

/*********************************************************************************
    classname:  CameradRvcImpl
    funcname:   onAwake
    contents:   RVC service awake operation
    parameter:  none
    return:     none
*********************************************************************************/
void
CameradRvcImpl::onAwake() {
    ALOGE("CameradRvcImpl onAwake!");
    if (m_handler != NULL) {
        m_handler->onAwake();
    }
}

/********************************************************************************************************************
 * HIDL Interface Methods from camerad follow
 * ********************************************************************************************************************/
/*********************************************************************************
    structname:  CameradRvcImpl
    funcname:   registerListener
    contents:   implement method of camerad RegisterListener
    parameter:  const sp<IRvcListener>& listener, EnRvcListenerID serviceId, uint64_t eventIds
    return:     Return<EnRvcFuncResult>
*********************************************************************************/
Return<EnRvcFuncResult>
CameradRvcImpl::registerListener(const sp<IRvcListener>& listener, EnCameradRvcListenerID serviceId, uint64_t eventIds) {
    EnRvcFuncResult funcResult = EnRvcFuncResult::RVC_RESULT_OK;

    if (m_handler != NULL) {
        ALOGD("CameradRvcImpl::registerListener() serviceId[%d] HIDL OK", static_cast<uint8_t>(serviceId));
        funcResult = m_handler->registerListener(listener, serviceId, eventIds);
    } else {
        ALOGE("CameradRvcImpl::registerListener() serviceId[%d] HIDL NG!!", static_cast<uint8_t>(serviceId));
        funcResult = EnRvcFuncResult::RVC_RESULT_ERR;
    }

    return funcResult;
}

/*********************************************************************************
    structname:  CameradRvcImpl
    funcname:   unRegisterListener
    contents:   implement method of camerad unregisterListener
    parameter:  EnRvcListenerID serviceId
    return:     Return<EnRvcFuncResult>
*********************************************************************************/
Return<EnRvcFuncResult>
CameradRvcImpl::unRegisterListener(EnCameradRvcListenerID serviceId) {
    EnRvcFuncResult funcResult = EnRvcFuncResult::RVC_RESULT_OK;

    if (m_handler != NULL) {
        ALOGD("CameradRvcImpl::unRegisterListener() serviceId[%d] HIDL OK", static_cast<uint8_t>(serviceId));
        funcResult = m_handler->unRegisterListener(serviceId);
    } else {
        ALOGE("CameradRvcImpl::unRegisterListener() serviceId[%d] HIDL NG!!", static_cast<uint8_t>(serviceId));
        funcResult = EnRvcFuncResult::RVC_RESULT_ERR;
    }

    return funcResult;
}

/*********************************************************************************
    structname: CameradRvcImpl
    funcname:   configCameraItem
    contents:   implement method of camerad SendDataToRtap
    parameter:  EnNotifyGuideLineStatus dispStatus
    return:     Return<EnRvcFuncResult>
*********************************************************************************/
Return<EnRvcFuncResult>
CameradRvcImpl::configCameraItem(const EnNotifyGuideLineStatus dispStatus) {
    EnRvcFuncResult funcResult = EnRvcFuncResult::RVC_RESULT_OK;

    if (m_handler != NULL) {
        ALOGD("CameradRvcImpl::configCameraItem() HIDL OK;data number :%hhx", dispStatus);
        // funcResult = m_handler->configCameraItem(dispStatus);
    } else {
        ALOGE("CameradRvcImpl::configCameraItem() HIDL NG!!");
        funcResult = EnRvcFuncResult::RVC_RESULT_ERR;
    }

    return funcResult;
}

}  // namespace nutshell
