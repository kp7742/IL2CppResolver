LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE  := IL2CppResolver

LOCAL_SRC_FILES := src/demo.cpp


include $(BUILD_SHARED_LIBRARY)