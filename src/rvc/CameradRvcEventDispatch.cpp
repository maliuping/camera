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
#include "log/log.h"
#include "ncore/NCLog.h"
#include "rvc/CameradRvcEventDispatch.h"
#include "rvc/CameradRvcMessageHandler.h"

#undef LOG_TAG
#define LOG_TAG "SYS_HWH_CAMERA_RVC"

namespace nutshell {
/*********************************************************************************
    classname:  CameradRvcEventDispatch
    funcname:   CameradRvcEventDispatch
    contents:   CameradRvcEventDispatch constructor
    parameter:  NHuComEventDispatch_TYPE type
    return:     none
    note:
*********************************************************************************/
CameradRvcEventDispatch::CameradRvcEventDispatch(NHRtapEventDispatch_TYPE type)
    : NHRtapEventDispatch(type)
    , m_handler(NULL) {
    ALOGD("CameradRvcEventDispatch constructor");
}

/*********************************************************************************
    classname:  CameradRvcEventDispatch
    funcname:   ~CameradRvcEventDispatch
    contents:   CameradRvcEventDispatch distructor
    parameter:  none
    return:     none
    note:
*********************************************************************************/
CameradRvcEventDispatch::~CameradRvcEventDispatch() {
    ALOGD("CameradRvcEventDispatch distructor");
    m_handler = NULL;
}

/*********************************************************************************
    classname:  CameradRvcEventDispatch
    funcname:   setHandler
    contents:   m_handler implement method
    parameter:  sp<CameradRvcMessageHandler> handler
    return:     none
    note:
*********************************************************************************/
void
CameradRvcEventDispatch::setHandler(sp<CameradRvcMessageHandler> handler) {
    ALOGD("RvcEventDispatch::setHandler");
    m_handler = handler;
}

/*********************************************************************************
    classname:  CameradRvcEventDispatch
    funcname:   onReceiveRvcDatafromRtap
    contents:   receive data from Rtap
    parameter:  const NHRtapInfoRcvData &rtapData
    return:     none
    note:
*********************************************************************************/
void
CameradRvcEventDispatch::onReceiveRvcDatafromRtap(const NHRtapInfoRcvData &rtapData) {
    if (m_handler != NULL) {
        m_handler->onReceiveRvcDatafromRtap(rtapData);
    } else {
        ALOGE("CameradRvcEventDispatch::onReceiveRvcDatafromRtap m_handler is NULL");
    }
}

}  // namespace nutshell
