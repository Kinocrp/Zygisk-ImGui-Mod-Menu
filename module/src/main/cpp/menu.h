//
// Created by Mono on 2024/12/30.
//

#ifndef ZYGISK_IMGUI_MOD_MENU_MENU_H
#define ZYGISK_IMGUI_MOD_MENU_MENU_H

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <algorithm>
#include <string>
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_android.h"
#include <time.h>
#include "font.h"
#include "il2cpp_hook.h"
#include "esp-draw.h"

float scale = 1.0f;
float baseFontSize = 14.0f;
float iconFontSize = baseFontSize * 2.0f / 3.0f;

ImFont* regular;
ImFont* medium;
ImFont* bold;
ImFont* title;
ImFont* icons;

void SetUpColors(ImGuiStyle& style, ImVec4* colors) {
    ImColor hovered = { 31, 110, 171 };
    ImColor Transparented = { 0, 0, 0, 255 };
    colors[ImGuiCol_WindowBg] = ImColor(20, 23, 25);
    colors[ImGuiCol_ChildBg] = ImColor(24, 28, 30);
    colors[ImGuiCol_Text] = ImColor(255, 255, 255);

    // Header
    colors[ImGuiCol_Header] = ImColor(30, 138, 200);
    colors[ImGuiCol_HeaderHovered] = hovered;
    colors[ImGuiCol_HeaderActive] = ImColor(30, 116, 215);

    // Buttons
    colors[ImGuiCol_Button] = ImColor(25, 145, 215);
    colors[ImGuiCol_ButtonHovered] = hovered;
    colors[ImGuiCol_ButtonActive] = ImColor(100, 161, 222);

    // Checkboxes
    colors[ImGuiCol_CheckMark] = ImColor(0, 0, 0);
    colors[ImGuiCol_FrameBg] = ImColor(25, 158, 215, 200);
    colors[ImGuiCol_FrameBgActive] = ImColor(25, 164, 215);
    colors[ImGuiCol_FrameBgHovered] = ImColor(20, 212, 250);

    colors[ImGuiCol_Border] = Transparented;

    style.WindowRounding = 20.0f;
    style.FrameRounding = 5.0f;
    style.ScrollbarRounding = 5.0f;
    style.GrabRounding = 2.3f;
    style.TabRounding = 2.3f;
    style.ChildRounding = 5.0f;
}

void SetupImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    io.DisplaySize = ImVec2((float) g_width, (float) g_height);

    float min_dim = std::min(g_width, g_height);
    float scale = min_dim / (300.0f * 1.5f);

    ImGui_ImplOpenGL3_Init("#version 100");
    io.Fonts->AddFontDefault();
    SetUpColors(style, colors);
    ImFontConfig font_config;
    font_config.PixelSnapH = false;
    font_config.OversampleH = 5;
    font_config.OversampleV = 5;
    font_config.RasterizerMultiply = 1.2f;
    static const ImWchar ranges[] =
            {
                    0x0020, 0x00FF, // Basic Latin + Latin Supplement
                    0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
                    0x2DE0, 0x2DFF, // Cyrillic Extended-A
                    0xA640, 0xA69F, // Cyrillic Extended-B
                    0xE000, 0xE226, // Icons
                    0
            };
    ImFontConfig icontfont_config;
    icontfont_config.MergeMode = true;
    icontfont_config.GlyphMinAdvanceX = iconFontSize;
    icontfont_config.PixelSnapH = true;

    regular = io.Fonts->AddFontFromMemoryTTF(RobotoRegular, sizeof(RobotoRegular), 15.0f, &font_config, ranges);
    medium  = io.Fonts->AddFontFromMemoryTTF(RobotoMedium, sizeof(RobotoMedium), 15.0f, &font_config, ranges);
    bold    = io.Fonts->AddFontFromMemoryTTF(RobotoBold, sizeof(RobotoBold), 15.0f, &font_config, ranges);
    title   = io.Fonts->AddFontFromMemoryTTF(RobotoBold, sizeof(RobotoBold), 25.0f, &font_config, ranges);
    io.FontGlobalScale = scale;
    io.FontDefault = regular;

    style.WindowPadding = ImVec2(8 * scale, 8 * scale);
    style.FramePadding  = ImVec2(6 * scale, 6 * scale);
    style.ItemSpacing   = ImVec2(8 * scale, 8 * scale);
    style.ItemInnerSpacing = ImVec2(4 * scale, 4 * scale);
    style.TouchExtraPadding = ImVec2(4 * scale, 4 * scale);

    style.WindowMinSize = { 300 * scale, 300 * scale };
    style.ScrollbarSize = 16 * scale;
    style.GrabMinSize   = 16 * scale;
}

void DrawESP(ESPManager& manager) {
    if (!IsESP) return;
    CalcESP(manager);
    for (auto& obj : manager.get_ESPObjects()) {
        LOGI("%d [%0.2f, %0.2f]", obj.objID, obj.x, obj.y);
        ESP::DrawText(ImVec2(obj.x, obj.y - 100), ImVec4(0, 255, 0, 255), std::to_string(obj.objID).c_str(), regular, 50.0f);
        ESP::DrawRect(ImVec4(obj.x - 50, obj.y - 50, 100, 100), ImVec4(255, 0, 0, 255), 5);
    }
}

void DrawMenu() {
    // Click On Zygisk Watermark To Open Menu
    ImGuiIO& io = ImGui::GetIO();
    static bool showMenu = false;

    if (g_last_touch.released) {
        ImVec2 mousePos = ImVec2(g_last_touch.x, g_last_touch.y);
        ImVec2 rectMin = ImVec2(g_width / 2 - 125, g_height - 50);
        ImVec2 rectMax = ImVec2(g_width / 2 + 125, g_height + 50);

        if (mousePos.x >= rectMin.x && mousePos.x <= rectMax.x && mousePos.y >= rectMin.y && mousePos.y <= rectMax.y) showMenu = !showMenu;
        g_last_touch.released = false;
    }

    if (showMenu) {
        // Mod Menu
        ImGui::SetNextWindowSizeConstraints(ImVec2(300 * scale, 300 * scale), ImVec2(300 * scale, 300 * scale));
        ImGui::Begin("Modded By Kinocrp", &showMenu, ImGuiWindowFlags_NoResize);

        ImGui::Text("%s", g_hook_status.c_str());
        ImGui::Checkbox("ESP", &IsESP);
        ImGui::End(); // Render end
    }

    // Zygisk Watermark
    ESP::DrawLine(ImVec2(g_width / 2 - 100, g_height), ImVec2(g_width / 2 + 100, g_height), ImVec4(0, 0, 0, 255), 50);
    ESP::DrawCircle(g_width / 2 - 100, g_height, 25, true, ImVec4(0, 0, 0, 255));
    ESP::DrawCircle(g_width / 2 + 100, g_height, 25, true, ImVec4(0, 0, 0, 255));
    ESP::DrawText(ImVec2(g_width / 2, g_height - 12), ImVec4(255, 255, 255, 255), "Powered By Zygisk", regular, 20.0f);
}

EGLBoolean (*old_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    eglQuerySurface(dpy, surface, EGL_WIDTH, &g_width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &g_height);
    if (!g_imgui_initialized)
    {
        SetupImGui();
        eglSwapInterval(dpy, 1); // vsync
        g_imgui_initialized = true;
    }
    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplOpenGL3_NewFrame();

    io.MousePos = ImVec2(g_last_touch.x, g_last_touch.y);
    io.MouseDown[0] = g_last_touch.down;
    ImGui::NewFrame();

    DrawMenu();
    DrawESP(std::ref(g_ESPManager));

    ImGui::EndFrame();
    ImGui::Render();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    return old_eglSwapBuffers(dpy, surface);
}

#endif //ZYGISK_IMGUI_MOD_MENU_MENU_H
