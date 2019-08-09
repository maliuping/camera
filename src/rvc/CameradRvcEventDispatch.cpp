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
#include "rvc/RtapdRvcEventDispatch.h"
#include "rvc/RtapdRvcMessageHandler.h"

#undef LOG_TAG
#define LOG_TAG "rvc"

namespace nutshell {
/*********************************************************************************
    classname:  RtapdRvcEventDispatch
    funcname:   RtapdRvcEventDispatch
    contents:   RtapdRvcEventDispatch constructor
    parameter:  NHuComEventDispatch_TYPE type
    return:     none
    note:
*********************************************************************************/
RtapdRvcEventDispatch::RtapdRvcEventDispatch(NHRtapEventDispatch_TYPE type)
    : NHRtapEventDispatch(type)
    , m_handler(NULL) {
    ALOGD("RtapdRvcEventDispatch constructor");
}

/*********************************************************************************
    classname:  RtapdRvcEventDispatch
    funcname:   ~RtapdRvcEventDispatch
    contents:   RtapdRvcEventDispatch distructor
    parameter:  none
    return:     none
    note:
*********************************************************************************/
RtapdRvcEventDispatch::~RtapdRvcEventDispatch() {
    ALOGD("RtapdRvcEventDispatch distructor");
    m_handler = NULL;
}

/*********************************************************************************
    classname:  RtapdRvcEventDispatch
    funcname:   setHandler
    contents:   m_handler implement method
    parameter:  sp<RtapdRvcMessageHandler> handler
    return:     none
    note:
*********************************************************************************/
void
RtapdRvcEventDispatch::setHandler(sp<RtapdRvcMessageHandler> handler) {
    ALOGD("RvcEventDispatch::setHandler");
    m_handler = handler;
}

/*********************************************************************************
    classname:  RtapdRvcEventDispatch
    funcname:   onReceiveRvcDatafromRtap
    contents:   receive data from Rtap
    parameter:  const NHRtapInfoRcvData &rtapData
    return:     none
    note:
*********************************************************************************/
void
RtapdRvcEventDispatch::onReceiveRvcDatafromRtap(const NHRtapInfoRcvData &rtapData) {
    if (m_handler != NULL) {
        m_handler->onReceiveRvcDatafromRtap(rtapData);
    } else {
        ALOGE("RtapdRvcEventDispatch::onReceiveRvcDatafromRtap m_handler is NULL");
    }
}

}  // namespace nutshell
