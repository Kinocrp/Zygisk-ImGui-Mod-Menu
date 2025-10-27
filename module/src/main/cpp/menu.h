//
// Created by Mono on 2024/12/30.
//

#ifndef ZYGISK_MOD_MENU_MENU_H
#define ZYGISK_MOD_MENU_MENU_H

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <algorithm>
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_android.h"
#include <time.h>
#include "font.h"
#include "il2cpp_hook.h"
#include "globals.h"
#include "esp.h"

int g_width = 0;
int g_height = 0;
bool g_imgui_initialized = false;

float scale = 1;
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

    //Header
    colors[ImGuiCol_Header] = ImColor(30, 138, 200);
    colors[ImGuiCol_HeaderHovered] = hovered;
    colors[ImGuiCol_HeaderActive] = ImColor(30, 116, 215);

    //buttons
    colors[ImGuiCol_Button] = ImColor(25, 145, 215);
    colors[ImGuiCol_ButtonHovered] = hovered;
    colors[ImGuiCol_ButtonActive] = ImColor(100, 161, 222);

    //checkboxes
    colors[ImGuiCol_CheckMark] = ImColor(0, 0, 0);
    colors[ImGuiCol_FrameBg] = ImColor(25, 158, 215, 200);
    colors[ImGuiCol_FrameBgActive] = ImColor(25, 164, 215);
    colors[ImGuiCol_FrameBgHovered] = ImColor(20, 212, 250);

    colors[ImGuiCol_Border] = Transparented;

    style.WindowRounding = 10.0f;
    style.FrameRounding = 5.0f;
    style.ScrollbarRounding = 5.0f;
    style.GrabRounding = 2.3f;
    style.TabRounding = 2.3f;

    style.WindowMinSize = { 300 * scale,300 * scale };
    style.ChildRounding = 5.0f;
}

void SetupImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    io.DisplaySize = ImVec2((float) g_width, (float) g_height);

    const float base_menu_size = 300.0f;
    float min_dim = std::min(g_width, g_height);
    float scale = min_dim / (base_menu_size * 1.5f);

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
                    0xE000, 0xE226, // icons
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

    style.WindowMinSize = { 300 * scale,300 * scale };
    style.ScrollbarSize = 16 * scale;
    style.GrabMinSize   = 16 * scale;
}

void DrawESP() {
    if (!IsESP) return;
    // ESP

}

void DrawMenu() {
    ESP::DrawLine(ImVec2(g_width/2 - 100, g_height), ImVec2(g_width/2 + 100, g_height), ImVec4(0, 0, 0, 255), 50);
    ESP::DrawCircle(g_width/2 - 100, g_height, 25, true, ImVec4(0, 0, 0, 255));
    ESP::DrawCircle(g_width/2 + 100, g_height, 25, true, ImVec4(0, 0, 0, 255));
    ESP::DrawText(ImVec2(g_width/2, g_height - 12), ImVec4(255, 255, 255, 255), "Powered By Zygisk", regular, 20.0f);

    // Mod Menu
    ImGui::Begin("Modded By Kinocrp", nullptr, ImGuiWindowFlags_NoResize);

    ImGui::Text("Basic Features");
    ImGui::Checkbox("ESP", &IsESP);
    ImGui::End(); // Render end
}

EGLBoolean (*old_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    eglQuerySurface(dpy, surface, EGL_WIDTH, &g_width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &g_height);
    if (!g_imgui_initialized)
    {
        SetupImGui();
        g_imgui_initialized = true;
    }
    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplOpenGL3_NewFrame();

    io.MousePos = ImVec2(g_last_touch.x, g_last_touch.y);
    io.MouseDown[0] = g_last_touch.down;

    ImGui::NewFrame();

    DrawESP();
    DrawMenu();

    ImGui::EndFrame();
    ImGui::Render();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    return old_eglSwapBuffers(dpy, surface);
}

#endif //ZYGISK_MOD_MENU_MENU_H
