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
#include <map>
#include <vector>
#include "log/log.h"
#include "ncore/NCLog.h"
//#include "nhrtap/NHRtapSenderIF.h"
#include "rvc/CameradRvcMessageHandler.h"
#include "rvc/CameradRvcEventDispatch.h"
#include "base/CameradThreadName.h"

#undef LOG_TAG
#define LOG_TAG "CameradRvc"

namespace nutshell {

/********************************************************************************************************************
 * Class for LooperThread method
 * ********************************************************************************************************************/
/*********************************************************************************
    classname:  CameradPrivateRvcMessageHandler
    funcname:   CameradPrivateRvcMessageHandler
    contents:   CameradPrivateRvcMessageHandler constructor
    parameter:  sp<CameradRvcMessageHandler> callback
    return:     none
    note:
*********************************************************************************/
CameradPrivateRvcMessageHandler::CameradPrivateRvcMessageHandler(sp<CameradRvcMessageHandler> callback)
    : m_callback(callback) {
}

/*********************************************************************************
    classname:  CameradPrivateRvcMessageHandler
    funcname:   ~CameradPrivateRvcMessageHandler
    contents:   CameradPrivateRvcMessageHandler distructor
    parameter:  none
    return:     none
    note:
*********************************************************************************/
CameradPrivateRvcMessageHandler::~CameradPrivateRvcMessageHandler() {
    m_callback = NULL;
}

/*********************************************************************************
    classname:  CameradPrivateRvcMessageHandler
    funcname:   onReceiveMessage
    contents:   virtual function ,public inherit MessageHandler
    parameter:  android::sp<Message> &msg
    return:     none
    note:
*********************************************************************************/
void
CameradPrivateRvcMessageHandler::onReceiveMessage(const android::sp<Message> &msg) {
    if (m_callback != NULL) {
        m_callback->onReceiveMessage(msg);
    } else {
        ALOGE("CameradPrivateRvcMessageHandler onReceiveMessage m_callback is NULL!");
    }
}

/********************************************************************************************************************
 * Class for listener death detection method
 * ********************************************************************************************************************/
/*********************************************************************************
    classname:  CameradRvcDeathRecipientHandler
    funcname:   CameradRvcDeathRecipientHandler
    contents:   CameradRvcDeathRecipientHandler constructor
    parameter:  sp<CameradRvcMessageHandler> handler
    return:     none
    note:
*********************************************************************************/
CameradRvcDeathRecipientHandler::CameradRvcDeathRecipientHandler(sp<CameradRvcMessageHandler> handler)
    : m_handler(handler) {
}

/*********************************************************************************
    classname:  CameradRvcDeathRecipientHandler
    funcname:   ~CameradRvcDeathRecipientHandler
    contents:   CameradRvcDeathRecipientHandler destructor
    parameter:  none
    return:     none
    note:
*********************************************************************************/
CameradRvcDeathRecipientHandler::~CameradRvcDeathRecipientHandler() {
    if (m_handler != NULL) {
        m_handler = NULL;
    } else {
        // do nothing
    }
}

/*********************************************************************************
    classname:  CameradRvcDeathRecipientHandler
    funcname:   serviceDied
    contents:   inform service died
    parameter:  uint64_t cookie
    return:     none
    note:
*********************************************************************************/
void
CameradRvcDeathRecipientHandler::serviceDied(uint64_t cookie,
    const android::wp<::android::hidl::base::V1_0::IBase>& who) {
    ALOGD("CameradRvcDeathRecipientHandler::serviceDied cookie:%d", static_cast<uint8_t>(cookie));
    (void)who;
    m_handler->listenerServiceDiedInform(cookie);
}

/********************************************************************************************************************
 * Class for Notify HIDL method
 * ********************************************************************************************************************/
/*********************************************************************************
    classname:  CameradRvcMessageHandler
    funcname:   CameradRvcMessageHandler
    contents:   CameradRvcMessageHandler constructor
    parameter:  none
    return:     none
    note:
*********************************************************************************/
CameradRvcMessageHandler::CameradRvcMessageHandler()
    : m_loopThread(NULL)
    , m_privateMessageHandler(NULL)
    , m_listenerMap()
    //, m_rvcEventDispatch(NULL)
    , m_listenerDeathReceiver(NULL) {
    m_privateMessageHandler = new CameradPrivateRvcMessageHandler(this);
    if (m_privateMessageHandler == NULL) {
        ALOGE("CameradRvcMessageHandler m_privateMessageHandler is NULL!");
    }

    m_loopThread = new LooperThread(m_privateMessageHandler);
    if (NULL == m_loopThread) {
        ALOGE("CameradRvcMessageHandler m_loopThread is NULL!");
    }

    // new m_rvcEventDispatch
    //m_rvcEventDispatch =
    //new CameradRvcEventDispatch(NHRtapEventDispatch_rvc);
    //if (m_rvcEventDispatch == NULL) {
    //    ALOGE("CameradRvcMessageHandler m_rvcEventDispatch is NULL!");
    //}

    //m_rvcEventDispatch->setHandler(this);

    // registered to dispatchmanager, so we can receive message from RTAP
    //NHRtapEventDispatchManager::Instance()->
    //registeredDispatcher(m_rvcEventDispatch);

    m_listenerDeathReceiver = new CameradRvcDeathRecipientHandler(this);
    if (m_listenerDeathReceiver == NULL) {
        ALOGE("CameradRvcMessageHandler m_listenerDeathReceiver is NULL!");
    }
}

/*********************************************************************************
    classname:  CameradRvcMessageHandler
    funcname:   ~CameradRvcMessageHandler
    contents:   CameradRvcMessageHandler distructor
    parameter:  none
    return:     none
    note:
*********************************************************************************/
CameradRvcMessageHandler::~CameradRvcMessageHandler() {
    if (m_loopThread != NULL) {
        m_loopThread->stop();
        delete m_loopThread;
        m_loopThread = NULL;
    }

    // unregistered to dispatchmanager, so we can not receive message from RTAP
    //NHRtapEventDispatchManager::Instance()->unRegisteredDispatcher(m_rvcEventDispatch);

    m_privateMessageHandler = NULL;
    //m_rvcEventDispatch = NULL;
    m_listenerDeathReceiver = NULL;
}

/*********************************************************************************
    classname:  CameradRvcMessageHandler
    funcname:   onAwake
    contents:    Camerad rvc MessageHandler awake operation
    parameter:  none
    return:     none
    note:
*********************************************************************************/
void
CameradRvcMessageHandler::onAwake() {
    if (m_loopThread != NULL) {
        m_loopThread->start(CAMERAD_RVC_MSG_LOOPER_THREAD, THREAD_PRIORITY_AUDIO);
    }
}

/*********************************************************************************
    classname:  CameradRvcMessageHandler
    funcname:   onReceiveMessage
    contents:   parse message from loop thread
    parameter:  android::sp<Message>
    return:     none
    note:
*********************************************************************************/
void
CameradRvcMessageHandler::onReceiveMessage(const android::sp<Message> &msg) {
    switch (msg->what()) {
        case Rvc_registerListener: {
            RvcRegisterMessage* rmsg = DOWNCAST((msg.get()), Message, RvcRegisterMessage);
            if (rmsg != NULL) {
                m_listenerMap[rmsg->service()] = rmsg->getElement();
                m_listenerMap[rmsg->service()].listener->linkToDeath(m_listenerDeathReceiver,
                    static_cast<uint64_t>(rmsg->service()));
                ALOGD("CameradRvcMessageHandler MAP:IN registerListenerID:%d", (uint8_t)rmsg->service());
            } else {
                ALOGE("CameradRvcMessageHandler DOWNCAST is failed!");
            }
            break;
        }
        case Rvc_unRegisterListener: {
            int service = 0;
            bool result = msg->getIntValue(0, &service);
            if (result) {
                m_listenerMap[static_cast<EnCameradRvcListenerID>(service)].listener->
                        unlinkToDeath(m_listenerDeathReceiver);
                m_listenerMap.erase(static_cast<EnCameradRvcListenerID>(service));
                ALOGD("CameradRvcMessageHandler MAP:erase serviceId:%d", service);
            } else {
                ALOGE("CameradRvcMessageHandler DOWNCAST is failed!");
            }
            break;
        }
        case Rvc_receiveRtapinfo: {
            RtapDataMessage< NHRtapInfoRcvData>* RcvMsg = DOWNCAST((msg.get()), nutshell::Message,
            nutshell::RtapDataMessage< NHRtapInfoRcvData>);
            if (RcvMsg != NULL) {
                NHRtapInfoRcvData rtapData;
                StRvcFromRtapCmdDataInfo commonMsgData;
                std::vector<BYTE>  temp;
                RcvMsg->getData(&rtapData);    // Get MeterData from Ucom
                commonMsgData.dwDataNum = rtapData.dwDataNum;
                for (DWORD i = 0; i < rtapData.dwDataNum; i++)
                   temp.push_back(rtapData.byData[i]);
                commonMsgData.byData = temp;
                notifyRvcinfofromRtap(commonMsgData);
            } else {
                ALOGE("CameradRvcMessageHandler DOWNCAST is failed!");
            }
            break;
        }
        case Rvc_listenerServiceDiedInform: {
            RtapDataMessage<uint64_t>* rvcDeathMsg = DOWNCAST((msg.get()), nutshell::Message,
                nutshell::RtapDataMessage<uint64_t>);
            if (rvcDeathMsg != NULL) {
                uint64_t cookie = 0;
                rvcDeathMsg->getData(&cookie);
                m_listenerMap.erase(static_cast<EnCameradRvcListenerID>(cookie));
                ALOGD("CameradRvcMessageHandler MAP:erase serviceId:%d", static_cast<uint8_t>(cookie));
            } else {
                ALOGE("CameradRvcMessageHandler DOWNCAST is failed!");
            }
            break;
        }
        default:
            break;
    }
}

/*********************************************************************************
    classname:  CameradRvcMessageHandler
    funcname:   listenerServiceDiedInform
    contents:   post listener died to loopthread
    parameter:  uint64_t cookie
    return:     none
*********************************************************************************/
void
CameradRvcMessageHandler::listenerServiceDiedInform(uint64_t cookie) {
    if (m_loopThread != NULL) {
        sp<Message> msg = new RtapDataMessage<uint64_t>(Rvc_listenerServiceDiedInform, cookie);
        if (msg == NULL) {
            ALOGE("CameradRvcMessageHandler::listenerServiceDiedInform msg is NULL!");
        }

        m_loopThread->postMessage(msg, 0);
        ALOGD("CameradRvcMessageHandler::listenerServiceDiedInform cookie[%d] Dead!!! PostMessage[%d] OK",
            static_cast<uint8_t>(cookie), Rvc_listenerServiceDiedInform);
    } else {
        ALOGE("CameradRvcMessageHandler::listenerServiceDiedInform cookie[%d] Dead!!! PostMessage[%d] NG",
            static_cast<uint8_t>(cookie), Rvc_listenerServiceDiedInform);
    }
}

/********************************************************************************************************************
 * HIDL Interface process function declaration
 * ******************************************************************************************************************/
/*********************************************************************************
    classname:  CameradRvcMessageHandler
    funcname:   registerListener
    contents:   post CameradRvcMessage to loopthread ,request add service listener pointer in map
    parameter:  const sp<IRvcListener>& listener, EnCameradRvcListenerID serviceId, uint64_t eventIds
    return:     EnRvcFuncResult
    note:
*********************************************************************************/
EnRvcFuncResult
CameradRvcMessageHandler::registerListener(const sp<IRvcListener>& listener,
    EnCameradRvcListenerID serviceId, uint64_t eventIds) {
    StRvcListenerElement element;
    EnRvcFuncResult funcResult = EnRvcFuncResult::RVC_RESULT_ERR;
    element.listener = listener;
    element.concernEventIds = eventIds;

    if (m_loopThread != NULL) {
        android::sp<Message> msg = new RvcRegisterMessage(Rvc_registerListener, element, serviceId);
        if (msg != NULL) {
            m_loopThread->postMessage(msg, 0);
            funcResult = EnRvcFuncResult::RVC_RESULT_OK;
            ALOGD("CameradRvcMessageHandler::registerListener serviceId[%d] PostMessage[%d] OK",
                static_cast<uint8_t>(serviceId), Rvc_registerListener);
        } else {
            ALOGE("CameradRvcMessageHandler::registerListener msg is NULL");
        }
    } else {
        ALOGE("CameradRvcMessageHandler::registerListener m_loopThread is NULL");
    }
    return funcResult;
}

/*********************************************************************************
    classname:  CameradRvcMessageHandler
    funcname:   UnRegisterListener
    contents:   post unregisterlistener message to loopthread ,request remove service listener pointer in map
    parameter:  EnCameradRvcListenerID serviceId
    return:     EnRvcFuncResult
    note:
*********************************************************************************/
EnRvcFuncResult
CameradRvcMessageHandler::unRegisterListener(EnCameradRvcListenerID serviceId) {
    EnRvcFuncResult funcResult = EnRvcFuncResult::RVC_RESULT_ERR;
    if (m_loopThread != NULL) {
        android::sp<Message> msg = new Message(Rvc_unRegisterListener);
        if (msg != NULL) {
            msg->setInt((uint8_t)serviceId);
            m_loopThread->postMessage(msg, 0);
            funcResult = EnRvcFuncResult::RVC_RESULT_OK;
            ALOGD("CameradRvcMessageHandler::unRegisterListener serviceId[%d] PostMessage[%d] OK",
                static_cast<uint8_t>(serviceId), Rvc_unRegisterListener);
        } else {
            ALOGE("CameradRvcMessageHandler::unRegisterListener msg is NULL");
        }
    } else {
        ALOGE("CameradRvcMessageHandler::unRegisterListener m_loopThread is NULL");
    }
    return funcResult;
}

/*********************************************************************************
    classname:  CameradRvcMessageHandler
    funcname:   SendDataToRtap
    contents:   send data to Rtap
    parameter:  StRvcToRtapCmdDataInfo&  rvcData
    return:     EnRvcFuncResult
    note:
*********************************************************************************/
EnRvcFuncResult
CameradRvcMessageHandler::SendDataToRtap(const StRvcToRtapCmdDataInfo&  rvcData ) {
    EnRvcFuncResult funcResult = EnRvcFuncResult::RVC_RESULT_ERR;
    bool rvcSendResult = FALSE;
    NHRtapInfoSendData sSendToRtapData;
    memset(&sSendToRtapData, 0x00, sizeof(NHRtapInfoSendData));
    if (rvcData.dwDataNum <= NH_URTAP_SIZE_D_NORMAL) {
      sSendToRtapData.dwDataNum = rvcData.dwDataNum;
      ALOGD("sSendToRtapData.dwDataNum : %d", sSendToRtapData.dwDataNum);
      for (uint8_t i = 0; i < rvcData.dwDataNum; i++) {
         sSendToRtapData.byData[i] = rvcData.byData[i];
      }
      rvcSendResult = NHRtapSenderIF::Instance()->sendrvcdata(sSendToRtapData);
      if (NC_TRUE == rvcSendResult) {
        funcResult = EnRvcFuncResult::RVC_RESULT_OK;
      }
    }
    return funcResult;
}

/*********************************************************************************
    classname:  CameradRvcMessageHandler
    funcname:   onReceiveRvcDatafromRtap
    contents:   Receive data from Rtap
    parameter:  NHRtapInfoRcvData &rtapData
    return:     none
    note:
*********************************************************************************/
void
CameradRvcMessageHandler::onReceiveRvcDatafromRtap(const NHRtapInfoRcvData &rtapData) {
    ALOGD("CameradRvcMessageHandler::onReceiveRvcDatafromRtap()");
    if (m_loopThread != NULL) {
        sp<Message> msg = new RtapDataMessage<NHRtapInfoRcvData>(Rvc_receiveRtapinfo, rtapData);
        m_loopThread->postMessage(msg, 0);
    } else {
        ALOGE("CameradRvcMessageHandler::onReceiveRvcDatafromRtap m_loopThread is NULL");
    }
}

/*********************************************************************************
    classname:  CameradRvcMessageHandler
    funcname:   notifyRvcinfofromRtap
    contents:   Send data to RVC service by callback
    parameter:  StRvcFromRtapCmdDataInfo &rtapData
    return:     none
    note:
*********************************************************************************/
void
CameradRvcMessageHandler::notifyRvcinfofromRtap(const StRvcFromRtapCmdDataInfo &rtapData) {
    ALOGD("CameradRvcMessageHandler::notifyRvcinfofromRtap()");
    CommonLisMap::iterator iter = m_listenerMap.begin();
    for (; iter != m_listenerMap.end(); ++iter) {
      if ((iter->second.listener != NULL)
            && (iter->second.concernEventIds
                & EnRvcEventId::EN_RTAP_TO_RVC_MSG)) {
            auto ret = iter->second.listener->
                         notifyRvcinfofromRtap(rtapData);
            if (ret.isOk()) {
                ALOGD("notify serviceID[%d]",
                        static_cast<uint8_t>(iter->first));
            } else {
                ALOGE("HWBINDER ERROR :notify serviceID[%d] ",
                        static_cast<uint8_t>(iter->first));
            }
        } else {
            ALOGE("serviceID[%d] do not register this event",
                    static_cast<uint8_t>(iter->first));
        }
    }
}

}  // namespace nutshell
