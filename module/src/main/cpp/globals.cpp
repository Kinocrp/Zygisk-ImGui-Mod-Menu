//
// Created by Kino on 10/26/2025.
//

#include "globals.h"

TouchEvent g_last_touch;

// ESP Struct

std::vector<ESPObject> g_ESPObjects = {
        {"Dummy1", 100.f, 100.f},
        {"Dummy2", 150.f, 150.f},
        {"Dummy3", 200.f, 200.f}
};

// Global Value

bool IsESP = false;