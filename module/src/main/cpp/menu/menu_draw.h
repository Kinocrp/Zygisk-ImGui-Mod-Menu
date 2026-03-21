#pragma once

#include "imgui_internal.h"

namespace ImDraw {
    void DrawLine(ImDrawList *drawList, ImVec2 p1, ImVec2 p2, ImU32 col, float thickness = 1.0f) {
        drawList->AddLine(p1, p2, col, thickness);
    }

    void DrawRect(ImDrawList *drawList, bool filled, ImVec2 p_min, ImVec2 p_max, ImU32 col, float rounding = 0.0f, float thickness = 1.0f) {
        if (filled) {
            drawList->AddRectFilled(p_min, p_max, col, rounding);
        } else {
            drawList->AddRect(p_min, p_max, col, rounding, 0, thickness);
        }
    }

    void DrawCircle(ImDrawList *drawList, bool filled, ImVec2 center, float radius, ImU32 col, float thickness = 1.0f) {
        if (filled) {
            drawList->AddCircleFilled(center, radius, col);
        } else {
            drawList->AddCircle(center, radius, col, 0, thickness);
        }
    }

    void DrawText(ImDrawList *drawList, ImFont *font, float font_size, ImVec2 pos, ImU32 col, const char *text) {
        if (!font) font = ImGui::GetFont();
        ImVec2 textSize = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, text);
        ImVec2 centeredPos = ImVec2(pos.x - textSize.x * 0.5f, pos.y - textSize.y * 0.5f);
        drawList->AddText(font, font_size, centeredPos, col, text);
    }
}