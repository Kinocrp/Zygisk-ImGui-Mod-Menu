#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <algorithm>
#include <string>
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_android.h"
#include <time.h>
#include "draw.h"
#include "font.h"
#include "log.h"

#include "game-utils.h"
#include "menu-value.h"
#include "menu-input.h"

float scale = 1.0f;
float baseFontSize = 14.0f;
float iconFontSize = baseFontSize * 2.0f / 3.0f;

ImFont* regular;
ImFont* medium;
ImFont* bold;
ImFont* title;
ImFont* icons;

void SetUpColors(ImGuiStyle &style, ImVec4 *colors) {
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
    io.FontDefault = regular;
}

void DrawMenu(ImVec2 mousePos) {

    static bool showMenu = true;

    if (g_last_touch.released) {
        ImVec2 rectMin = ImVec2(g_menu_value.width / 2 - 125, g_menu_value.height - 50);
        ImVec2 rectMax = ImVec2(g_menu_value.width / 2 + 125, g_menu_value.height + 50);
        if (mousePos.x >= rectMin.x && mousePos.x <= rectMax.x && mousePos.y >= rectMin.y && mousePos.y <= rectMax.y) showMenu = !showMenu;
        g_last_touch.released = false;
    }

    if (showMenu) {
        ImGui::SetNextWindowSizeConstraints(ImVec2(300 * scale, 300 * scale), ImVec2(300 * scale, 300 * scale));
        ImGui::Begin("Modded By Kinocrp", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        ImGui::Checkbox("ESP", &g_menu_value.IsESP);

        ImGui::End();
    }

    ImDraw::DrawLine(ImVec2(g_menu_value.width / 2 - 100, g_menu_value.height), ImVec2(g_menu_value.width / 2 + 100, g_menu_value.height), ImVec4(0, 0, 0, 1.0f), 50);
    ImDraw::DrawCircle(g_menu_value.width / 2 - 100, g_menu_value.height, 25, true, ImVec4(0, 0, 0, 1.0f));
    ImDraw::DrawCircle(g_menu_value.width / 2 + 100, g_menu_value.height, 25, true, ImVec4(0, 0, 0, 1.0f));
    ImDraw::DrawText(ImVec2(g_menu_value.width / 2, g_menu_value.height - 12), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Powered By Zygisk", regular, 20.0f);
}

EGLBoolean (*orig_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
EGLBoolean proxy_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    eglQuerySurface(dpy, surface, EGL_WIDTH, &g_menu_value.width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &g_menu_value.height);
    float min_dim = std::min(g_menu_value.width, g_menu_value.height);
    float scale = min_dim / 450.0f;
    float scale_x = (g_menu_value.screen_width > 0) ? ((float)g_menu_value.width / g_menu_value.screen_width) : 1.0f;
    float scale_y = (g_menu_value.screen_height > 0) ? ((float)g_menu_value.height / g_menu_value.screen_height) : 1.0f;

    if (!g_menu_value.imgui_initialized) {
        SetupImGui();
        eglSwapInterval(dpy, 1); // vsync
        g_menu_value.imgui_initialized = true;
    }

    ImGuiIO &io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();

    io.DisplaySize = ImVec2((float)g_menu_value.width, (float)g_menu_value.height);
    io.MousePos = ImVec2(g_last_touch.x * scale_x, g_last_touch.y * scale_y);
    io.MouseDown[0] = g_last_touch.down;
    io.FontGlobalScale = scale;

    style.WindowPadding = ImVec2(8 * scale, 8 * scale);
    style.FramePadding  = ImVec2(6 * scale, 6 * scale);
    style.ItemSpacing   = ImVec2(8 * scale, 8 * scale);
    style.ItemInnerSpacing = ImVec2(4 * scale, 4 * scale);
    style.TouchExtraPadding = ImVec2(4 * scale, 4 * scale);
    style.WindowMinSize = { 300 * scale, 300 * scale };
    style.ScrollbarSize = 16 * scale;
    style.GrabMinSize   = 16 * scale;

    ImGui_ImplOpenGL3_NewFrame();

    ImGui::NewFrame();

    DrawMenu(io.MousePos);

    ImGui::EndFrame();
    ImGui::Render();
	GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
    GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);

	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glScissor(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glViewport(last_viewport[0], last_viewport[1], last_viewport[2], last_viewport[3]);
    glScissor(last_scissor_box[0], last_scissor_box[1], last_scissor_box[2], last_scissor_box[3]);
    return orig_eglSwapBuffers(dpy, surface);
}
