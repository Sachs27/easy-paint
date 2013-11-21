LOCAL_PATH := $(call my-dir)/../../
MY_ROOT_PATH := $(LOCAL_PATH)/..

include $(CLEAR_VARS)

LOCAL_MODULE := easy-paint-android

LOCAL_CFLAGS := -DANDROID -DGLES2

LOCAL_C_INCLUDES :=	\
	$(MY_ROOT_PATH)/external/sf/include	\
	$(MY_ROOT_PATH)/external/libzip		\
	$(MY_ROOT_PATH)/external/libpng

MY_JNI_PATH := $(LOCAL_PATH)/android/jni

LOCAL_SRC_FILES :=		\
	3dmath.c			\
	app.c				\
	brush.c				\
	canvas.c			\
	matrix.c			\
	record.c			\
	renderer2d.c		\
	resource_manager.c	\
	sf_rect.c			\
	system.c			\
	texture.c			\
	ui.c				\
	ui_color_picker.c	\
	ui_imagebox.c		\
	ui_menu.c			\
	ui_replay_panel.c	\
	ui_toolbox.c		\
	user_learn_panel.c	\
	user_paint_panel.c	\
	vector.c			\
	$(MY_JNI_PATH)/input_manager.c						\
	$(MY_JNI_PATH)/me_sachs_easypaint_EasyPaintLib.c	\
	$(MY_JNI_PATH)/window.c								\

LOCAL_STATIC_LIBRARIES := libpng libzip sf

LOCAL_LDLIBS := -lz -lGLESv2 -llog

include $(BUILD_SHARED_LIBRARY)

include $(MY_ROOT_PATH)/external/sf/Android.mk
include $(MY_ROOT_PATH)/external/libzip/Android.mk
include $(MY_ROOT_PATH)/external/libpng/Android.mk
