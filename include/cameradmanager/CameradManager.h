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
 * @file CameradManager.h
 * @brief Declaration file of class CameradManager
 */
#ifndef INCLUDE_CAMERADMANAGER_CAMERADMANAGER_H_
#define INCLUDE_CAMERADMANAGER_CAMERADMANAGER_H_
#ifndef __cplusplus
#    error ERROR: This file requires C++ compilation(use a .cpp suffix)
#endif
#include <utils/RefBase.h>

namespace nutshell {
class NHCameraManager;
class CameradRvcImpl;

class CameradManager : virtual public android::RefBase {
 public:
    /*************************************************************************/
    /**
        CameradManager Constructor
    */
    /*************************************************************************/
    CameradManager();

    /*************************************************************************/
    /**
        CameradManager Destructor
    */
    /*************************************************************************/
    virtual ~CameradManager();

    /*************************************************************************/
    /**
        CameradManager instance
    */
    /*************************************************************************/
    static CameradManager* instance();

    /*************************************************************************/
    /**
        CameradManager init
    */
    /*************************************************************************/
    void init();

 private:
    static android::sp<CameradManager> s_instance;
    NHCameraManager* m_uCameraManager;
    android::sp<CameradRvcImpl> m_rvcImpl;
};

}  // namespace nutshell
#endif  // INCLUDE_RTAPDMANAGER_RTAPDMANAGER_H_
