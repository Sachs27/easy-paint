LOCAL_PATH := $(call my-dir)
MY_SRC_PATH := ../..
MY_ROOT_PATH := ../..

include $(CLEAR_VARS)

LOCAL_MODULE := easy-paint

LOCAL_CFLAGS := -DANDROID -DGLES2

LOCAL_C_INCLUDES :=						\
	$(MY_ROOT_PATH)/external/sf/include	\
	$(MY_ROOT_PATH)/external/libzip		\
	$(MY_ROOT_PATH)/external/libpng

LOCAL_SRC_FILES :=						\
	input_manager.c						\
	main.c								\
	window.c							\
	$(MY_SRC_PATH)/3dmath.c				\
	$(MY_SRC_PATH)/app.c				\
	$(MY_SRC_PATH)/brush.c				\
	$(MY_SRC_PATH)/canvas.c				\
	$(MY_SRC_PATH)/matrix.c				\
	$(MY_SRC_PATH)/record.c				\
	$(MY_SRC_PATH)/renderer2d.c			\
	$(MY_SRC_PATH)/resource_manager.c	\
	$(MY_SRC_PATH)/sf_rect.c			\
	$(MY_SRC_PATH)/system.c				\
	$(MY_SRC_PATH)/texture.c			\
	$(MY_SRC_PATH)/ui.c					\
	$(MY_SRC_PATH)/ui_color_picker.c	\
	$(MY_SRC_PATH)/ui_imagebox.c		\
	$(MY_SRC_PATH)/ui_menu.c			\
	$(MY_SRC_PATH)/ui_replay_panel.c	\
	$(MY_SRC_PATH)/ui_toolbox.c			\
	$(MY_SRC_PATH)/user_learn_panel.c	\
	$(MY_SRC_PATH)/user_paint_panel.c	\
	$(MY_SRC_PATH)/vector.c				\

LOCAL_STATIC_LIBRARIES := android_native_app_glue libpng libzip sf

LOCAL_LDLIBS := -landroid -lEGL -lGLESv2 -llog -lz

include $(BUILD_SHARED_LIBRARY)

include $(MY_ROOT_PATH)/external/sf/Android.mk
include $(MY_ROOT_PATH)/external/libzip/Android.mk
include $(MY_ROOT_PATH)/external/libpng/Android.mk

$(call import-module,android/native_app_glue)
