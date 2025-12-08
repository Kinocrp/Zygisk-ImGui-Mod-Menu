#pragma once

#include "imgui_internal.h"

namespace ImDraw {
    void DrawLine(ImVec2 start, ImVec2 end, ImVec4 color, float thickness) {
        auto background = ImGui::GetBackgroundDrawList();
        if(background) {
            background->AddLine(start, end, ImColor(color.x, color.y, color.z, color.w), thickness);
        }
    }

    void DrawRect(ImVec4 rect, ImVec4 color, float thickness = 3.0f, float rounding = 0.0f, int flag = 0) {
        auto background = ImGui::GetBackgroundDrawList();
        if (background) {
            background->AddRect(ImVec2(rect.x, rect.y), ImVec2(rect.x + rect.z, rect.y + rect.w), ImColor(color.x, color.y, color.z, color.w), rounding, flag, thickness);
        }
    }

    void DrawCircle(float X, float Y, float radius, bool filled, ImVec4 color, float thickness = 3.0f) {
        auto background = ImGui::GetBackgroundDrawList();
        if(background) {
            if(filled) {
                background->AddCircleFilled(ImVec2(X, Y), radius, ImColor(color.x, color.y ,color.z ,color.w));
            } else {
                background->AddCircle(ImVec2(X, Y), radius, ImColor(color.x, color.y, color.z, color.w), 20, thickness);
            }
        }
    }

    void DrawText(ImVec2 position, ImVec4 color, const char *text, auto fontStyle, float size) {
        auto background = ImGui::GetBackgroundDrawList();
        if (!background) return;

        ImVec2 textSize = fontStyle->CalcTextSizeA(size, FLT_MAX, 0.0f, text);
        ImVec2 centeredPos = ImVec2(position.x - textSize.x * 0.5f, position.y - textSize.y * 0.5f);

        background->AddText(fontStyle, size, centeredPos, ImColor(color), text);
    }
}