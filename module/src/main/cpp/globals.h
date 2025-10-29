//
// Created by Kino on 10/26/2025.
//

#ifndef ZYGISK_MOD_MENU_GLOBALS_H
#define ZYGISK_MOD_MENU_GLOBALS_H

#pragma once

#include "imgui.h"
#include <string>
#include <vector>

struct TouchEvent {
    float x = 0.0f;
    float y = 0.0f;
    bool down = false;
    bool clicked = false;
    bool released = false;
};
extern TouchEvent g_last_touch;

// ESP Struct

struct ESPObject {
    std::string name;
    float x, y;
};
extern std::vector<ESPObject> g_ESPObjects;

// Global Value

extern bool IsESP;

#endif //ZYGISK_MOD_MENU_GLOBALS_H
