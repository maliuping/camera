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
 * @file RtapdRvcEventDispatch.h
 * @brief Declaration file of class RtapdRvcEventDispatch
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
class RtapdRvcMessageHandler;

/*********************************************************************************
    class name : RtapdRvcEventDispatch
    purpose    : RtapdRvcEventDispatch implement method in Comd meter
*********************************************************************************/
class RtapdRvcEventDispatch : public virtual NHRtapEventDispatch {
 public:
    /*************************************************************************/
    /**
       RtapdRvcEventDispatch class constructor
    */
    /*************************************************************************/
    explicit RtapdRvcEventDispatch(NHRtapEventDispatch_TYPE type);

    /*************************************************************************/
    /**
        RtapdRvcEventDispatch class destructor
    */
    /*************************************************************************/
    virtual ~RtapdRvcEventDispatch();

    /*************************************************************************/
    /**
        setHandler

        @param  handler : [IN]  RtapdRvcEventDispatch sp pointer

        @retval none
    */
    /*************************************************************************/
    void
    setHandler(sp<RtapdRvcMessageHandler> handler);

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
    sp<RtapdRvcMessageHandler> m_handler;
};  // EOF class RtapdRvcEventDispatch

}  // namespace nutshell

#endif  // INCLUDE_RVC_RTAPDRVCEVENTDISPATCH_H_
