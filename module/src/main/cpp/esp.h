//
// Created by Kino on 10/27/2025.
//

#ifndef ZYGISK_MOD_MENU_ESP_H
#define ZYGISK_MOD_MENU_ESP_H

#include "imgui_internal.h"
#include "font.h"

namespace ESP {
    void DrawLine(ImVec2 start, ImVec2 end, ImVec4 color, float size) {
        auto background = ImGui::GetBackgroundDrawList();
        if(background) {
            background->AddLine(start, end, ImColor(color.x,color.y,color.z,color.w), size);
        }
    }

    void DrawRect(ImVec4 rect, ImVec4 color, float size) {
        ImVec2 p1(rect.x, rect.y);
        ImVec2 p2(rect.x + rect.w, rect.y);
        ImVec2 p3(rect.x + rect.w, rect.y + rect.z);
        ImVec2 p4(rect.x, rect.y + rect.z);

        DrawLine(p1, p2, color, size);
        DrawLine(p2, p3, color, size);
        DrawLine(p3, p4, color, size);
        DrawLine(p4, p1, color, size);
    }

    void DrawCircle(float X, float Y, float radius, bool filled, ImVec4 color) {
        auto background = ImGui::GetBackgroundDrawList();
        if(background) {
            if(filled) {
                background->AddCircleFilled(ImVec2(X, Y), radius, ImColor(color.x,color.y,color.z,color.w));
            } else {
                background->AddCircle(ImVec2(X, Y), radius, ImColor(color.x,color.y,color.z,color.w));
            }
        }
    }

    void DrawText(ImVec2 position, ImVec4 color, const char *text, auto fontStyle, float size) {
        auto drawList = ImGui::GetBackgroundDrawList();
        if (!drawList) return;

        // Calculate text size using the given font and size
        ImVec2 textSize = fontStyle->CalcTextSizeA(size, FLT_MAX, 0.0f, text);

        // Adjust position to center (both X and Y)
        ImVec2 centeredPos = ImVec2(
                position.x - textSize.x * 0.5f,
                position.y - textSize.y * 0.5f
        );

        drawList->AddText(fontStyle, size, centeredPos, ImColor(color), text);
    }
}


#endif //ZYGISK_MOD_MENU_ESP_H
