#include "il2cpp.h"
#include <vector>
#include <sstream>
#include <fstream>
#include <locale>
#include <codecvt>
#include "xdl.h"
#include "log.h"

static std::string get_method_modifier(uint32_t flags) {
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

static bool _il2cpp_type_is_byref(const Il2CppType *type) {
    auto byref = type->byref;
    if (IL2CPP::API::il2cpp_type_is_byref) {
        byref = IL2CPP::API::il2cpp_type_is_byref(type);
    }
    return byref;
}

static std::string dump_method(Il2CppClass *klass) {
    std::stringstream outPut;
    outPut << "\n\t// Methods\n";
    void *iter = nullptr;
    while (auto method = IL2CPP::API::il2cpp_class_get_methods(klass, &iter)) {
        if (method->methodPointer) {
            outPut << "\t// RVA: 0x";
            outPut << std::hex << (uint64_t)method->methodPointer - IL2CPP::il2cpp_base;
            outPut << " VA: 0x";
            outPut << std::hex << (uint64_t)method->methodPointer;
        } else {
            outPut << "\t// RVA: 0x VA: 0x0";
        }
        outPut << "\n\t";
        uint32_t iflags = 0;
        auto flags = IL2CPP::API::il2cpp_method_get_flags(method, &iflags);
        outPut << get_method_modifier(flags);
        auto return_type = IL2CPP::API::il2cpp_method_get_return_type(method);
        if (_il2cpp_type_is_byref(return_type)) {
            outPut << "ref ";
        }
        auto return_class = IL2CPP::API::il2cpp_class_from_type(return_type);
        outPut << IL2CPP::API::il2cpp_class_get_name(return_class) << " " << IL2CPP::API::il2cpp_method_get_name(method) << "(";
        auto param_count = IL2CPP::API::il2cpp_method_get_param_count(method);
        for (int i = 0; i < param_count; ++i) {
            auto param = IL2CPP::API::il2cpp_method_get_param(method, i);
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
            auto parameter_class = IL2CPP::API::il2cpp_class_from_type(param);
            outPut << IL2CPP::API::il2cpp_class_get_name(parameter_class) << " " << IL2CPP::API::il2cpp_method_get_param_name(method, i);
            outPut << ", ";
        }
        if (param_count > 0) {
            outPut.seekp(-2, outPut.cur);
        }
        outPut << ") { }\n";
    }
    return outPut.str();
}

static std::string dump_property(Il2CppClass *klass) {
    std::stringstream outPut;
    outPut << "\n\t// Properties\n";
    void *iter = nullptr;
    while (auto prop_const = IL2CPP::API::il2cpp_class_get_properties(klass, &iter)) {
        auto prop = const_cast<PropertyInfo*>(prop_const);
        auto get = IL2CPP::API::il2cpp_property_get_get_method(prop);
        auto set = IL2CPP::API::il2cpp_property_get_set_method(prop);
        auto prop_name = IL2CPP::API::il2cpp_property_get_name(prop);
        outPut << "\t";
        Il2CppClass *prop_class = nullptr;
        uint32_t iflags = 0;
        if (get) {
            outPut << get_method_modifier(IL2CPP::API::il2cpp_method_get_flags(get, &iflags));
            prop_class = IL2CPP::API::il2cpp_class_from_type(IL2CPP::API::il2cpp_method_get_return_type(get));
        } else if (set) {
            outPut << get_method_modifier(IL2CPP::API::il2cpp_method_get_flags(set, &iflags));
            auto param = IL2CPP::API::il2cpp_method_get_param(set, 0);
            prop_class = IL2CPP::API::il2cpp_class_from_type(param);
        }
        if (prop_class) {
            outPut << IL2CPP::API::il2cpp_class_get_name(prop_class) << " " << prop_name << " { ";
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

static std::string dump_field(Il2CppClass *klass) {
    std::stringstream outPut;
    outPut << "\n\t// Fields\n";
    auto is_enum = IL2CPP::API::il2cpp_class_is_enum(klass);
    void *iter = nullptr;
    while (auto field = IL2CPP::API::il2cpp_class_get_fields(klass, &iter)) {
        outPut << "\t";
        auto attrs = IL2CPP::API::il2cpp_field_get_flags(field);
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
        auto field_type = IL2CPP::API::il2cpp_field_get_type(field);
        auto field_class = IL2CPP::API::il2cpp_class_from_type(field_type);
        outPut << IL2CPP::API::il2cpp_class_get_name(field_class) << " " << IL2CPP::API::il2cpp_field_get_name(field);
        if (attrs & FIELD_ATTRIBUTE_LITERAL && is_enum) {
            uint64_t val = 0;
            IL2CPP::API::il2cpp_field_static_get_value(field, &val);
            outPut << " = " << std::dec << val;
        }
        outPut << "; // 0x" << std::hex << IL2CPP::API::il2cpp_field_get_offset(field) << "\n";
    }
    return outPut.str();
}

static std::string dump_type(const Il2CppType *type) {
    std::stringstream outPut;
    auto *klass = IL2CPP::API::il2cpp_class_from_type(type);
    outPut << "\n// Namespace: " << IL2CPP::API::il2cpp_class_get_namespace(klass) << "\n";
    auto flags = IL2CPP::API::il2cpp_class_get_flags(klass);
    if (flags & TYPE_ATTRIBUTE_SERIALIZABLE) {
        outPut << "[Serializable]\n";
    }
    auto is_valuetype = IL2CPP::API::il2cpp_class_is_valuetype(klass);
    auto is_enum = IL2CPP::API::il2cpp_class_is_enum(klass);
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
    outPut << IL2CPP::API::il2cpp_class_get_name(klass);
    std::vector<std::string> extends;
    auto parent = IL2CPP::API::il2cpp_class_get_parent(klass);
    if (!is_valuetype && !is_enum && parent) {
        auto parent_type = IL2CPP::API::il2cpp_class_get_type(parent);
        if (parent_type->type != IL2CPP_TYPE_OBJECT) {
            extends.emplace_back(IL2CPP::API::il2cpp_class_get_name(parent));
        }
    }
    void *iter = nullptr;
    while (auto itf = IL2CPP::API::il2cpp_class_get_interfaces(klass, &iter)) {
        extends.emplace_back(IL2CPP::API::il2cpp_class_get_name(itf));
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



namespace IL2CPP {

    uint64_t il2cpp_base = 0;
    Il2CppDomain *domain = nullptr;

    namespace API {
        #define DO_API(r, n, p) r (*n) p = nullptr
        #include "il2cpp-api-functions.h"
        #undef DO_API
    }

    bool Init() {
        auto handle = xdl_open("libil2cpp.so", 0);
        if (!handle) return false;

        #define DO_API(r, n, p) {                           \
                API::n = (r (*) p)xdl_sym(handle, #n, nullptr);      \
                if(!API::n) {                                        \
                    LOGW("[-] API not found %s", #n);               \
                }                                               \
            }
        #include "il2cpp-api-functions.h"
        #undef DO_API

        xdl_info_t info;
        xdl_info(handle, XDL_DI_DLINFO, &info);
        il2cpp_base = (uint64_t)info.dli_fbase;

        LOGI("[+] IL2CPP Base: 0x%" PRIx64"", il2cpp_base);
        return true;
    }

    Il2CppThread *Attach() {
        if (!il2cpp_base) return nullptr;
        if (!API::il2cpp_is_vm_thread(nullptr)) return nullptr;
        if (!domain) {
            domain = API::il2cpp_domain_get();
            LOGI("[+] IL2CPP Domain: %p", domain);
        }

        auto thread = API::il2cpp_thread_attach(domain);
        LOGI("[+] Thread Attach: %p", thread);
        return thread;
    }

    void Dump(std::string outDir) {
        LOGI("[+] Dumping...");
        size_t size;
        auto assemblies = API::il2cpp_domain_get_assemblies(domain, &size);
        std::stringstream imageOutput;
        for (int i = 0; i < size; ++i) {
            auto image = API::il2cpp_assembly_get_image(assemblies[i]);
            imageOutput << "// Image " << i << ": " << API::il2cpp_image_get_name(image) << "\n";
        }
        std::vector<std::string> outPuts;
        if (API::il2cpp_image_get_class) {
            LOGI("[*] Version is greater than 2018.3");
            for (int i = 0; i < size; ++i) {
                auto image = API::il2cpp_assembly_get_image(assemblies[i]);
                std::stringstream imageStr;
                imageStr << "\n// Dll : " << API::il2cpp_image_get_name(image);
                auto classCount = API::il2cpp_image_get_class_count(image);
                for (int j = 0; j < classCount; ++j) {
                    auto klass = API::il2cpp_image_get_class(image, j);
                    auto type = API::il2cpp_class_get_type(const_cast<Il2CppClass*>(klass));
                    auto outPut = imageStr.str() + dump_type(type);
                    outPuts.push_back(outPut);
                }
            }
        } else {
            LOGI("[*] Version is less than 2018.3");
            auto corlib = API::il2cpp_get_corlib();
            auto assemblyClass = API::il2cpp_class_from_name(corlib, "System.Reflection", "Assembly");
            auto assemblyLoad = API::il2cpp_class_get_method_from_name(assemblyClass, "Load", 1);
            auto assemblyGetTypes = API::il2cpp_class_get_method_from_name(assemblyClass, "GetTypes", 0);
            if (assemblyLoad && assemblyLoad->methodPointer) {
                LOGI("[+] Assembly::Load: %p", assemblyLoad->methodPointer);
            } else {
                LOGE("[!] Miss Assembly::Load");
                return;
            }
            if (assemblyGetTypes && assemblyGetTypes->methodPointer) {
                LOGI("[+] Assembly::GetTypes: %p", assemblyGetTypes->methodPointer);
            } else {
                LOGE("[!] Miss Assembly::GetTypes");
                return;
            }
            typedef void *(*Assembly_Load_ftn)(void*, Il2CppString*, void*);
            typedef Il2CppArray *(*Assembly_GetTypes_ftn)(void*, void*);
            for (int i = 0; i < size; ++i) {
                auto image = API::il2cpp_assembly_get_image(assemblies[i]);
                std::stringstream imageStr;
                auto image_name = API::il2cpp_image_get_name(image);
                imageStr << "\n// Dll : " << image_name;
                auto imageName = std::string(image_name);
                auto pos = imageName.rfind('.');
                auto imageNameNoExt = imageName.substr(0, pos);
                auto assemblyFileName = API::il2cpp_string_new(imageNameNoExt.data());
                auto reflectionAssembly = ((Assembly_Load_ftn)assemblyLoad->methodPointer)(nullptr, assemblyFileName, nullptr);
                auto reflectionTypes = ((Assembly_GetTypes_ftn)assemblyGetTypes->methodPointer)(reflectionAssembly, nullptr);
                auto items = reflectionTypes->vector;
                for (int j = 0; j < reflectionTypes->max_length; ++j) {
                    auto klass = API::il2cpp_class_from_system_type((Il2CppReflectionType*)items[j]);
                    auto type = API::il2cpp_class_get_type(klass);
                    auto outPut = imageStr.str() + dump_type(type);
                    outPuts.push_back(outPut);
                }
            }
        }
        LOGI("[*] Saving...");
        auto outPath = outDir.append("/dump.cs");
        std::ofstream outStream(outPath);
        outStream << imageOutput.str();
        auto count = outPuts.size();
        for (int i = 0; i < count; ++i) {
            outStream << outPuts[i];
        }
        outStream.close();
        LOGI("[+] Saved to %s", outPath.c_str());
    }

    Il2CppImage *GetImage(const char *dllName) {
        size_t assemblyCount = 0;
        const Il2CppAssembly **assemblies = API::il2cpp_domain_get_assemblies(domain, &assemblyCount);
        for (size_t i = 0; i < assemblyCount; ++i) {
            const Il2CppImage *cimage = API::il2cpp_assembly_get_image(assemblies[i]);
            const char *name = API::il2cpp_image_get_name(cimage);
            if (name && strcmp(name, dllName) == 0) return const_cast<Il2CppImage*>(cimage);
        }
        return nullptr;
    }

    const MethodInfo *GetMethod(Il2CppClass* klass, const char *name, int paramIndex, const char *paramName, const char *paramType) {
        void *iter = nullptr;
        const MethodInfo *method = nullptr;
        while ((method = API::il2cpp_class_get_methods(klass, &iter))) {
            const char *currentName = API::il2cpp_method_get_name(method);
            if (!currentName || strcmp(currentName, name) != 0) continue;
            if (paramIndex == -1) return method;
            int paramCount = API::il2cpp_method_get_param_count(method);
            if (paramIndex == paramCount) {
                if (!paramName && !paramType) return method;
            } else if (paramIndex < paramCount) {
                bool isMatch = true;
                if (paramName) {
                    const char *argName = API::il2cpp_method_get_param_name(method, paramIndex);
                    if (!argName || strcmp(argName, paramName) != 0) isMatch = false;
                }
                if (paramType && isMatch) {
                    const Il2CppType *argType = API::il2cpp_method_get_param(method, paramIndex);
                    Il2CppClass *argClass = API::il2cpp_class_from_type(argType);
                    const char *argTypeName = argClass ? API::il2cpp_class_get_name(argClass) : nullptr;
                    if (!argTypeName || strcmp(argTypeName, paramType) != 0) isMatch = false;
                }
                if (isMatch) return method;
            }
        }
        return nullptr;
    }

    std::string Il2CppToString(Il2CppString* str) {
        if (str == nullptr) return "";
        Il2CppChar *chars = API::il2cpp_string_chars(str);
        std::u16string u16(reinterpret_cast<const char16_t*>(chars));
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
        return converter.to_bytes(u16);
    }
}