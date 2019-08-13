LOCAL_PATH :=$(call my-dir)

##############build libcameradrvc####################
include $(CLEAR_VARS)

$(warning $(LOCAL_PATH))
LOCAL_MODULE := libcameradrvc

LOCAL_C_INCLUDES := ivi/system/hardware/handler/camera/include \
		    ivi/system/core/include/


LOCAL_SRC_FILES := interface_impl/rvc/CameradRvcImpl.cpp \
                   src/rvc/CameradRvcMessageHandler.cpp \



LOCAL_SHARED_LIBRARIES := libutils liblog libservicebase\
                          libhidlbase libhidltransport \
                          iauto.hardware.camerad.rvc@1.0


ifeq ($(TARGET_ARCH), x86_64)
LOCAL_STATIC_LIBRARIES += libgcov libprofile_rt
endif

LOCAL_CFLAGS += -g3 -o0 -fprofile-arcs -ftest-coverage  -Xclang -coverage-cfg-checksum -Xclang -coverage-no-function-names-in-data -Xclang -coverage-version='504*'
LOCAL_CLIKER := --coverage
LOCAL_STATIC_LIBRARIES += libgcov libprofile_rt

LOCAL_MODULE_TAGS := optional

LOCAL_VENDOR_MODULE := true

include $(BUILD_SHARED_LIBRARY)


##############build libcameradmanager####################
include $(CLEAR_VARS)

$(warning $(LOCAL_PATH))
LOCAL_MODULE := libcameradmanager

LOCAL_C_INCLUDES := ivi/system/hardware/handler/camera/include \
		    ivi/system/core/include/

LOCAL_SRC_FILES := src/cameradmanager/CameradManager.cpp

LOCAL_CFLAGS += -g3 -o0 -fprofile-arcs -ftest-coverage  -Xclang -coverage-cfg-checksum -Xclang -coverage-no-function-names-in-data -Xclang -coverage-version='504*'
LOCAL_CLIKER := --coverage
LOCAL_STATIC_LIBRARIES += libgcov libprofile_rt

LOCAL_SHARED_LIBRARIES := libutils liblog libservicebase \
                          libcameradrvc \
                          iauto.hardware.camerad.rvc@1.0



LOCAL_MODULE_TAGS := optional

LOCAL_VENDOR_MODULE := true

include $(BUILD_SHARED_LIBRARY)

include $(call all-makefiles-under, $(LOCAL_PATH))
