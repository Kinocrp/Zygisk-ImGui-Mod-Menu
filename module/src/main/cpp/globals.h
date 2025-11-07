//
// Created by Kino on 10/26/2025.
//

#ifndef ZYGISK_IMGUI_MOD_MENU_GLOBALS_H
#define ZYGISK_IMGUI_MOD_MENU_GLOBALS_H

#pragma once

#include "imgui.h"
#include "esp-manager.h"

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
extern bool g_hook_status;

// ESP
extern ESPManager g_ESPManager;

// Global Value
extern bool IsESP;

#endif //ZYGISK_IMGUI_MOD_MENU_GLOBALS_H
