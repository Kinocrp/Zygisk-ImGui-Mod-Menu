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
std::thread g_ESPThread;
ESPManager g_ESPManager;

// Global Value

bool IsESP = false;