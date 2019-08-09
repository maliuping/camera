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

#ifndef INCLUDE_BASE_RTAPDTHREADNAME_H_
#define INCLUDE_BASE_RTAPDTHREADNAME_H_
#ifndef __cplusplus
#   error ERROR: This file requires C++ compilation(use a .cpp suffix)
#endif
namespace nutshell {
/*========================================================================
*   RTAPD
========================================================================*/
// rtapd nhrtap receive data thread
#define RTAPD_NHRTAP_RECEIVE_THREAD_DEF   NCTEXT("RTAPD_NHRTAP_RECEIVE_THREAD")
const NCCHAR RTAPD_NHRTAP_RECEIVE_THREAD[] = RTAPD_NHRTAP_RECEIVE_THREAD_DEF;
// rtapd nhrtap receive data service thread
#define RTAPD_NHRTAP_PARSE_THREAD_DEF   NCTEXT("RTAPD_NHRTAP_PARSE_THREAD")
const NCCHAR RTAPD_NHRTAP_PARSE_THREAD[] = RTAPD_NHRTAP_PARSE_THREAD_DEF;
// romd nhrtap send data service thread
#define RTAPD_NHRTAP_SEND_SERVICE_THREAD_DEF   NCTEXT("RTAPD_NHRTAP_SEND_SERVICE_THREAD")
const NCCHAR RTAPD_NHRTAP_SEND_SERVICE_THREAD[] = RTAPD_NHRTAP_SEND_SERVICE_THREAD_DEF;

/*========================================================================
*   rvc service
========================================================================*/
// camerad rvc message looper thread
#define CAMERAD_RVC_MSG_LOOPER_THREAD_DEF   NCTEXT("CAMERAD_RVC_MSG_LOOPER_THREAD")
const NCCHAR CAMERAD_RVC_MSG_LOOPER_THREAD[] = CAMERAD_RVC_MSG_LOOPER_THREAD_DEF;

}  // namespace nutshell
#endif  // INCLUDE_BASE_RTAPDTHREADNAME_H_
