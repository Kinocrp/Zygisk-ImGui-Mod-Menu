#pragma once

#include <jni.h>

struct MenuValue {
    int width = 0;
    int height = 0;
    int screen_width = 0;
    int screen_height = 0;
    bool imgui_initialized = false;

    bool IsESP = false;
};
extern MenuValue g_menu_value;
