#include "il2cpp-resolver.h"
#include <cstring>
#include <cstdint>
#include <unistd.h>
#include <locale>
#include <codecvt>
#include "xdl.h"
#include "log.h"
#include "menu-config.h"

#define DO_API(r, n, p) r (*n) p = nullptr;
#include "il2cpp-api-functions.h"
#undef DO_API

uint64_t il2cpp_base = 0;

void il2cpp_api_init(void *handle) {
    #define DO_API(r, n, p) {                           \
        n = (r (*) p)xdl_sym(handle, #n, nullptr);      \
        if(!n) {                                        \
            LOGW("api not found %s", #n);               \
        }                                               \
    }
    #include "il2cpp-api-functions.h"
    #undef DO_API

    xdl_info_t info;
    xdl_info(handle, XDL_DI_DLINFO, &info);
    il2cpp_base = (uint64_t)info.dli_fbase;
    if (il2cpp_base) {
        LOGI("il2cpp_base: %" PRIx64"", il2cpp_base);
    } else {
        LOGE("failed to initialize il2cpp api.");
        return;
    }

    LOGI("waiting for il2cpp_init...");
    while (!il2cpp_is_vm_thread(nullptr)) {
        usleep(10000);
    }
    il2cpp_thread_attach(il2cpp_domain_get());
}

Il2CppImage *get_image(Il2CppDomain *domain, const char *dllName) {
    size_t assemblyCount = 0;
    const Il2CppAssembly **assemblies = il2cpp_domain_get_assemblies(domain, &assemblyCount);
    for (size_t i = 0; i < assemblyCount; ++i) {
        const Il2CppImage *cimage = il2cpp_assembly_get_image(assemblies[i]);
        const char *name = il2cpp_image_get_name(cimage);
        if (name && strcmp(name, dllName) == 0) return const_cast<Il2CppImage*>(cimage);
    }
    return nullptr;
}

const MethodInfo *get_method(Il2CppClass *klass, const char *name, int paramIndex, const char *paramName, const char *paramType) {
    void *iter = nullptr;
    const MethodInfo *method = nullptr;
    while ((method = il2cpp_class_get_methods(klass, &iter))) {
        const char *currentName = il2cpp_method_get_name(method);
        if (!currentName || strcmp(currentName, name) != 0) continue;
        if (paramIndex == -1) return method;
        int paramCount = il2cpp_method_get_param_count(method);
        if (paramIndex == paramCount) {
            if (!paramName && !paramType) return method;
        } else if (paramIndex < paramCount) {
            bool isMatch = true;
            if (paramName) {
                const char *argName = il2cpp_method_get_param_name(method, paramIndex);
                if (!argName || strcmp(argName, paramName) != 0) isMatch = false;
            }
            if (paramType && isMatch) {
                const Il2CppType *argType = il2cpp_method_get_param(method, paramIndex);
                Il2CppClass *argClass = il2cpp_class_from_type(argType);
                const char *argTypeName = argClass ? il2cpp_class_get_name(argClass) : nullptr;
                if (!argTypeName || strcmp(argTypeName, paramType) != 0) isMatch = false;
            }
            if (isMatch) return method;
        }
    }
    return nullptr;
}

std::string il2cpp_string_to_std_string(Il2CppString* str) {
    if (str == nullptr) return "";
    Il2CppChar* chars = il2cpp_string_chars(str);
    std::u16string u16(reinterpret_cast<const char16_t*>(chars));
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
    return converter.to_bytes(u16);
}
