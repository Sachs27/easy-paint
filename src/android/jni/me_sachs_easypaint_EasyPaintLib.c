#include <android/log.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))

#include <sf/log.h>

#include "me_sachs_easypaint_EasyPaintLib.h"
#include "../../app.h"
#include "../../window.h"
#include "../../input_manager.h"

/*
 * Class:     me_sachs_easypaint_EasyPaintLib
 * Method:    app_init_window
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_me_sachs_easypaint_EasyPaintLib_app_1init_1window
  (JNIEnv *env, jclass class) {
    g_app.window = window_create(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
    g_app.im = input_manager_create(g_app.window);
}

static sf_bool_t on_sf_log(sf_log_level_t level, const char *str) {
    switch (level) {
    case SF_LOG_INFO:
        LOGI("%s", str);
        break;
    case SF_LOG_WARN:
        LOGW("%s", str);
    default:
        LOGD("%s", str);
    }
}

/*
 * Class:     me_sachs_easypaint_EasyPaintLib
 * Method:    app_init_render_context
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_me_sachs_easypaint_EasyPaintLib_app_1init_1render_1context
  (JNIEnv *env, jclass class) {
    sf_log_set_hook(on_sf_log);
    return app_init();
}

/*
 * Class:     me_sachs_easypaint_EasyPaintLib
 * Method:    app_load_resource
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_me_sachs_easypaint_EasyPaintLib_app_1load_1resource
  (JNIEnv *env, jclass cls, jstring apk_path) {
    const char *str = (*env)->GetStringUTFChars(env, apk_path, 0);
    app_load_resource(str);
}

/*
 * Class:     me_sachs_easypaint_EasyPaintLib
 * Method:    app_on_update
 * Signature: (D)V
 */
JNIEXPORT void JNICALL Java_me_sachs_easypaint_EasyPaintLib_app_1on_1update
  (JNIEnv *env, jclass class, jdouble dt) {
    app_on_update(dt);
}

/*
 * Class:     me_sachs_easypaint_EasyPaintLib
 * Method:    app_on_render
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_me_sachs_easypaint_EasyPaintLib_app_1on_1render
  (JNIEnv *env, jclass class) {
    app_on_render();
}

/*
 * Class:     me_sachs_easypaint_EasyPaintLib
 * Method:    app_on_resize
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_me_sachs_easypaint_EasyPaintLib_app_1on_1resize
  (JNIEnv *env, jclass class, jint width, jint height) {
    if (width == 0 || height == 0) {
        return;
    }

    g_app.window->w = width;
    g_app.window->h = height;

    app_on_resize(g_app.window, width, height);
}

/*
 * Class:     me_sachs_easypaint_EasyPaintLib
 * Method:    app_input_manager_touch_down
 * Signature: (I[I[I)V
 */
JNIEXPORT void JNICALL Java_me_sachs_easypaint_EasyPaintLib_app_1input_1manager_1touch_1down
  (JNIEnv *env, jclass cls, jint n, jintArray x, jintArray y) {
    jint *ax, *ay;
    ax = (*env)->GetIntArrayElements(env, x, 0);
    ay = (*env)->GetIntArrayElements(env, y, 0);
    input_manager_touch_down(n, ax, ay);
    (*env)->ReleaseIntArrayElements(env, x, ax, 0);
    (*env)->ReleaseIntArrayElements(env, y, ay, 0);
}

/*
 * Class:     me_sachs_easypaint_EasyPaintLib
 * Method:    app_input_manager_touch_up
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_me_sachs_easypaint_EasyPaintLib_app_1input_1manager_1touch_1up
  (JNIEnv *env, jclass cls) {
    input_manager_touch_up();
}

