#pragma once

#include "src/il2cpp-class.h"
#include "src/il2cpp-tabledefs.h"
#include <string>
#include <cinttypes>
#include <type_traits>

namespace IL2CPP {

    extern uint64_t il2cpp_base;
    extern Il2CppDomain *domain;

    namespace API {
        #define DO_API(r, n, p) extern r (*n) p
        #include "il2cpp-api-functions.h"
        #undef DO_API
    }

    bool Init();
    Il2CppThread *Attach();
    void Dump(std::string outDir);
    Il2CppImage *GetImage(const char *dllName);
    const MethodInfo *GetMethod(Il2CppClass *klass, const char *name, int paramIndex = -1, const char *paramName = nullptr, const char *paramType = nullptr);
    std::string Il2CppToString(Il2CppString *str);

    template<typename T>
    void SetFieldValue(void *obj, size_t offset, T value, bool isStruct = false) {
        if (!obj) return;
        if (isStruct) offset -= API::il2cpp_object_header_size();
        *reinterpret_cast<T*>((uintptr_t)obj + offset) = value;
    }

    template<typename T>
    T GetFieldValue(void *obj, size_t offset, bool isStruct = false) {
        if (!obj) return T();
        if (isStruct) offset -= API::il2cpp_object_header_size();
        return *reinterpret_cast<T*>((uintptr_t)obj + offset);
    }

    template<typename T>
    T *GetFieldPointer(void *obj, size_t offset, bool isStruct = false) {
        if (!obj) return nullptr;
        if (isStruct) offset -= API::il2cpp_object_header_size();
        return reinterpret_cast<T*>((uintptr_t)obj + offset);
    }

    template<typename T>
    T GetArrayElement(void *obj, uint32_t index) {
        if (!obj) return T();

        auto arr = reinterpret_cast<Il2CppArray*>(obj);
        auto length = *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(arr) + API::il2cpp_offset_of_array_length_in_array_object_header());
        if (index >= length) return T();

        auto start = reinterpret_cast<uintptr_t>(arr) + API::il2cpp_array_object_header_size();

        if constexpr (std::is_pointer<T>::value) {
            auto data = reinterpret_cast<void**>(start);
            return reinterpret_cast<T>(data[index]);
        } else {
            auto data = reinterpret_cast<char*>(start);
            return *reinterpret_cast<T*>(data + index * sizeof(T));
        }
    }
}