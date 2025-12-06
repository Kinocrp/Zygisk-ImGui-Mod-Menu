#include "hack.h"
#include "il2cpp-hook.h"
#include "log.h"
#include "xdl.h"
#include "dobby.h"
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <sys/system_properties.h>
#include <dlfcn.h>
#include <jni.h>
#include <sys/mman.h>
#include <linux/unistd.h>
#include <array>
#include <atomic>
#include <thread>
#include "reserved-data.h"
#include "menu-input.h"
#include "menu.h"

void hack_start(const char *game_data_dir) {
    bool load = false;
    for (int i = 0; i < 10; i++) {
        void *handle = xdl_open("libil2cpp.so", 0);
        if (handle) {
            load = true;
            il2cpp_api_init(handle);
            il2cpp_dump(game_data_dir);
            il2cpp_hook();
            break;
        } else {
            sleep(1);
        }
    }
    if (!load) {
        LOGI("[ERROR] libil2cpp.so not found in thread %d", gettid());
    }
}

std::string GetLibDir(JavaVM *vms) {
    JNIEnv *env = nullptr;
    vms->AttachCurrentThread(&env, nullptr);
    jclass activity_thread_clz = env->FindClass("android/app/ActivityThread");
    if (activity_thread_clz != nullptr) {
        jmethodID currentApplicationId = env->GetStaticMethodID(activity_thread_clz, "currentApplication", "()Landroid/app/Application;");
        if (currentApplicationId) {
            jobject application = env->CallStaticObjectMethod(activity_thread_clz, currentApplicationId);
            jclass application_clazz = env->GetObjectClass(application);
            if (application_clazz) {
                jmethodID get_application_info = env->GetMethodID(application_clazz, "getApplicationInfo", "()Landroid/content/pm/ApplicationInfo;");
                if (get_application_info) {
                    jobject application_info = env->CallObjectMethod(application, get_application_info);
                    jfieldID native_library_dir_id = env->GetFieldID(env->GetObjectClass(application_info), "nativeLibraryDir", "Ljava/lang/String;");
                    if (native_library_dir_id) {
                        auto native_library_dir_jstring = (jstring) env->GetObjectField(application_info, native_library_dir_id);
                        auto path = env->GetStringUTFChars(native_library_dir_jstring, nullptr);
                        LOGI("[CHEAT] lib dir %s", path);
                        std::string lib_dir(path);
                        env->ReleaseStringUTFChars(native_library_dir_jstring, path);
                        return lib_dir;
                    } else {
                        LOGE("[ERROR] nativeLibraryDir not found");
                    }
                } else {
                    LOGE("[ERROR] getApplicationInfo not found");
                }
            } else {
                LOGE("[ERROR] application class not found");
            }
        } else {
            LOGE("[ERROR] currentApplication not found");
        }
    } else {
        LOGE("[ERROR] ActivityThread not found");
    }
    return {};
}

static std::string GetNativeBridgeLibrary() {
    auto value = std::array<char, PROP_VALUE_MAX>();
    __system_property_get("ro.dalvik.vm.native.bridge", value.data());
    return {value.data()};
}

struct NativeBridgeCallbacks {
    uint32_t version;
    void *initialize;

    void *(*loadLibrary)(const char *libpath, int flag);

    void *(*getTrampoline)(void *handle, const char *name, const char *shorty, uint32_t len);

    void *isSupported;
    void *getAppEnv;
    void *isCompatibleWith;
    void *getSignalHandler;
    void *unloadLibrary;
    void *getError;
    void *isPathSupported;
    void *initAnonymousNamespace;
    void *createNamespace;
    void *linkNamespaces;

    void *(*loadLibraryExt)(const char *libpath, int flag, void *ns);
};

bool NativeBridgeLoad(const char *game_data_dir, int api_level, void *data, size_t length) {
    // wait for unity to load
    sleep(5);

    auto libart = xdl_open("libart.so", XDL_TRY_FORCE_LOAD);
    auto JNI_GetCreatedJavaVMs = (jint(*)(JavaVM**, jsize, jsize*))xdl_sym(libart, "JNI_GetCreatedJavaVMs", nullptr);
    LOGI("[CHEAT] JNI_GetCreatedJavaVMs %p", JNI_GetCreatedJavaVMs);
    JavaVM *vms_buf[1];
    JavaVM *vms;
    jsize num_vms;
    jint status = JNI_GetCreatedJavaVMs(vms_buf, 1, &num_vms);
    if (status == JNI_OK && num_vms > 0) {
        vms = vms_buf[0];
    } else {
        LOGE("[CHEAT] GetCreatedJavaVMs error");
        return false;
    }

#if defined(__arm__) || defined(__aarch64__)
    return false;
#endif

    auto lib_dir = GetLibDir(vms);
    if (lib_dir.empty()) {
        LOGE("[CHEAT] GetLibDir error");
        return false;
    }
    if (lib_dir.find("/lib/x86") != std::string::npos) {
        LOGI("[CHEAT] no need NativeBridge");
        munmap(data, length);
        return false;
    }

    auto nb = xdl_open("libhoudini.so", XDL_TRY_FORCE_LOAD);
    if (!nb) {
        auto native_bridge = GetNativeBridgeLibrary();
        LOGI("[CHEAT] native bridge: %s", native_bridge.data());
        nb = xdl_open(native_bridge.data(), XDL_TRY_FORCE_LOAD);
    }
    if (nb) {
        LOGI("[CHEAT] native bridge: %p", nb);
        auto callbacks = (NativeBridgeCallbacks*)xdl_sym(nb, "NativeBridgeItf", nullptr);
        if (callbacks) {
            LOGI("[CHEAT] NativeBridgeLoadLibrary %p", callbacks->loadLibrary);
            LOGI("[CHEAT] NativeBridgeLoadLibraryExt %p", callbacks->loadLibraryExt);
            LOGI("[CHEAT] NativeBridgeGetTrampoline %p", callbacks->getTrampoline);

            int fd = syscall(__NR_memfd_create, "anon", MFD_CLOEXEC);
            ftruncate(fd, (off_t) length);
            void *mem = mmap(nullptr, length, PROT_WRITE, MAP_SHARED, fd, 0);
            memcpy(mem, data, length);
            munmap(mem, length);
            munmap(data, length);
            char path[PATH_MAX];
            snprintf(path, PATH_MAX, "/proc/self/fd/%d", fd);
            LOGI("[CHEAT] arm path %s", path);

            void *arm_handle;
            if (api_level >= 26) {
                arm_handle = callbacks->loadLibraryExt(path, RTLD_NOW, (void*)3);
            } else {
                arm_handle = callbacks->loadLibrary(path, RTLD_NOW);
            }
            if (arm_handle) {
                LOGI("[CHEAT] arm handle %p", arm_handle);
                auto init = (void(*)(JavaVM*, void*))callbacks->getTrampoline(arm_handle, "JNI_OnLoad", nullptr, 0);
                LOGI("[CHEAT] JNI_OnLoad %p", init);
                init(vms, (void*)&reserved);
                return true;
            }
            close(fd);
        }
    }
    return false;
}

void hack_prepare(const char *game_data_dir, void *data, size_t length) {
    LOGI("[CHEAT] hack thread: %d", gettid());
    int api_level = android_get_device_api_level();
    LOGI("[CHEAT] api level: %d", api_level);

    reserved.game_data_dir = game_data_dir;
    reserved.menu_value = &g_menu_value;

    DobbyHook(xdl_sym(xdl_open("libEGL.so", XDL_TRY_FORCE_LOAD), "eglSwapBuffers", nullptr), (void*)proxy_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
    DobbyHook(DobbySymbolResolver(nullptr, "_ZN7android13InputConsumer21initializeMotionEventEPNS_11MotionEventEPKNS_12InputMessageE"), (void*)proxy_input, (void**)&orig_input);

    if (!NativeBridgeLoad(game_data_dir, api_level, data, length)) {
        hack_start(game_data_dir);
    }
}

#if defined(__arm__) || defined(__aarch64__)

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *data) {
    reserved = *((ReservedData*)data);
    std::thread hack_thread(hack_start, reserved.game_data_dir);
    hack_thread.detach();
    return JNI_VERSION_1_6;
}

#endif