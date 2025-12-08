#pragma once

#include <android/input.h>

struct InputEvent {
    float x = 0.0f;
    float y = 0.0f;
    bool down = false;
    bool clicked = false;
    bool released = false;
};
extern InputEvent input;

extern void (*orig_input)(void *_this, void *ex_ab, void *ex_ac);
void proxy_input(void *_this, void *ex_ab, void *ex_ac);