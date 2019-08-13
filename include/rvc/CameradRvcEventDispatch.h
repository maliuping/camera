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
 * @file CameradRvcEventDispatch.h
 * @brief Declaration file of class CameradRvcEventDispatch
 */

#ifndef INCLUDE_RVC_CAMERADRVCEVENTDISPATCH_H_
#define INCLUDE_RVC_CAMERADRVCEVENTDISPATCH_H_

#ifndef __cplusplus
#   error ERROR: This file requires C++ compilation(use a .cpp suffix)
#endif

/*********************************************************************************
                            include files
*********************************************************************************/
//#include "nhrtap/NHRtapEventDispatch.h"

namespace nutshell {

/*********************************************************************************
                                Class Declaration
*********************************************************************************/
class CameradRvcMessageHandler;

/*********************************************************************************
    class name : CameradRvcEventDispatch
    purpose    : CameradRvcEventDispatch implement method in Comd meter
*********************************************************************************/
class CameradRvcEventDispatch : public virtual NHRtapEventDispatch {
 public:
    /*************************************************************************/
    /**
       CameradRvcEventDispatch class constructor
    */
    /*************************************************************************/
    explicit CameradRvcEventDispatch(NHRtapEventDispatch_TYPE type);

    /*************************************************************************/
    /**
        CameradRvcEventDispatch class destructor
    */
    /*************************************************************************/
    virtual ~CameradRvcEventDispatch();

    /*************************************************************************/
    /**
        setHandler

        @param  handler : [IN]  CameradRvcEventDispatch sp pointer

        @retval none
    */
    /*************************************************************************/
    void
    setHandler(sp<CameradRvcMessageHandler> handler);

    /*************************************************************************/
    /**
        onReceiveRvcDatafromRtap

        @param  rtapData : [IN]  data receive from Rtap

        @retval none
    */
    /*************************************************************************/
    void
    onReceiveRvcDatafromRtap(const NHRtapInfoRcvData &rtapData);

 private:
    sp<CameradRvcMessageHandler> m_handler;
};  // EOF class CameradRvcEventDispatch

}  // namespace nutshell

#endif  // INCLUDE_RVC_RTAPDRVCEVENTDISPATCH_H_
