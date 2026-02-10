#pragma once
#include <jni.h>
#include <string>
#include "imgui.h"
#include "imgui_impl_opengl3.h"

#ifdef __cplusplus
extern "C" {
#endif

    ImGuiContext* GetImGuiContext();
    void InitService(JavaVM *vm, JNIEnv *env, jobject activity, void *_init_gui_cb, void *_resize_gui_cb, void *_draw_gui_cb, void *_touch_gui_cb);

#ifdef __cplusplus
}
#endif
