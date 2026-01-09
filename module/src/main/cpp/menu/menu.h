#pragma once

#include <vector>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_android.h>
#include "imgui.h"
#include "imgui_impl_android.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_vulkan.h"
#include "il2cpp-resolver.h"

#include "menu-config.h"
#include "menu-input.h"
#include "menu-draw.h"

// #include "font.h"

static bool menu_inited = false;
static bool menu_show = false;
static int screenWidth = 0;
static int screenHeight = 0;
static float scaleFactor = 1.0f;
static float icon_size = 22.0f;

static ImVec2 menuPos = ImVec2(100, 100);

MenuConfig menu_config;

int32_t (*get_width)(const MethodInfo*) = nullptr;
int32_t (*get_height)(const MethodInfo*) = nullptr;

void menu_init(Il2CppDomain *domain) {
    auto UnityEngine = get_image(domain, "UnityEngine.CoreModule.dll");
    if (!UnityEngine) UnityEngine = get_image(domain, "UnityEngine.dll");
    auto Screen = il2cpp_class_from_name(UnityEngine, "UnityEngine", "Screen");

    get_width = (int32_t(*)(const MethodInfo*))(void*)get_method(Screen, "get_width")->methodPointer;
    get_height = (int32_t(*)(const MethodInfo*))(void*)get_method(Screen, "get_height")->methodPointer;

    ImGui::CreateContext();
    ImGuiStyle &style = ImGui::GetStyle();

    ImGui_ImplOpenGL3_Init("#version 300 es");

    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.16f, 0.16f, 0.21f, 1.0f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.74f, 0.58f, 0.98f, 1.0f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    style.Colors[ImGuiCol_Header] = ImColor(76, 76, 76, 150);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.19f, 0.2f, 0.25f, 1.0f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.16f, 0.16f, 0.21f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImColor(76, 76, 76, 150);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.2f, 0.25f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.16f, 0.16f, 0.21f, 1.0f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.74f, 0.58f, 0.98f, 1.0f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.1f, 0.1f, 0.13f, 0.92f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.44f, 0.37f, 0.61f, 0.54f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.74f, 0.58f, 0.98f, 0.54f);
    style.Colors[ImGuiCol_FrameBg] = ImColor(64, 64, 74, 150);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.2f, 0.25f, 1.0f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.16f, 0.16f, 0.21f, 1.0f);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.16f, 0.16f, 0.21f, 1.0f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.24f, 0.24f, 0.32f, 1.0f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.2f, 0.22f, 0.27f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.16f, 0.16f, 0.21f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.16f, 0.16f, 0.21f, 1.0f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.16f, 0.16f, 0.21f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.16f, 0.21f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.16f, 0.16f, 0.21f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.1f, 0.1f, 0.13f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.16f, 0.16f, 0.21f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.19f, 0.2f, 0.25f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.24f, 0.24f, 0.32f, 1.0f);
    style.Colors[ImGuiCol_Separator] = ImVec4(0.44f, 0.37f, 0.61f, 1.0f);
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.74f, 0.58f, 0.98f, 1.0f);
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.84f, 0.58f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.44f, 0.37f, 0.61f, 0.29f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.74f, 0.58f, 0.98f, 0.29f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.84f, 0.58f, 1.0f, 0.29f);

    /*
    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->Clear();

    ImFontConfig cfg_base;
    cfg_base.FontDataOwnedByAtlas = false;
    cfg_base.PixelSnapH = false;
    cfg_base.OversampleH = 5;
    cfg_base.OversampleV = 5;
    cfg_base.RasterizerMultiply = 1.0f;

    io.FontDefault = io.Fonts->AddFontFromMemoryCompressedTTF(
            (void*)font_data,
            font_size,
            15.0f,
            &cfg_base,
            io.Fonts->GetGlyphRangesChineseFull()
    );
    */
}

void menu_resize(ImGuiIO &io, ImGuiStyle &style) {
    io.DisplaySize = ImVec2((float)screenWidth, (float)screenHeight);

    float min_dim = std::min((float)screenWidth, (float)screenHeight);
    if (min_dim < 1.0f) min_dim = 1.0f;
    scaleFactor = min_dim / 400.0f;

    style.WindowPadding = ImVec2(8.0f * scaleFactor, 8.0f * scaleFactor);
    style.FramePadding = ImVec2(8.0f * scaleFactor, 6.0f * scaleFactor);
    style.ItemSpacing = ImVec2(8.0f * scaleFactor, 8.0f * scaleFactor);
    style.ItemInnerSpacing = ImVec2(4.0f * scaleFactor, 4.0f * scaleFactor);
    style.TouchExtraPadding = ImVec2(0.0f, 0.0f);
    style.IndentSpacing = 21.0f * scaleFactor;
    style.ColumnsMinSpacing = 6.0f * scaleFactor;
    style.ScrollbarSize = 18.0f * scaleFactor;
    style.GrabMinSize = 12.0f * scaleFactor;
    style.WindowBorderSize = 1.5f * scaleFactor;
    style.ChildBorderSize = 1.0f * scaleFactor;
    style.PopupBorderSize = 1.0f * scaleFactor;
    style.FrameBorderSize = 0.0f * scaleFactor;
    style.TabBorderSize = 0.0f * scaleFactor;
    style.WindowRounding = 7.0f * scaleFactor;
    style.ChildRounding = 7.0f * scaleFactor;
    style.FrameRounding = 4.0f * scaleFactor;
    style.PopupRounding = 4.0f * scaleFactor;
    style.ScrollbarRounding = 9.0f * scaleFactor;
    style.GrabRounding = 3.0f * scaleFactor;
    style.TabRounding = 4.0f * scaleFactor;
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);

    io.FontGlobalScale = scaleFactor;
}

void menu_icon(ImGuiIO &io, ImDrawList *drawList) {
    static bool isDragging = false;
    static bool wasDragging = false;
    static ImVec2 dragOffset = ImVec2(0, 0);

    float radius = icon_size * scaleFactor;
    float fontSize = icon_size * scaleFactor * 0.7f;

    float dx = io.MousePos.x - menuPos.x;
    float dy = io.MousePos.y - menuPos.y;
    float distance = sqrtf(dx * dx + dy * dy);
    bool isHovered = (distance <= radius);

    if (ImGui::IsMouseClicked(0) && isHovered) {
        isDragging = true;
        wasDragging = false;
        dragOffset = ImVec2(io.MousePos.x - menuPos.x, io.MousePos.y - menuPos.y);
    }

    if (isDragging) {
        if (ImGui::IsMouseDown(0)) {
            if (io.MouseDragMaxDistanceSqr[0] > 10.0f) {
                wasDragging = true;
            }
            menuPos.x = io.MousePos.x - dragOffset.x;
            menuPos.y = io.MousePos.y - dragOffset.y;
        } else {
            if (!wasDragging) {
                menu_show = !menu_show;
            }
            isDragging = false;
        }
    }

    menuPos.x = ImClamp(menuPos.x, radius, io.DisplaySize.x - radius);
    menuPos.y = ImClamp(menuPos.y, radius, io.DisplaySize.y - radius);

    ImColor circleColor = ImColor(0.16f, 0.16f, 0.21f, 1.0f);
    if (isDragging) circleColor = ImColor(0.24f, 0.24f, 0.32f, 1.0f);
    else if (isHovered) circleColor = ImColor(0.19f, 0.2f, 0.25f, 1.0f);

    ImDraw::DrawCircle(drawList, true, menuPos, radius, circleColor);
    ImDraw::DrawCircle(drawList, false, menuPos, radius, ImColor(0.74f, 0.58f, 0.98f, 1.0f), 1.5f * scaleFactor);
    ImDraw::DrawText(drawList, nullptr, fontSize, ImVec2(menuPos.x, menuPos.y), ImColor(1.0f, 1.0f, 1.0f, 1.0f), "KINO");
}

void menu_draw(ImGuiIO &io, ImGuiStyle &style) {
    ImDrawList *drawList = ImGui::GetForegroundDrawList();
    ImDraw::DrawRect(drawList, true, ImVec2(io.DisplaySize.x / 2.0f - 55.0f * scaleFactor, io.DisplaySize.y - 10.0f * scaleFactor), ImVec2(io.DisplaySize.x / 2.0f + 55.0f * scaleFactor, io.DisplaySize.y + 10.0f * scaleFactor), ImColor(0.0f, 0.0f, 0.0f, 1.0f), 20.0f * scaleFactor);
    ImDraw::DrawText(drawList, nullptr, 8.0f * scaleFactor, ImVec2(io.DisplaySize.x / 2.0f, io.DisplaySize.y - 5.0f * scaleFactor), ImColor(1.0f, 1.0f, 1.0f, 1.0f), "Powered By Zygisk");
    if (menu_show) {
        static bool isDragging = false;
        static bool dragLocked = false;
        static int count = 0;

        float min_dim = std::min((float)screenWidth, (float)screenHeight);
        auto menuSize = ImVec2(min_dim * 0.7f, min_dim * 0.7f);
        float radius = icon_size * scaleFactor;

        auto initPos = ImVec2(
                ImClamp(menuPos.x - radius, 0.0f, io.DisplaySize.x - menuSize.x),
                ImClamp(menuPos.y - radius, 0.0f, io.DisplaySize.y - menuSize.y)
        );

        ImGui::SetNextWindowPos(initPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(menuSize, ImGuiCond_Always);

        if (ImGui::Begin("Modded By Kinocrp", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove)) {
            if (ImGui::IsMouseDown(0)) {
                if (count < 2) {
                    if (ImGui::IsAnyItemHovered()) {
                        dragLocked = true;
                    } else {
                        dragLocked = false;
                    }
                } else if (!dragLocked && ImGui::IsMouseDragging(0) && ImGui::IsWindowHovered()) {
                    isDragging = true;
                }
                count++;
            } else {
                isDragging = false;
                dragLocked = false;
                count = 0;
            }

            if (isDragging) {
                menuPos.x += io.MouseDelta.x;
                menuPos.y += io.MouseDelta.y;
                menuPos.x = ImClamp(menuPos.x, 0.0f, io.DisplaySize.x - menuSize.x + radius);
                menuPos.y = ImClamp(menuPos.y, 0.0f, io.DisplaySize.y - menuSize.y + radius);
            }

            ImGui::Checkbox("ESP", &menu_config.IsESP);

            ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ImGui::GetFrameHeight() - style.WindowPadding.y);
            if (ImGui::Button("Close Menu", ImVec2(-FLT_MIN, 0))) menu_show = false;
        }
        ImGui::End();
    } else {
        menu_icon(io, drawList);
    }
}

// OpenGL
EGLBoolean (*o_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
EGLBoolean h_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {

    if (!menu_inited) {
        auto domain = il2cpp_domain_get();
        il2cpp_thread_attach(domain);
        menu_init(domain);
        menu_input_init(domain);
        menu_inited = true;
    }
    screenWidth = get_width(nullptr);
    screenHeight = get_height(nullptr);

    ImGuiIO &io = ImGui::GetIO();
    ImGuiStyle &style = ImGui::GetStyle();

    menu_resize(io, style);
    menu_input(io, screenHeight);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    menu_draw(io, style);

    ImGui::EndFrame();
    ImGui::Render();

    GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
    GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);

    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glScissor(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glViewport(last_viewport[0], last_viewport[1], last_viewport[2], last_viewport[3]);
    glScissor(last_scissor_box[0], last_scissor_box[1], last_scissor_box[2], last_scissor_box[3]);

    return o_eglSwapBuffers(dpy, surface);
}