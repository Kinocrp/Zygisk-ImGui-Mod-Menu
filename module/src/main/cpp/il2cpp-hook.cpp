#include "il2cpp-hook.h"
#include "log.h"
#include "xdl.h"
#include "dobby.h"
#include <dlfcn.h>
#include <cstdlib>
#include <cstring>
#include <cinttypes>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <unordered_map>
#include "il2cpp-tabledefs.h"
#include "il2cpp-class.h"
#include "reserved-data.h"

#define DO_API(r, n, p) r (*n) p

#include "il2cpp-api-functions.h"

#undef DO_API

static uint64_t il2cpp_base = 0;
static std::unordered_map<std::string, uint64_t> PreloadMap;

void init_il2cpp_api(void *handle) {
#define DO_API(r, n, p) {                      \
    n = (r (*) p)xdl_sym(handle, #n, nullptr); \
    if(!n) {                                   \
        LOGW("api not found %s", #n);          \
    }                                          \
}

#include "il2cpp-api-functions.h"

#undef DO_API
}

std::string get_method_modifier(uint32_t flags) {
    std::stringstream outPut;
    auto access = flags & METHOD_ATTRIBUTE_MEMBER_ACCESS_MASK;
    switch (access) {
        case METHOD_ATTRIBUTE_PRIVATE:
            outPut << "private ";
            break;
        case METHOD_ATTRIBUTE_PUBLIC:
            outPut << "public ";
            break;
        case METHOD_ATTRIBUTE_FAMILY:
            outPut << "protected ";
            break;
        case METHOD_ATTRIBUTE_ASSEM:
        case METHOD_ATTRIBUTE_FAM_AND_ASSEM:
            outPut << "internal ";
            break;
        case METHOD_ATTRIBUTE_FAM_OR_ASSEM:
            outPut << "protected internal ";
            break;
    }
    if (flags & METHOD_ATTRIBUTE_STATIC) {
        outPut << "static ";
    }
    if (flags & METHOD_ATTRIBUTE_ABSTRACT) {
        outPut << "abstract ";
        if ((flags & METHOD_ATTRIBUTE_VTABLE_LAYOUT_MASK) == METHOD_ATTRIBUTE_REUSE_SLOT) {
            outPut << "override ";
        }
    } else if (flags & METHOD_ATTRIBUTE_FINAL) {
        if ((flags & METHOD_ATTRIBUTE_VTABLE_LAYOUT_MASK) == METHOD_ATTRIBUTE_REUSE_SLOT) {
            outPut << "sealed override ";
        }
    } else if (flags & METHOD_ATTRIBUTE_VIRTUAL) {
        if ((flags & METHOD_ATTRIBUTE_VTABLE_LAYOUT_MASK) == METHOD_ATTRIBUTE_NEW_SLOT) {
            outPut << "virtual ";
        } else {
            outPut << "override ";
        }
    }
    if (flags & METHOD_ATTRIBUTE_PINVOKE_IMPL) {
        outPut << "extern ";
    }
    return outPut.str();
}

bool _il2cpp_type_is_byref(const Il2CppType *type) {
    auto byref = type->byref;
    if (il2cpp_type_is_byref) {
        byref = il2cpp_type_is_byref(type);
    }
    return byref;
}

std::string dump_method(Il2CppClass *klass) {
    std::stringstream outPut;
    outPut << "\n\t// Methods\n";
    void *iter = nullptr;
    while (auto method = il2cpp_class_get_methods(klass, &iter)) {
        if (method->methodPointer) {
            outPut << "\t// RVA: 0x";
            outPut << std::hex << (uint64_t) method->methodPointer - il2cpp_base;
            outPut << " VA: 0x";
            outPut << std::hex << (uint64_t) method->methodPointer;
        } else {
            outPut << "\t// RVA: 0x VA: 0x0";
        }
        /*if (method->slot != 65535) {
            outPut << " Slot: " << std::dec << method->slot;
        }*/
        outPut << "\n\t";
        uint32_t iflags = 0;
        auto flags = il2cpp_method_get_flags(method, &iflags);
        outPut << get_method_modifier(flags);
        auto return_type = il2cpp_method_get_return_type(method);
        if (_il2cpp_type_is_byref(return_type)) {
            outPut << "ref ";
        }
        auto return_class = il2cpp_class_from_type(return_type);
        outPut << il2cpp_class_get_name(return_class) << " " << il2cpp_method_get_name(method) << "(";
        auto param_count = il2cpp_method_get_param_count(method);
        for (int i = 0; i < param_count; ++i) {
            auto param = il2cpp_method_get_param(method, i);
            auto attrs = param->attrs;
            if (_il2cpp_type_is_byref(param)) {
                if (attrs & PARAM_ATTRIBUTE_OUT && !(attrs & PARAM_ATTRIBUTE_IN)) {
                    outPut << "out ";
                } else if (attrs & PARAM_ATTRIBUTE_IN && !(attrs & PARAM_ATTRIBUTE_OUT)) {
                    outPut << "in ";
                } else {
                    outPut << "ref ";
                }
            } else {
                if (attrs & PARAM_ATTRIBUTE_IN) {
                    outPut << "[In] ";
                }
                if (attrs & PARAM_ATTRIBUTE_OUT) {
                    outPut << "[Out] ";
                }
            }
            auto parameter_class = il2cpp_class_from_type(param);
            outPut << il2cpp_class_get_name(parameter_class) << " " << il2cpp_method_get_param_name(method, i);
            outPut << ", ";
        }
        if (param_count > 0) {
            outPut.seekp(-2, outPut.cur);
        }
        outPut << ") { }\n";
    }
    return outPut.str();
}

std::string dump_property(Il2CppClass *klass) {
    std::stringstream outPut;
    outPut << "\n\t// Properties\n";
    void *iter = nullptr;
    while (auto prop_const = il2cpp_class_get_properties(klass, &iter)) {
        auto prop = const_cast<PropertyInfo *>(prop_const);
        auto get = il2cpp_property_get_get_method(prop);
        auto set = il2cpp_property_get_set_method(prop);
        auto prop_name = il2cpp_property_get_name(prop);
        outPut << "\t";
        Il2CppClass *prop_class = nullptr;
        uint32_t iflags = 0;
        if (get) {
            outPut << get_method_modifier(il2cpp_method_get_flags(get, &iflags));
            prop_class = il2cpp_class_from_type(il2cpp_method_get_return_type(get));
        } else if (set) {
            outPut << get_method_modifier(il2cpp_method_get_flags(set, &iflags));
            auto param = il2cpp_method_get_param(set, 0);
            prop_class = il2cpp_class_from_type(param);
        }
        if (prop_class) {
            outPut << il2cpp_class_get_name(prop_class) << " " << prop_name << " { ";
            if (get) {
                outPut << "get; ";
            }
            if (set) {
                outPut << "set; ";
            }
            outPut << "}\n";
        } else {
            if (prop_name) {
                outPut << " // unknown property " << prop_name;
            }
        }
    }
    return outPut.str();
}

std::string dump_field(Il2CppClass *klass) {
    std::stringstream outPut;
    outPut << "\n\t// Fields\n";
    auto is_enum = il2cpp_class_is_enum(klass);
    void *iter = nullptr;
    while (auto field = il2cpp_class_get_fields(klass, &iter)) {
        outPut << "\t";
        auto attrs = il2cpp_field_get_flags(field);
        auto access = attrs & FIELD_ATTRIBUTE_FIELD_ACCESS_MASK;
        switch (access) {
            case FIELD_ATTRIBUTE_PRIVATE:
                outPut << "private ";
                break;
            case FIELD_ATTRIBUTE_PUBLIC:
                outPut << "public ";
                break;
            case FIELD_ATTRIBUTE_FAMILY:
                outPut << "protected ";
                break;
            case FIELD_ATTRIBUTE_ASSEMBLY:
            case FIELD_ATTRIBUTE_FAM_AND_ASSEM:
                outPut << "internal ";
                break;
            case FIELD_ATTRIBUTE_FAM_OR_ASSEM:
                outPut << "protected internal ";
                break;
        }
        if (attrs & FIELD_ATTRIBUTE_LITERAL) {
            outPut << "const ";
        } else {
            if (attrs & FIELD_ATTRIBUTE_STATIC) {
                outPut << "static ";
            }
            if (attrs & FIELD_ATTRIBUTE_INIT_ONLY) {
                outPut << "readonly ";
            }
        }
        auto field_type = il2cpp_field_get_type(field);
        auto field_class = il2cpp_class_from_type(field_type);
        outPut << il2cpp_class_get_name(field_class) << " " << il2cpp_field_get_name(field);
        if (attrs & FIELD_ATTRIBUTE_LITERAL && is_enum) {
            uint64_t val = 0;
            il2cpp_field_static_get_value(field, &val);
            outPut << " = " << std::dec << val;
        }
        outPut << "; // 0x" << std::hex << il2cpp_field_get_offset(field) << "\n";
    }
    return outPut.str();
}

std::string dump_type(const Il2CppType *type) {
    std::stringstream outPut;
    auto *klass = il2cpp_class_from_type(type);
    outPut << "\n// Namespace: " << il2cpp_class_get_namespace(klass) << "\n";
    auto flags = il2cpp_class_get_flags(klass);
    if (flags & TYPE_ATTRIBUTE_SERIALIZABLE) {
        outPut << "[Serializable]\n";
    }
    auto is_valuetype = il2cpp_class_is_valuetype(klass);
    auto is_enum = il2cpp_class_is_enum(klass);
    auto visibility = flags & TYPE_ATTRIBUTE_VISIBILITY_MASK;
    switch (visibility) {
        case TYPE_ATTRIBUTE_PUBLIC:
        case TYPE_ATTRIBUTE_NESTED_PUBLIC:
            outPut << "public ";
            break;
        case TYPE_ATTRIBUTE_NOT_PUBLIC:
        case TYPE_ATTRIBUTE_NESTED_FAM_AND_ASSEM:
        case TYPE_ATTRIBUTE_NESTED_ASSEMBLY:
            outPut << "internal ";
            break;
        case TYPE_ATTRIBUTE_NESTED_PRIVATE:
            outPut << "private ";
            break;
        case TYPE_ATTRIBUTE_NESTED_FAMILY:
            outPut << "protected ";
            break;
        case TYPE_ATTRIBUTE_NESTED_FAM_OR_ASSEM:
            outPut << "protected internal ";
            break;
    }
    if (flags & TYPE_ATTRIBUTE_ABSTRACT && flags & TYPE_ATTRIBUTE_SEALED) {
        outPut << "static ";
    } else if (!(flags & TYPE_ATTRIBUTE_INTERFACE) && flags & TYPE_ATTRIBUTE_ABSTRACT) {
        outPut << "abstract ";
    } else if (!is_valuetype && !is_enum && flags & TYPE_ATTRIBUTE_SEALED) {
        outPut << "sealed ";
    }
    if (flags & TYPE_ATTRIBUTE_INTERFACE) {
        outPut << "interface ";
    } else if (is_enum) {
        outPut << "enum ";
    } else if (is_valuetype) {
        outPut << "struct ";
    } else {
        outPut << "class ";
    }
    outPut << il2cpp_class_get_name(klass);
    std::vector<std::string> extends;
    auto parent = il2cpp_class_get_parent(klass);
    if (!is_valuetype && !is_enum && parent) {
        auto parent_type = il2cpp_class_get_type(parent);
        if (parent_type->type != IL2CPP_TYPE_OBJECT) {
            extends.emplace_back(il2cpp_class_get_name(parent));
        }
    }
    void *iter = nullptr;
    while (auto itf = il2cpp_class_get_interfaces(klass, &iter)) {
        extends.emplace_back(il2cpp_class_get_name(itf));
    }
    if (!extends.empty()) {
        outPut << " : " << extends[0];
        for (int i = 1; i < extends.size(); ++i) {
            outPut << ", " << extends[i];
        }
    }
    outPut << "\n{";
    outPut << dump_field(klass);
    outPut << dump_property(klass);
    outPut << dump_method(klass);
    outPut << "}\n";
    return outPut.str();
}

uint64_t get_handle_base(void *handle) {
    if (!handle) return 0;
    xdl_info_t info; 
    if (xdl_info(handle, XDL_DI_DLINFO, &info) == 0) return (uint64_t)info.dli_fbase;
    return 0;
}

void il2cpp_api_init(void *handle) {
    LOGI("[CHEAT] il2cpp_handle: %p", handle);
    init_il2cpp_api(handle);
    il2cpp_base = get_handle_base(handle);
    if (il2cpp_base) {
        LOGI("[CHEAT] il2cpp_base: %" PRIx64"", il2cpp_base);
        return;
    }
    LOGE("[CHEAT] failed to initialize il2cpp api.");
}

void il2cpp_dump(const char *outDir) {
    LOGI("[CHEAT] dumping...");
    size_t size;
    auto domain = il2cpp_domain_get();
    auto assemblies = il2cpp_domain_get_assemblies(domain, &size);
    std::stringstream imageOutput;
    for (int i = 0; i < size; ++i) {
        auto image = il2cpp_assembly_get_image(assemblies[i]);
        imageOutput << "// Image " << i << ": " << il2cpp_image_get_name(image) << "\n";
    }
    std::vector<std::string> outPuts;
    if (il2cpp_image_get_class) {
        LOGI("[CHEAT] version greater than 2018.3");
        for (int i = 0; i < size; ++i) {
            auto image = il2cpp_assembly_get_image(assemblies[i]);
            std::stringstream imageStr;
            imageStr << "\n// Dll : " << il2cpp_image_get_name(image);
            auto classCount = il2cpp_image_get_class_count(image);
            for (int j = 0; j < classCount; ++j) {
                auto klass = il2cpp_image_get_class(image, j);
                auto type = il2cpp_class_get_type(const_cast<Il2CppClass *>(klass));
                // LOGD("type name : %s", il2cpp_type_get_name(type));
                auto outPut = imageStr.str() + dump_type(type);
                outPuts.push_back(outPut);
            }
        }
    } else {
        LOGI("[CHEAT] version less than 2018.3");
        auto corlib = il2cpp_get_corlib();
        auto assemblyClass = il2cpp_class_from_name(corlib, "System.Reflection", "Assembly");
        auto assemblyLoad = il2cpp_class_get_method_from_name(assemblyClass, "Load", 1);
        auto assemblyGetTypes = il2cpp_class_get_method_from_name(assemblyClass, "GetTypes", 0);
        if (assemblyLoad && assemblyLoad->methodPointer) {
            LOGI("[CHEAT] Assembly::Load: %p", assemblyLoad->methodPointer);
        } else {
            LOGI("[CHEAT] miss Assembly::Load");
            return;
        }
        if (assemblyGetTypes && assemblyGetTypes->methodPointer) {
            LOGI("[CHEAT] Assembly::GetTypes: %p", assemblyGetTypes->methodPointer);
        } else {
            LOGI("[CHEAT] miss Assembly::GetTypes");
            return;
        }
        typedef void *(*Assembly_Load_ftn)(void*, Il2CppString*, void*);
        typedef Il2CppArray *(*Assembly_GetTypes_ftn)(void*, void*);
        for (int i = 0; i < size; ++i) {
            auto image = il2cpp_assembly_get_image(assemblies[i]);
            std::stringstream imageStr;
            auto image_name = il2cpp_image_get_name(image);
            imageStr << "\n// Dll : " << image_name;
            // LOGD("image name : %s", image->name);
            auto imageName = std::string(image_name);
            auto pos = imageName.rfind('.');
            auto imageNameNoExt = imageName.substr(0, pos);
            auto assemblyFileName = il2cpp_string_new(imageNameNoExt.data());
            auto reflectionAssembly = ((Assembly_Load_ftn) assemblyLoad->methodPointer)(nullptr, assemblyFileName, nullptr);
            auto reflectionTypes = ((Assembly_GetTypes_ftn) assemblyGetTypes->methodPointer)(reflectionAssembly, nullptr);
            auto items = reflectionTypes->vector;
            for (int j = 0; j < reflectionTypes->max_length; ++j) {
                auto klass = il2cpp_class_from_system_type((Il2CppReflectionType *) items[j]);
                auto type = il2cpp_class_get_type(klass);
                // LOGD("type name : %s", il2cpp_type_get_name(type));
                auto outPut = imageStr.str() + dump_type(type);
                outPuts.push_back(outPut);
            }
        }
    }
    LOGI("[CHEAT] write dump file");
    auto outPath = std::string(outDir).append("/files/dump.cs");
    std::ofstream outStream(outPath);
    outStream << imageOutput.str();
    auto count = outPuts.size();
    for (int i = 0; i < count; ++i) {
        outStream << outPuts[i];
    }
    outStream.close();
    LOGI("[CHEAT] dump done!");
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

const MethodInfo *get_method(Il2CppClass *klass, const char *name, int paramIndex = -1, const char *paramName = nullptr, const char *paramType = nullptr) {
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

enum SaveType : uint8_t { 
    Field = 0, 
    Method = 1 
};

void AppendPreloadMap(const std::string &path, const std::string &key, uint64_t value, uint8_t type) {
    std::ofstream out(path, std::ios::binary | std::ios::app);
    if (out.is_open()) {
        uint64_t len = key.size();
        out.write((char*)&len, sizeof(len));
        out.write(key.c_str(), len);
        out.write((char*)&value, sizeof(uint64_t));
        out.write((char*)&type, sizeof(uint8_t));
        out.close();
    }
}

void LoadPreloadMap(const std::string &path) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) return;
    PreloadMap.clear();
    while (in.peek() != EOF) {
        uint64_t len = 0; 
        in.read((char*)&len, sizeof(len));
        if (!in) break;
        std::string key(len, '\0');
        in.read(&key[0], len);
        uint64_t value = 0;
        in.read((char*)&value, sizeof(uint64_t));
        uint8_t type = 0;
        in.read((char*)&type, sizeof(uint8_t));
        if (type == Method) {
            PreloadMap[key] = value + il2cpp_base;
        } else {
            PreloadMap[key] = value; 
        }
    }
    in.close();
}

void il2cpp_save(const std::string &path, Il2CppDomain *domain, const char *imageName, const char *namespaceName, const char *klassName, const char *name, SaveType type, int paramIndex = -1, const char *paramName = nullptr, const char *paramType = nullptr) {
    auto image = get_image(domain, imageName);
    auto klass = il2cpp_class_from_name(image, namespaceName, klassName);

    std::string key = std::string(namespaceName) + "." + klassName + "::" + name;
    uint64_t result = 0;

    switch (type) {
        case Field:
            result = (uint64_t)il2cpp_field_get_offset(il2cpp_class_get_field_from_name(klass, name));
            break;
        case Method:
            result = (uint64_t)get_method(klass, name, paramIndex, paramName, paramType)->methodPointer - il2cpp_base;
            break;
    }

    AppendPreloadMap(path, key, result, (uint8_t)type);
    LOGI("[SAVED] %s", key.c_str());
}

void il2cpp_prepare(const char *outDir) {
    LOGI("[CHEAT] preparing...");
    auto path = std::string(outDir).append("/files/PreloadMap.bin");
    auto domain = il2cpp_domain_get();

    // il2cpp_save(path, domain, "Project_d.dll", "Assets.Scripts.GameSystem", "CRoleInfoManager", "IsPrivaceOn", Method, 1, nullptr, "COM_USER_PRIVACY_MASK");

    // create empty file to prevent error
    std::ofstream out(path); 
    out.close();
}

/*
bool (*orig_IsPrivaceOn)(void *_this, void *mask) = nullptr;
bool proxy_IsPrivaceOn(void *_this, void *mask) {
    return false;
}
*/

// reserved.menu_value->IsESP;

void il2cpp_hook(const char *outDir) {
    auto path = std::string(outDir).append("/files/PreloadMap.bin");
    LoadPreloadMap(path);

    // DobbyHook((void*)PreloadMap["Assets.Scripts.GameSystem.CRoleInfoManager::IsPrivaceOn"], (void*)proxy_IsPrivaceOn, (void**)&orig_IsPrivaceOn);

    remove(path.c_str());
    LOGI("[CHEAT] loaded!");
}