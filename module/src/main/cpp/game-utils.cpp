#include "game-utils.h"
#include <unistd.h>
#include <signal.h>
#include "log.h"

static void PerformRestart(JNIEnv *env) {
    jclass activityThreadClass = env->FindClass("android/app/ActivityThread");
    jmethodID currentApplicationMethod = env->GetStaticMethodID(activityThreadClass, "currentApplication", "()Landroid/app/Application;");
    jobject context = env->CallStaticObjectMethod(activityThreadClass, currentApplicationMethod);

    jclass contextClass = env->GetObjectClass(context);
    jmethodID getPackageManagerMethod = env->GetMethodID(contextClass, "getPackageManager", "()Landroid/content/pm/PackageManager;");
    jobject packageManager = env->CallObjectMethod(context, getPackageManagerMethod);

    jmethodID getPackageNameMethod = env->GetMethodID(contextClass, "getPackageName", "()Ljava/lang/String;");
    jstring packageName = (jstring)env->CallObjectMethod(context, getPackageNameMethod);

    jclass packageManagerClass = env->GetObjectClass(packageManager);
    jmethodID getLaunchIntentMethod = env->GetMethodID(packageManagerClass, "getLaunchIntentForPackage", "(Ljava/lang/String;)Landroid/content/Intent;");
    jobject intent = env->CallObjectMethod(packageManager, getLaunchIntentMethod, packageName);

    jclass intentClass = env->GetObjectClass(intent);
    jmethodID addFlagsMethod = env->GetMethodID(intentClass, "addFlags", "(I)Landroid/content/Intent;");
    int flags = 0x10000000 | 0x00008000; 
    env->CallObjectMethod(intent, addFlagsMethod, flags);

    jmethodID startActivityMethod = env->GetMethodID(contextClass, "startActivity", "(Landroid/content/Intent;)V");
    env->CallVoidMethod(context, startActivityMethod, intent);
    kill(getpid(), SIGKILL);
}

void RestartProcess(JavaVM *vm) {
    JNIEnv *env = nullptr;
    int getEnvStat = vm->GetEnv((void**)&env, JNI_VERSION_1_6);

    if (getEnvStat == JNI_EDETACHED) {
        PerformRestart(env);
        vm->DetachCurrentThread();
    } else if (getEnvStat == JNI_OK) {
        PerformRestart(env);
    }
}

void SetScreenSize(JavaVM *vm, MenuValue *menu) {
    if (!vm || !menu) return;

    JNIEnv *env = nullptr;
    int getEnvStat = vm->GetEnv((void**)&env, JNI_VERSION_1_6);
    bool didAttach = false;

    if (getEnvStat == JNI_EDETACHED) {
        if (vm->AttachCurrentThread(&env, nullptr) != 0) {
            return;
        }
        didAttach = true;
    }

    jclass unityPlayer = env->FindClass("com/unity3d/player/UnityPlayer");
    jfieldID currentActivityField = env->GetStaticFieldID(unityPlayer, "currentActivity", "Landroid/app/Activity;");
    jobject activity = env->GetStaticObjectField(unityPlayer, currentActivityField);

    jclass activityClass = env->GetObjectClass(activity);
    jmethodID getWindowManager = env->GetMethodID(activityClass, "getWindowManager", "()Landroid/view/WindowManager;");
    jobject windowManager = env->CallObjectMethod(activity, getWindowManager);

    jclass windowManagerClass = env->GetObjectClass(windowManager);
    jmethodID getDefaultDisplay = env->GetMethodID(windowManagerClass, "getDefaultDisplay", "()Landroid/view/Display;");
    jobject display = env->CallObjectMethod(windowManager, getDefaultDisplay);

    jclass displayClass = env->GetObjectClass(display);
    jclass displayMetricsClass = env->FindClass("android/util/DisplayMetrics");
    jobject metrics = env->AllocObject(displayMetricsClass);
    
    jmethodID getRealMetrics = env->GetMethodID(displayClass, "getRealMetrics", "(Landroid/util/DisplayMetrics;)V");
    env->CallVoidMethod(display, getRealMetrics, metrics);

    jfieldID widthPixelsField = env->GetFieldID(displayMetricsClass, "widthPixels", "I");
    jfieldID heightPixelsField = env->GetFieldID(displayMetricsClass, "heightPixels", "I");

    menu->screen_width = env->GetIntField(metrics, widthPixelsField);
    menu->screen_height = env->GetIntField(metrics, heightPixelsField);

    env->DeleteLocalRef(unityPlayer);
    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(windowManager);
    env->DeleteLocalRef(display);
    env->DeleteLocalRef(metrics);
    env->DeleteLocalRef(activityClass);
    env->DeleteLocalRef(windowManagerClass);
    env->DeleteLocalRef(displayClass);
    env->DeleteLocalRef(displayMetricsClass);

    if (didAttach) vm->DetachCurrentThread();
}