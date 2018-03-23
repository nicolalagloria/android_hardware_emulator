LOCAL_PATH:=$(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := sensor-daughter-emulator
LOCAL_CFLAGS += -Wall -fPIE
LOCAL_LDFLAGS += -fPIE -pie
LOCAL_C_INCLUDES := bionic

LOCAL_SRC_FILES := helpers.c wsemu.c

include $(BUILD_EXECUTABLE)

