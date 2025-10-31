//
// Created by Kino on 10/26/2025.
//

#include "globals.h"

TouchEvent g_last_touch;

// Menu

int g_width = 0;
int g_height = 0;
bool g_imgui_initialized = false;

// ESP

std::vector<ESPObject> g_ESPObjects = {
        { nullptr, 1, 100.f, 100.f},
        { nullptr, 2, 150.f, 150.f},
        { nullptr, 3, 200.f, 200.f}
};
float ESP_FPS = 60.0f;
void* mainCamera = nullptr;

// Global Value

bool IsESP = false;