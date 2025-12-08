#pragma once

struct MenuData {
    int width = 0;
    int height = 0;
    int screen_width = 0;
    int screen_height = 0;
    bool imgui_initialized = false;

    bool IsESP = false;
};
extern MenuData menu;