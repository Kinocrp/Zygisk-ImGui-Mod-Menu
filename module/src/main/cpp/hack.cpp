#include "hack.h"
#include "il2cpp-hook.h"
#include "log.h"
#include "xdl.h"
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <sys/system_properties.h>
#include <dlfcn.h>
#include <jni.h>
#include <thread>
#include <sys/mman.h>
#include <linux/unistd.h>
#include <array>
#include <atomic>
#include <fcntl.h>
#include "dobby.h"

#include "reserved-data.h"
#include "menu-input.h"
#include "menu-data.h"
#include "menu.h"
#include "menu-utils.h"

void hack_start(const char *game_data_dir) {
    while (true) {
        void *handle = xdl_open("libil2cpp.so", 0);
        if (handle) {
            il2cpp_api_init(handle);
            il2cpp_dump(game_data_dir);
            il2cpp_hook();
            break;
        }
        sleep(1);
    }
    GetPhysicalScreenSize(GetEnv(reserved->vm), reserved->menu->screen_width, reserved->menu->screen_height);
    LOGI("screen size: %d x %d", reserved->menu->screen_width, reserved->menu->screen_height);
}

struct PayloadData {
    void *data;
    size_t length;
    JavaVM *vm;
} *payload = nullptr;

static std::string GetNativeBridgeLibrary() {
    auto value = std::array<char, PROP_VALUE_MAX>();
    __system_property_get("ro.dalvik.vm.native.bridge", value.data());
    return {value.data()};
}

struct NativeBridgeCallbacks {
    uint32_t version;
    void *initialize;

    void *loadLibrary;

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

    void *loadLibraryExt;
} *callbacks = nullptr;

const char *create_payload(int fd) {
    ftruncate(fd, (off_t) payload->length);
    void *mem = mmap(nullptr, payload->length, PROT_WRITE, MAP_SHARED, fd, 0);
    memcpy(mem, payload->data, payload->length);
    munmap(mem, payload->length);
    munmap(payload->data, payload->length);
    static char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/self/fd/%d", fd);
    LOGI("arm path %s", path);
    return path;
}

void *(*orig_loadLibrary)(const char *libpath, int flag) = nullptr;
void *proxy_loadLibrary(const char *libpath, int flag) {
    int fd = syscall(__NR_memfd_create, "anon", MFD_CLOEXEC);
    auto arm_handle = orig_loadLibrary(create_payload(fd), RTLD_NOW);
    auto init = (void(*)(JavaVM*, void*))callbacks->getTrampoline(arm_handle, "JNI_OnLoad", nullptr, 0);
    LOGI("JNI_OnLoad %p", init);
    init(payload->vm, reserved);
    close(fd);
    void *ret = orig_loadLibrary(libpath, flag);
    DobbyDestroy(callbacks->loadLibrary);
    return ret;
}

void *(*orig_loadLibraryExt)(const char *libpath, int flag, void *ns) = nullptr;
void *proxy_loadLibraryExt(const char *libpath, int flag, void *ns) {
    int fd = syscall(__NR_memfd_create, "anon", MFD_CLOEXEC);
    auto arm_handle = orig_loadLibraryExt(create_payload(fd), RTLD_NOW, (void*)3);
    auto init = (void(*)(JavaVM*, void*))callbacks->getTrampoline(arm_handle, "JNI_OnLoad", nullptr, 0);
    LOGI("JNI_OnLoad %p", init);
    init(payload->vm, reserved);
    close(fd);
    void *ret = orig_loadLibraryExt(libpath, flag, ns);
    DobbyDestroy(callbacks->loadLibraryExt);
    return ret;
}

bool NativeBridgeLoad(const char *game_data_dir, int api_level, void *data, size_t length, const std::string &lib_dir) {
    auto libart = xdl_open("libart.so", XDL_TRY_FORCE_LOAD);
    auto JNI_GetCreatedJavaVMs = (jint(*)(JavaVM**, jsize, jsize*))xdl_sym(libart, "JNI_GetCreatedJavaVMs", nullptr);
    LOGI("JNI_GetCreatedJavaVMs %p", JNI_GetCreatedJavaVMs);
    JavaVM *vms_buf[1];
    jsize num_vms;
    jint status = JNI_GetCreatedJavaVMs(vms_buf, 1, &num_vms);
    if (status == JNI_OK && num_vms > 0) {
        payload = new PayloadData{data, length, vms_buf[0]};
    } else {
        LOGE("GetCreatedJavaVMs error");
        return false;
    }

    reserved = new ReservedData{game_data_dir, vms_buf[0], &menu};

#if defined(__arm__) || defined(__aarch64__)
    return false;
#endif

    if (lib_dir.empty()) {
        LOGE("GetLibDir error");
        return false;
    }
    if (lib_dir.find("/lib/x86") != std::string::npos) {
        LOGI("no need NativeBridge");
        munmap(data, length);
        return false;
    }

    auto nb = xdl_open("libhoudini.so", XDL_TRY_FORCE_LOAD);
    if (!nb) {
        auto native_bridge = GetNativeBridgeLibrary();
        LOGI("native bridge: %s", native_bridge.data());
        nb = xdl_open(native_bridge.data(), XDL_TRY_FORCE_LOAD);
    }
    if (nb) {
        LOGI("nb %p", nb);
        callbacks = (NativeBridgeCallbacks*)xdl_sym(nb, "NativeBridgeItf", nullptr);
        if (callbacks) {
            LOGI("NativeBridgeLoadLibrary %p", callbacks->loadLibrary);
            LOGI("NativeBridgeLoadLibraryExt %p", callbacks->loadLibraryExt);
            LOGI("NativeBridgeGetTrampoline %p", callbacks->getTrampoline);

            if (api_level < 26) {
                DobbyHook(callbacks->loadLibrary, (void*)proxy_loadLibrary, (void**)&orig_loadLibrary);
            } else {
                DobbyHook(callbacks->loadLibraryExt, (void*)proxy_loadLibraryExt, (void**)&orig_loadLibraryExt);
            }
            return true;
        }
    }
    return false;
}

void hack_prepare(const char *game_data_dir, void *data, size_t length, const std::string &lib_dir) {
    LOGI("hack thread: %d", gettid());
    int api_level = android_get_device_api_level();
    LOGI("api level: %d", api_level);

    DobbyHook(xdl_sym(xdl_open("libEGL.so", XDL_TRY_FORCE_LOAD), "eglSwapBuffers", nullptr), (void*)proxy_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
    DobbyHook(DobbySymbolResolver("libinput.so", "_ZN7android13InputConsumer21initializeMotionEventEPNS_11MotionEventEPKNS_12InputMessageE"), (void*)proxy_input, (void**)&orig_input);

    if (!NativeBridgeLoad(game_data_dir, api_level, data, length, lib_dir)) {
        std::thread hack_thread(hack_start, reserved->game_data_dir);
        hack_thread.detach();
    }
}

#if defined(__arm__) || defined(__aarch64__)

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *data) {
    reserved = (ReservedData*)data;
    reserved->vm = vm;
    std::thread hack_thread(hack_start, reserved->game_data_dir);
    hack_thread.detach();
    return JNI_VERSION_1_6;
}

#endif