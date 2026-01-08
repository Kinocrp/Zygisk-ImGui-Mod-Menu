#pragma once

#include <string>
#include <cinttypes>
#include <type_traits>
#include "il2cpp-tabledefs.h"
#include "il2cpp-class.h"

#define DO_API(r, n, p) extern r (*n) p;
#include "il2cpp-api-functions.h"
#undef DO_API

extern uint64_t il2cpp_base;

void il2cpp_api_init(void *handle);

Il2CppImage *get_image(Il2CppDomain *domain, const char *dllName);
const MethodInfo *get_method(Il2CppClass *klass, const char *name, int paramIndex = -1, const char *paramName = nullptr, const char *paramType = nullptr);

template<typename T>
void set_field_value(void *obj, size_t offset, T value, bool isStruct = false) {
    if (!obj) return;
    if (isStruct) offset -= il2cpp_object_header_size();
    *reinterpret_cast<T*>((uintptr_t)obj + offset) = value;
}

template<typename T>
T get_field_value(void *obj, size_t offset, bool isStruct = false) {
    if (!obj) return T();
    if (isStruct) offset -= il2cpp_object_header_size();
    return *reinterpret_cast<T*>((uintptr_t)obj + offset);
}

template<typename T>
T* get_field_pointer(void *obj, size_t offset, bool isStruct = false) {
    if (!obj) return nullptr;
    if (isStruct) offset -= il2cpp_object_header_size();
    return reinterpret_cast<T*>((uintptr_t)obj + offset);
}

template<typename T>
T get_array_element(void *obj, uint32_t index) {
    if (!obj) return T();

    auto arr = reinterpret_cast<Il2CppArray*>(obj);
    auto length = *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(arr) + il2cpp_offset_of_array_length_in_array_object_header());
    if (index >= length) return T();

    auto start = reinterpret_cast<uintptr_t>(arr) + il2cpp_array_object_header_size();

    if constexpr (std::is_pointer_v<T>) {
        auto data = reinterpret_cast<void**>(start);
        return reinterpret_cast<T>(data[index]);
    } else {
        auto data = reinterpret_cast<char*>(start);
        return *reinterpret_cast<T*>(data + index * sizeof(T));
    }
}

std::string il2cpp_string_to_std_string(Il2CppString* str);
