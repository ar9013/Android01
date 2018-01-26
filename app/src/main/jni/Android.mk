LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

OPENCV_INSTALL_MODULES:= on
OPENCV_CAMERA_MODULES:= off
OPENCV_LIB_TYPE:= STATIC
include /Users/luokangyu/Downloads/opencv_2.4_binary_for_android_with_crystax_ndk-master/opencv/sdk/native/jni/OpenCV.mk

LOCAL_MODULE    := test-boost
LOCAL_SRC_FILES :=  jni.cpp gps.cpp iCRT.cpp iCRTImp.cpp FileUtils.cpp jniHelper.cpp
LOCAL_SHARED_LIBRARIES := boost_thread_shared boost_serialization_shared boost_system_shared boost_iostreams_shared

LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog -lz -latomic
LOCAL_CFLAGS += -g

include $(BUILD_SHARED_LIBRARY)

$(call import-module,boost/1.62.0)
