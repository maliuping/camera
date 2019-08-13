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
 * @file CameradRvcMessageHandler.h
 * @brief Declaration file of class CameradRvcMessageHandler
 */
#ifndef INCLUDE_RVC_CAMERADRVCMESSAGEHANDLER_H_
#define INCLUDE_RVC_CAMERADRVCMESSAGEHANDLER_H_

#ifndef __cplusplus
#    error ERROR: This file requires C++ compilation(use a .cpp suffix)
#endif


/*********************************************************************************
                                include files
*********************************************************************************/
#include <utils/RefBase.h>
#include <map>
#include "base/MessageDerive.h"
#include "servicebase/LooperThread.h"
#include "servicebase/Message.h"
#include "servicebase/MessageHandler.h"
#include "rvc/CameradRvcImpl.h"
//#include "nhrtap/NHRtapHandler_DEF.h"


namespace nutshell {
/*********************************************************************************
                                Class Declaration
*********************************************************************************/
// class CameradRvcEventDispatch;
class CameradRvcMessageHandler;
class CameradPrivateRvcMessageHandler;
class CameradRvcImpl;

/*********************************************************************************
                        using element in other namespace
*********************************************************************************/
using ::android::sp;
using ::iauto::hardware::camerad::rvc::V1_0::IRvcListener;
using ::iauto::hardware::camerad::rvc::V1_0::EnRvcEventId;
using ::iauto::hardware::camerad::rvc::V1_0::EnRvcFuncResult;
using ::iauto::hardware::camerad::rvc::V1_0::EnCameradRvcListenerID;
using ::iauto::hardware::camerad::rvc::V1_0::EnNotifyGuideLineStatus;
using ::iauto::hardware::camerad::rvc::V1_0::EnOnRvcReadyInfo;

/*********************************************************************************
                                enum definiton
*********************************************************************************/
/*Enum for Event ID*/
enum {
    Rvc_registerListener,
    Rvc_unRegisterListener,
    Rvc_receiveRtapinfo,
    Rvc_listenerServiceDiedInform,
};

/*********************************************************************************
                                struct definiton
*********************************************************************************/
/*struct of meter element in listener map*/
struct StRvcListenerElement {
    sp<IRvcListener> listener;
    uint64_t concernEventIds;
};

/*typedef register message in common service*/
typedef RegisterMessage<StRvcListenerElement, EnCameradRvcListenerID> RvcRegisterMessage;
typedef std::map<EnCameradRvcListenerID, StRvcListenerElement> CommonLisMap;


/*********************************************************************************
    class name : CameradPrivateRvcMessageHandler
    purpose    : public inherit MessageHandler, get message from loorthread
*********************************************************************************/
class CameradPrivateRvcMessageHandler : public MessageHandler {
 public:
    /*************************************************************************/
    /**
       CameradPrivateRvcMessageHandler class constructor
    */
    /*************************************************************************/
    explicit CameradPrivateRvcMessageHandler(sp<CameradRvcMessageHandler> callback);

    /*************************************************************************/
    /**
        CameradPrivateRvcMessageHandler class destructor
    */
    /*************************************************************************/
    virtual ~CameradPrivateRvcMessageHandler();

    /*************************************************************************/
    /**
        Receive Message from Loop post message

        @param  msg : [IN] message from loopthread

        @retval none
    */
    /*************************************************************************/
    virtual void
    onReceiveMessage(const android::sp<Message> &msg);

 private:
    sp<CameradRvcMessageHandler> m_callback;
    DISABLE_COPY(CameradPrivateRvcMessageHandler);
};  // EOF class CameradPrivateRvcMessageHandler

/*********************************************************************************
    class name : CameradRvcDeathRecipientHandler
    purpose    : public inherit MessageHandler, Client Death detection
*********************************************************************************/
class CameradRvcDeathRecipientHandler : public android::hardware::hidl_death_recipient {
 public:
    /*************************************************************************/
    /**
       CameradRvcDeathRecipientHandler class constructor
    */
    /*************************************************************************/
    explicit CameradRvcDeathRecipientHandler(sp<CameradRvcMessageHandler> handler);

    /*************************************************************************/
    /**
        CameradRvcDeathRecipientHandler class destructor
    */
    /*************************************************************************/
    virtual ~CameradRvcDeathRecipientHandler();

    /*************************************************************************/
    /**
        service died notify,public inherit hidl_death_recipient

        @param  cookie : [IN]  client serial number
        @param  who    : [IN]  wp point of death client

        @retval none
    */
    /*************************************************************************/
    virtual void
    serviceDied(uint64_t cookie, const android::wp<::android::hidl::base::V1_0::IBase>& who);

 private:
    sp<CameradRvcMessageHandler> m_handler;
};  // EOF class CameradRvcDeathRecipientHandler

/*********************************************************************************
    class name : CameradRvcMessageHandler
    purpose    :  Concrete realization of meterHandler
*********************************************************************************/
class CameradRvcMessageHandler : public android::RefBase {
 public:
    /*************************************************************************/
    /**
       CameradRvcMessageHandler class constructor
    */
    /*************************************************************************/
    CameradRvcMessageHandler();

    /*************************************************************************/
    /**
        CameradRvcMessageHandler class destructor
    */
    /*************************************************************************/
    virtual ~CameradRvcMessageHandler();

    /*************************************************************************/
    /**
        onAwake

        @param  none

        @retval none
    */
    /*************************************************************************/
    void
    onAwake();

    /*************************************************************************/
    /**
        onReceiveMessage

        @param  msg : [IN]  message from loopthread

        @retval none
    */
    /*************************************************************************/
    void
    onReceiveMessage(const android::sp<Message> &msg);

    /*************************************************************************/
    /**
        listenerServiceDiedInform

        @param  cookie : [IN]  post listener died to loopthread

        @retval none
    */
    /*************************************************************************/
    void
    listenerServiceDiedInform(uint64_t cookie);

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
    EnRvcFuncResult
    registerListener(const sp<IRvcListener>& listener, EnCameradRvcListenerID serviceId, uint64_t eventIds);

    /*************************************************************************/
    /**
        unRegisterListener

        @param  serviceId : [IN]  service ID

        @retval EnRvcFuncResult
    */
    /*************************************************************************/
    EnRvcFuncResult
    unRegisterListener(EnCameradRvcListenerID serviceId);

    /*************************************************************************/
    /**
        SendDataToRtap

        @param  rvcData : [IN]  data for sending to Rtap

        @retval EnRvcFuncResult
    */
    /*************************************************************************/
    // EnRvcFuncResult
    // SendDataToRtap(const StRvcToRtapCmdDataInfo& rvcData);


    /*************************************************************************/
    /**
        onRvcCameradReadyinfo

        @param  rtapData : [IN]  data for sending to RVC service

        @retval none
    */
    /*************************************************************************/
    void
    onRvcCameradReadyinfo(const EnOnRvcReadyInfo info);

 private:
    LooperThread* m_loopThread;
    sp<MessageHandler> m_privateMessageHandler;
    std::map<EnCameradRvcListenerID, StRvcListenerElement> m_listenerMap;
    // sp<CameradRvcEventDispatch> m_rvcEventDispatch;
    sp<CameradRvcDeathRecipientHandler> m_listenerDeathReceiver;

 private:
    DISABLE_COPY(CameradRvcMessageHandler);
};  // EOF class CameradRvcMessageHandler
}  // namespace nutshell

#endif  // INCLUDE_RVC_RTAPDRVCMESSAGEHANDLER_H_
