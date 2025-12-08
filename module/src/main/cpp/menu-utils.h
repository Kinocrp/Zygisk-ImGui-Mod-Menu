#pragma once

#include <jni.h>

JNIEnv* GetEnv(JavaVM *vm) {
    JNIEnv *env = nullptr;
    int status = vm->GetEnv((void**)&env, JNI_VERSION_1_6);
    if (status == JNI_EDETACHED) {
        if (vm->AttachCurrentThread(&env, nullptr) != 0) {
            return nullptr;
        }
    }
    return env;
}

void GetPhysicalScreenSize(JNIEnv *env, int &width, int &height) {
    width = 0; height = 0;

    jclass activityThreadClass = env->FindClass("android/app/ActivityThread");
    jmethodID currentApplicationMethod = env->GetStaticMethodID(activityThreadClass, "currentApplication", "()Landroid/app/Application;");
    jobject context = env->CallStaticObjectMethod(activityThreadClass, currentApplicationMethod);
    env->DeleteLocalRef(activityThreadClass);

    if (!context) return;

    jclass contextClass = env->GetObjectClass(context);
    jmethodID getSystemService = env->GetMethodID(contextClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    jstring windowServiceStr = env->NewStringUTF("window");
    jobject windowManager = env->CallObjectMethod(context, getSystemService, windowServiceStr);
    
    env->DeleteLocalRef(windowServiceStr);
    env->DeleteLocalRef(contextClass);
    env->DeleteLocalRef(context);

    if (!windowManager) return;

    jclass wmClass = env->GetObjectClass(windowManager);
    jmethodID getDefaultDisplay = env->GetMethodID(wmClass, "getDefaultDisplay", "()Landroid/view/Display;");
    jobject display = env->CallObjectMethod(windowManager, getDefaultDisplay);
    env->DeleteLocalRef(wmClass);
    env->DeleteLocalRef(windowManager);

    if (!display) return;

    jclass metricsClass = env->FindClass("android/util/DisplayMetrics");
    jobject metrics = env->NewObject(metricsClass, env->GetMethodID(metricsClass, "<init>", "()V"));
    
    jclass displayClass = env->GetObjectClass(display);
    env->CallVoidMethod(display, env->GetMethodID(displayClass, "getRealMetrics", "(Landroid/util/DisplayMetrics;)V"), metrics);

    width = env->GetIntField(metrics, env->GetFieldID(metricsClass, "widthPixels", "I"));
    height = env->GetIntField(metrics, env->GetFieldID(metricsClass, "heightPixels", "I"));

    env->DeleteLocalRef(display);
    env->DeleteLocalRef(metrics);
    env->DeleteLocalRef(metricsClass);
    env->DeleteLocalRef(displayClass);
}