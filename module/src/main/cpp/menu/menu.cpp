#include "menu.h"
#include <unistd.h>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <sys/stat.h>
#include <android/native_window_jni.h>
#include <android/looper.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include "log.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"

#include "menu_draw.h"
#include "font.h"

#include "il2cpp.h"
#include "service.h"

ImGuiContext* CurrentContext = nullptr;
ImVec2 GUIPosition = ImVec2(100, 100);
float GUIIconSize = 22.0f;
float GUIScale = 1.0f;
bool GUIShow = false;
bool IsTouchGUI = false;

namespace Menu {
    bool IsESP = false;
}

bool TouchGUI() {
    return IsTouchGUI;
}

void InitGUI(ImGuiIO &io, ImGuiStyle &style) {

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

    ImFontConfig cfg_base;
    cfg_base.FontDataOwnedByAtlas = false;
    cfg_base.PixelSnapH = false;
    cfg_base.OversampleH = 5;
    cfg_base.OversampleV = 5;
    cfg_base.RasterizerMultiply = 1.0f;

    io.Fonts->Clear();
    io.FontDefault = io.Fonts->AddFontFromMemoryCompressedTTF((void*)font_data, font_size, 15.0f, &cfg_base, io.Fonts->GetGlyphRangesChineseFull());
}

void DrawIcon(ImGuiIO &io, ImDrawList *drawList) {
    static bool isDragging = false;
    static bool wasDragging = false;
    static ImVec2 dragOffset = ImVec2(0, 0);

    float radius = GUIIconSize * GUIScale;
    float fontSize = GUIIconSize * GUIScale * 0.7f;

    float dx = io.MousePos.x - GUIPosition.x;
    float dy = io.MousePos.y - GUIPosition.y;
    float distance = sqrtf(dx * dx + dy * dy);
    bool isHovered = (distance <= radius);

    if (ImGui::IsMouseClicked(0) && isHovered) {
        isDragging = true;
        wasDragging = false;
        dragOffset = ImVec2(io.MousePos.x - GUIPosition.x, io.MousePos.y - GUIPosition.y);
    }

    if (isDragging) {
        if (ImGui::IsMouseDown(0)) {
            if (io.MouseDragMaxDistanceSqr[0] > 10.0f) wasDragging = true;
            GUIPosition.x = io.MousePos.x - dragOffset.x;
            GUIPosition.y = io.MousePos.y - dragOffset.y;
        } else {
            if (!wasDragging) GUIShow = !GUIShow;
            isDragging = false;
        }
    }

    GUIPosition.x = ImClamp(GUIPosition.x, radius, io.DisplaySize.x - radius);
    GUIPosition.y = ImClamp(GUIPosition.y, radius, io.DisplaySize.y - radius);

    ImColor circleColor = ImColor(0.16f, 0.16f, 0.21f, 1.0f);
    if (isDragging) circleColor = ImColor(0.24f, 0.24f, 0.32f, 1.0f);
    else if (isHovered) circleColor = ImColor(0.19f, 0.2f, 0.25f, 1.0f);
    IsTouchGUI = isHovered || isDragging;

    ImDraw::DrawCircle(drawList, true, GUIPosition, radius, circleColor);
    ImDraw::DrawCircle(drawList, false, GUIPosition, radius, ImColor(0.74f, 0.58f, 0.98f, 1.0f), 1.5f * GUIScale);
    ImDraw::DrawText(drawList, nullptr, fontSize, ImVec2(GUIPosition.x, GUIPosition.y), ImColor(1.0f, 1.0f, 1.0f, 1.0f), "KINO");
}

void DrawGUI(ImGuiIO &io, ImGuiStyle &style, std::string appName) {
    ImDrawList *drawList = ImGui::GetForegroundDrawList();
    if (GUIShow) {
        static bool isDragging = false;
        static bool dragLocked = false;
        static int count = 0;

        float minDim = std::min(io.DisplaySize.x, io.DisplaySize.y);
        auto menuSize = ImVec2(minDim * 0.7f, minDim * 0.7f);
        float radius = GUIIconSize * GUIScale;

        auto initPos = ImVec2(
                ImClamp(GUIPosition.x - radius, 0.0f, io.DisplaySize.x - menuSize.x),
                ImClamp(GUIPosition.y - radius, 0.0f, io.DisplaySize.y - menuSize.y)
        );

        ImGui::SetNextWindowPos(initPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(menuSize, ImGuiCond_Always);

        if (ImGui::Begin(appName.c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove)) {
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
                GUIPosition.x += io.MouseDelta.x;
                GUIPosition.y += io.MouseDelta.y;
                GUIPosition.x = ImClamp(GUIPosition.x, 0.0f, io.DisplaySize.x - menuSize.x + radius);
                GUIPosition.y = ImClamp(GUIPosition.y, 0.0f, io.DisplaySize.y - menuSize.y + radius);
            }

            ImGui::Checkbox("ESP", &Menu::IsESP);

            ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ImGui::GetFrameHeight() - style.WindowPadding.y);
            if (ImGui::Button("Close Menu", ImVec2(-FLT_MIN, 0))) GUIShow = false;
        }
        ImGui::End();
    } else {
        DrawIcon(io, drawList);
    }
}

float ResizeGUI(ImGuiIO &io, ImGuiStyle &style) {

    if (!CurrentContext) CurrentContext = GetImGuiContext();

    float minDim = std::min(io.DisplaySize.x, io.DisplaySize.y);
    if (minDim < 1.0f) minDim = 1.0f;
    GUIScale = minDim / 400.0f;

    style.WindowPadding = ImVec2(8.0f * GUIScale, 8.0f * GUIScale);
    style.FramePadding = ImVec2(8.0f * GUIScale, 6.0f * GUIScale);
    style.ItemSpacing = ImVec2(8.0f * GUIScale, 8.0f * GUIScale);
    style.ItemInnerSpacing = ImVec2(4.0f * GUIScale, 4.0f * GUIScale);
    style.TouchExtraPadding = ImVec2(0.0f, 0.0f);
    style.IndentSpacing = 21.0f * GUIScale;
    style.ColumnsMinSpacing = 6.0f * GUIScale;
    style.ScrollbarSize = 18.0f * GUIScale;
    style.GrabMinSize = 12.0f * GUIScale;
    style.WindowBorderSize = 1.5f * GUIScale;
    style.ChildBorderSize = 1.0f * GUIScale;
    style.PopupBorderSize = 1.0f * GUIScale;
    style.FrameBorderSize = 0.0f * GUIScale;
    style.TabBorderSize = 0.0f * GUIScale;
    style.WindowRounding = 7.0f * GUIScale;
    style.ChildRounding = 7.0f * GUIScale;
    style.FrameRounding = 4.0f * GUIScale;
    style.PopupRounding = 4.0f * GUIScale;
    style.ScrollbarRounding = 9.0f * GUIScale;
    style.GrabRounding = 3.0f * GUIScale;
    style.TabRounding = 4.0f * GUIScale;
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);

    io.FontGlobalScale = GUIScale;
    return GUIScale;
}

jobject FindActivity(JNIEnv *env) {
    jclass atClass = env->FindClass("android/app/ActivityThread");
    jmethodID currentAtMethod = env->GetStaticMethodID(atClass, "currentActivityThread", "()Landroid/app/ActivityThread;");
    jobject at = env->CallStaticObjectMethod(atClass, currentAtMethod);
    if (!at) return nullptr;

    jfieldID mActivitiesField = env->GetFieldID(atClass, "mActivities", "Landroid/util/ArrayMap;");
    jobject mActivities = env->GetObjectField(at, mActivitiesField);
    if (!mActivities) return nullptr;

    jclass mapClass = env->GetObjectClass(mActivities);
    jmethodID valuesMethod = env->GetMethodID(mapClass, "values", "()Ljava/util/Collection;");
    jobject values = env->CallObjectMethod(mActivities, valuesMethod);
    jclass collectionClass = env->GetObjectClass(values);
    jmethodID toArrayMethod = env->GetMethodID(collectionClass, "toArray", "()[Ljava/lang/Object;");
    jobjectArray array = (jobjectArray)env->CallObjectMethod(values, toArrayMethod);

    int size = env->GetArrayLength(array);
    for (int i = 0; i < size; i++) {
        jobject record = env->GetObjectArrayElement(array, i);
        if (!record) continue;

        jfieldID activityField = env->GetFieldID(env->GetObjectClass(record), "activity", "Landroid/app/Activity;");
        jobject act = env->GetObjectField(record, activityField);
        if (!act) continue;

        jclass activityClass = env->GetObjectClass(act);
        jmethodID isFinishing = env->GetMethodID(activityClass, "isFinishing", "()Z");
        if (env->CallBooleanMethod(act, isFinishing)) continue;

        jmethodID hasWindowFocus = env->GetMethodID(activityClass, "hasWindowFocus", "()Z");
        if (!env->CallBooleanMethod(act, hasWindowFocus)) continue;

        return act;
    }
    return nullptr;
}

void RenderThread(JavaVM *vm, JNIEnv *env) {
    sleep(5);

    jobject Activity = nullptr;
    LOGI("[*] Searching for active activity...");
    while (!Activity) {
        jobject act = FindActivity(env);
        if (act) Activity = env->NewGlobalRef(act);
        usleep(200000);
    }
    LOGI("[+] Activity found");

    InitService(vm, env, Activity, (void*)&InitGUI, (void*)&ResizeGUI, (void*)&DrawGUI, (void*)&TouchGUI);
}