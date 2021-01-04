LOCAL_PATH := $(call my-dir)
 
include $(CLEAR_VARS)
 
#LOCAL_C_INCLUDES += \
#    frameworks/av/include \
#    development/ndk/platforms/android-14/include
 
 
LOCAL_SHARED_LIBRARIES:= libaudioclient libbinder libutils libcutils
LOCAL_SRC_FILES:= \
    audio_record_test.cpp
  
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE:= test_audiorecord
 
LOCAL_CFLAGS += -g
LOCAL_CFLAGS += -Wno-unused-parameter -Wno-format -Wno-unused-comparison -Wno-unused-variable
#LOCAL_32_BIT_ONLY := true
include $(BUILD_EXECUTABLE)
