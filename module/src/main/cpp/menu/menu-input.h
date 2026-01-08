#pragma once

#include "imgui.h"
#include "il2cpp-resolver.h"
#include "il2cpp-structs.h"

struct TouchBox {
    uint8_t data[128];
};

int32_t (*get_touchCount)(const MethodInfo*) = nullptr;
TouchBox (*GetTouch)(int32_t, const MethodInfo*) = nullptr;
bool (*GetMouseButton)(int32_t, const MethodInfo*) = nullptr;
Vector3 (*get_mousePosition)(const MethodInfo*) = nullptr;
Vector2 (*get_position)(Il2CppObject*, const MethodInfo*) = nullptr;
int32_t (*get_phase)(Il2CppObject*, const MethodInfo*) = nullptr;

void menu_input_init(Il2CppDomain *domain) {
    auto UnityEngine = get_image(domain, "UnityEngine.InputLegacyModule.dll");
    if (!UnityEngine) UnityEngine = get_image(domain, "UnityEngine.dll");
    auto Input = il2cpp_class_from_name(UnityEngine, "UnityEngine", "Input");
    auto Touch = il2cpp_class_from_name(UnityEngine, "UnityEngine", "Touch");

    get_touchCount = (int32_t(*)(const MethodInfo*))(void*)get_method(Input, "get_touchCount")->methodPointer;
    GetTouch = (TouchBox(*)(int32_t, const MethodInfo*))(void*)get_method(Input, "GetTouch")->methodPointer;
    GetMouseButton = (bool(*)(int32_t, const MethodInfo*))(void*)get_method(Input, "GetMouseButton")->methodPointer;
    get_mousePosition = (Vector3(*)(const MethodInfo*))(void*)get_method(Input, "get_mousePosition")->methodPointer;
    get_position = (Vector2(*)(Il2CppObject*, const MethodInfo*))(void*)get_method(Touch, "get_position")->methodPointer;
    get_phase = (int32_t(*)(Il2CppObject*, const MethodInfo*))(void*)get_method(Touch, "get_phase")->methodPointer;
}

void menu_input(ImGuiIO &io, int height) {
    if (get_touchCount && get_touchCount(nullptr) > 0) {
        auto touch = GetTouch(0, nullptr);
        auto position = get_position((Il2CppObject*)&touch, nullptr);
        auto phase = get_phase((Il2CppObject*)&touch, nullptr);
        io.MousePos = ImVec2(position.x, (float)height - position.y);
        switch (phase) {
            case 0:
            case 1:
            case 2:
                io.MouseDown[0] = true;
                break;
            case 3:
            case 4:
                io.MouseDown[0] = false;
                break;
            default:
                break;
        }
        return;
    }

    if (GetMouseButton && get_mousePosition) {
        Vector3 position = get_mousePosition(nullptr);
        io.MouseDown[0] = GetMouseButton(0, nullptr);
        io.MousePos = ImVec2(position.x, (float)height - position.y);
    }
}