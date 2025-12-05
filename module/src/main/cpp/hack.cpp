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
#include "game-utils.h"
#include "menu-input.h"
#include "menu.h"

static std::atomic<bool> hack_loaded = false;
static std::atomic<bool> payload_loaded = false;

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
};

static NativeBridgeCallbacks *callbacks = nullptr;

static std::string GetNativeBridgeLibrary() {
    auto value = std::array<char, PROP_VALUE_MAX>();
    __system_property_get("ro.dalvik.vm.native.bridge", value.data());
    return {value.data()};
}

const char *create_payload(int fd) {
    ftruncate(fd, (off_t) reserved.length);
    void *mem = mmap(nullptr, reserved.length, PROT_WRITE, MAP_SHARED, fd, 0);
    memcpy(mem, reserved.data, reserved.length);
    munmap(mem, reserved.length);
    munmap(reserved.data, reserved.length);
    static char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/self/fd/%d", fd);
    LOGI("[CHEAT] arm path %s", path);
    return path;
}

void *(*orig_loadLibrary)(const char *libpath, int flag) = nullptr;
void *proxy_loadLibrary(const char *libpath, int flag) {
    void *handle = orig_loadLibrary(libpath, flag);
    if (payload_loaded.exchange(true)) return handle;
    int fd = syscall(__NR_memfd_create, "anon", MFD_CLOEXEC);
    auto arm_handle = orig_loadLibrary(create_payload(fd), RTLD_NOW);
    auto init = (void(*)(JavaVM*, void*))callbacks->getTrampoline(arm_handle, "JNI_OnLoad", nullptr, 0);
    LOGI("[CHEAT] JNI_OnLoad %p", init);
    init(reserved.vm, (void*)&reserved);
    close(fd);
    return handle;
}

void *(*orig_loadLibraryExt)(const char *libpath, int flag, void *ns) = nullptr;
void *proxy_loadLibraryExt(const char *libpath, int flag, void *ns) {
    void *handle = orig_loadLibraryExt(libpath, flag, ns);
    if (payload_loaded.exchange(true)) return handle;
    int fd = syscall(__NR_memfd_create, "anon", MFD_CLOEXEC);
    auto arm_handle = orig_loadLibraryExt(create_payload(fd), RTLD_NOW, (void*)3);
    auto init = (void(*)(JavaVM*, void*))callbacks->getTrampoline(arm_handle, "JNI_OnLoad", nullptr, 0);
    LOGI("[CHEAT] JNI_OnLoad %p", init);
    init(reserved.vm, (void*)&reserved);
    close(fd);
    return handle;
}

bool NativeBridgeLoad(const char *game_data_dir, int api_level, void *data, size_t length, const std::string &lib_dir) {
    auto libart = xdl_open("libart.so", XDL_TRY_FORCE_LOAD);
    auto JNI_GetCreatedJavaVMs = (jint(*)(JavaVM**, jsize, jsize*))xdl_sym(libart, "JNI_GetCreatedJavaVMs", nullptr);
    LOGI("[CHEAT] JNI_GetCreatedJavaVMs %p", JNI_GetCreatedJavaVMs);
    JavaVM *vms_buf[1];
    jsize num_vms;
    jint status = JNI_GetCreatedJavaVMs(vms_buf, 1, &num_vms);
    if (status == JNI_OK && num_vms > 0) {
        reserved.vm = vms_buf[0];
    } else {
        LOGE("[CHEAT] GetCreatedJavaVMs error");
        return false;
    }

#if defined(__arm__) || defined(__aarch64__)
    return false;
#endif

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
        LOGI("[CHEAT] native bridge %p", nb);
        callbacks = (NativeBridgeCallbacks*)xdl_sym(nb, "NativeBridgeItf", nullptr);
        if (callbacks) {
            LOGI("[CHEAT] NativeBridgeLoadLibrary %p", callbacks->loadLibrary);
            LOGI("[CHEAT] NativeBridgeLoadLibraryExt %p", callbacks->loadLibraryExt);
            LOGI("[CHEAT] NativeBridgeGetTrampoline %p", callbacks->getTrampoline);

            api_level < 26 ?
                DobbyHook(callbacks->loadLibrary, (void*)proxy_loadLibrary, (void**)&orig_loadLibrary) :
                DobbyHook(callbacks->loadLibraryExt, (void*)proxy_loadLibraryExt, (void**)&orig_loadLibraryExt);
            return true;
        }
    }
    return false;
}

void (*orig_il2cpp_init)(const char *domain_name) = nullptr;
void proxy_il2cpp_init(const char *domain_name) {
    orig_il2cpp_init(domain_name);
    
    il2cpp_dump(reserved.game_data_dir);
    il2cpp_prepare(reserved.game_data_dir);

    RestartProcess(reserved.vm);
}

jint (*orig_JNI_OnLoad)(JavaVM *vm, void *data) = nullptr;
jint proxy_JNI_OnLoad(JavaVM *vm, void *data) {
    SetScreenSize(reserved.vm, reserved.menu_value);
    return orig_JNI_OnLoad(vm, data);
}

void *(*orig_dlopen)(const char *filename, int flags, const void* caller_addr) = nullptr;
void *proxy_dlopen(const char *filename, int flags, const void* caller_addr) {
    /* block anti-cheat if needed
    if (filename && strstr(filename, "anogs")) {
        LOGI("[BLOCK] %s", filename);
        return nullptr;
    }
    */
    
    void *handle = orig_dlopen(filename, flags, caller_addr);
    if (filename && strstr(filename, "libunity.so")) {
        void *unity_handle = xdl_open("libunity.so", 0);
        void *sym = xdl_sym(unity_handle, "JNI_OnLoad", nullptr);
        DobbyHook(sym, (void*)proxy_JNI_OnLoad, (void**)&orig_JNI_OnLoad);
    }

    if (filename && strstr(filename, "libil2cpp.so")) {
        if (hack_loaded.exchange(true)) return handle;

        void *il2cpp_handle = xdl_open("libil2cpp.so", 0);
        il2cpp_api_init(il2cpp_handle);

        dobby_enable_near_branch_trampoline();

        if (access(std::string(reserved.game_data_dir).append("/files/PreloadMap.bin").c_str(), F_OK) != 0) {
            void *sym = xdl_sym(il2cpp_handle, "il2cpp_init", nullptr);
            DobbyHook(sym, (void*)proxy_il2cpp_init, (void**)&orig_il2cpp_init);
            LOGI("[CHEAT] il2cpp_init %p", sym);
        } else {
            il2cpp_hook(reserved.game_data_dir);
        }
    }
    return handle;
}

void hack_prepare(const char *game_data_dir, void *data, size_t length, const std::string &lib_dir) {
    LOGI("[CHEAT] hack thread: %d", gettid());
    int api_level = android_get_device_api_level();
    LOGI("[CHEAT] api level: %d", api_level);

    reserved.game_data_dir = game_data_dir;
    reserved.data = data;
    reserved.length = length;
    reserved.vm = nullptr;

    reserved.menu_value = &g_menu_value;

    DobbyHook(xdl_sym(xdl_open("libEGL.so", XDL_TRY_FORCE_LOAD), "eglSwapBuffers", nullptr), (void*)proxy_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
    DobbyHook(DobbySymbolResolver(nullptr, "_ZN7android13InputConsumer21initializeMotionEventEPNS_11MotionEventEPKNS_12InputMessageE"), (void*)proxy_input, (void**)&orig_input);

    if (!NativeBridgeLoad(game_data_dir, api_level, data, length, lib_dir)) {
        void *linker_handle = xdl_open("/system/bin/linker64", XDL_TRY_FORCE_LOAD);
        if (!linker_handle) linker_handle = xdl_open("/system/bin/linker", XDL_TRY_FORCE_LOAD);
        DobbyHook(xdl_sym(linker_handle, "__loader_dlopen", nullptr), (void*)proxy_dlopen, (void**)&orig_dlopen);
    }
}

#if defined(__arm__) || defined(__aarch64__)

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *data) {
    reserved = *((ReservedData*)data);
    reserved.vm = vm;

    DobbyHook(xdl_sym(xdl_open("libdl.so", XDL_TRY_FORCE_LOAD), "dlopen", nullptr), (void*)proxy_dlopen, (void**)&orig_dlopen);
    return JNI_VERSION_1_6;
}

#endif