#pragma once

#include <jni.h>

namespace Menu {
    extern bool IsESP;
}

void RenderThread(JavaVM *vm, JNIEnv *env);
