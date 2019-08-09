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
#ifndef INCLUDE_BASE_MESSAGEDERIVE_H_
#define INCLUDE_BASE_MESSAGEDERIVE_H_
#ifndef __cplusplus
#    error ERROR: This file requires C++ compilation(use a .cpp suffix)
#endif
#include "servicebase/Message.h"

namespace nutshell {
#define OFFSET(base, inherit) (reinterpret_cast<char*>(0x400) \
        - reinterpret_cast<char*>(static_cast<base*>(reinterpret_cast<inherit*>(0x400))))
#define DOWNCAST(pBase, base, inherit) reinterpret_cast<inherit*>(reinterpret_cast<char*>(pBase) \
        + OFFSET(base, inherit))

/*********************************************************************************
    class name : CameraDataMessage
    purpose    : used to post rtap data to loopthread
*********************************************************************************/
template<typename T>
class CameraDataMessage : public Message {
 public:
    /*********************************************************************************
        classname:  CameraDataMessage
        funcname:   CameraDataMessage
        contents:   CameraDataMessage constructor
        parameter:  int msgId, T& data
        return:     none
        note:
    *********************************************************************************/
    CameraDataMessage(int msgId, const T& data)
        : Message(msgId)
        , m_data(data) {
    }

    /*********************************************************************************
        classname:  CameraDataMessage
        funcname:   ~CameraDataMessage
        contents:   CameraDataMessage distructor
        parameter:  none
        return:     none
        note:
    *********************************************************************************/
    virtual ~CameraDataMessage() {
    }

    /*********************************************************************************
        classname:  CameraDataMessage
        funcname:   getData
        contents:   read T
        parameter:  T* data
        return:     none
        note:
    *********************************************************************************/
    void getData(T* data) {
        *data = m_data;
    }

 private:
    T m_data;
};  // EOF class CameraDataMessage

/*********************************************************************************
    class name : RegisterMessage
    purpose    : add hallistener pointer and service ID in Message
                 MessageHandler can get hallistener pointer and service ID  from loorthread
*********************************************************************************/
template<typename T, typename S>
class RegisterMessage : public Message {
 public:
    /*********************************************************************************
        classname:  RegisterMessage
        funcname:   RegisterMessage
        contents:   RegisterMessage constructor
        parameter:  int msgId, T listenerElement, S service
        return:     none
        note:
    *********************************************************************************/
    RegisterMessage(int32_t msgId, T listenerElement, S service)
        : Message(msgId)
        , m_listenerElement(listenerElement)
        , m_serviceId(service) {
    }

    /*********************************************************************************
        classname:  RegisterMessage
        funcname:   ~RegisterMessage
        contents:   RegisterMessage distructor
        parameter:  none
        return:     none
        note:
    *********************************************************************************/
    virtual ~RegisterMessage() {
    }

    /*********************************************************************************
        classname:  RegisterMessage
        funcname:   getElement
        contents:   get listener pointer
        parameter:  none
        return:     T
        note:
    *********************************************************************************/
    T getElement() {
        return m_listenerElement;
    }

    /*********************************************************************************
        classname:  RegisterMessage
        funcname:   service
        contents:   get service ID
        parameter:  none
        return:     S
        note:
    *********************************************************************************/
    S service() {
        return m_serviceId;
    }

 private:
    T m_listenerElement;
    S m_serviceId;
};  // EOF class RegisterMessage

}  // namespace nutshell


#endif  // INCLUDE_BASE_MESSAGEDERIVE_H_
