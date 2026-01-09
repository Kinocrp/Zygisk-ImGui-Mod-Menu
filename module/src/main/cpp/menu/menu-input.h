#pragma once

#include "imgui.h"
#include "il2cpp-resolver.h"
#include "il2cpp-structs.h"

bool (*GetMouseButton)(int32_t, const MethodInfo*) = nullptr;
Vector3 (*get_mousePosition)(const MethodInfo*) = nullptr;

void menu_input_init(Il2CppDomain *domain) {
    auto UnityEngine = get_image(domain, "UnityEngine.InputLegacyModule.dll");
    if (!UnityEngine) UnityEngine = get_image(domain, "UnityEngine.dll");
    auto Input = il2cpp_class_from_name(UnityEngine, "UnityEngine", "Input");

    GetMouseButton = (bool(*)(int32_t, const MethodInfo*))(void*)get_method(Input, "GetMouseButton")->methodPointer;
    get_mousePosition = (Vector3(*)(const MethodInfo*))(void*)get_method(Input, "get_mousePosition")->methodPointer;
}

void menu_input(ImGuiIO &io, int height) {
    Vector3 position = get_mousePosition(nullptr);
    io.MouseDown[0] = GetMouseButton(0, nullptr);
    io.MousePos = ImVec2(position.x, (float)height - position.y);
}