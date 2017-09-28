LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	cameracap_main.cpp \
	CameraCapture.cpp \
	CameraIn.cpp \
	shader.cpp \
	texture.cpp \
	cam_renderer.cpp \
	resource_manager.cpp

LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES -DBUILD_FOR_ANDROID -DGLM_FORCE_CXX98

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	liblog \
	libandroidfw \
	libutils \
	libstlport \
	libbinder \
    libui \
	libskia \
    libEGL \
    libGLESv2 \
    libgui

LOCAL_C_INCLUDES := \
	bionic \
	external/stlport/stlport

LOCAL_C_INCLUDES += \
	$(call include-path-for, corecg graphics libstdc++)

LOCAL_NDK_STL_VARIANT := stlport_shared
	
LOCAL_MODULE:= cameracap

include $(BUILD_EXECUTABLE)
