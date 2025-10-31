//
// Created by Kino on 10/26/2025.
//

#ifndef ZYGISK_MOD_MENU_GLOBALS_H
#define ZYGISK_MOD_MENU_GLOBALS_H

#pragma once

#include "imgui.h"
#include <string>
#include <vector>
#include <thread>

struct TouchEvent {
    float x = 0.0f;
    float y = 0.0f;
    bool down = false;
    bool clicked = false;
    bool released = false;
};
extern TouchEvent g_last_touch;

// Menu

extern int g_width;
extern int g_height;
extern bool g_imgui_initialized;

// ESP

extern std::thread ESPThread;
struct ESPObject {
    void* espObj;
    int objID;
    float x, y;
};
extern std::vector<ESPObject> g_ESPObjects;
extern float ESP_FPS;
extern void* mainCamera;

// Global Value

extern bool IsESP;

#endif //ZYGISK_MOD_MENU_GLOBALS_H
