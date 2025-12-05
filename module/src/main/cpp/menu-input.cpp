#include "menu-input.h"
#include <android/input.h>

TouchEvent g_last_touch;

void (*orig_input)(void *_this, void *ex_ab, void *ex_ac) = nullptr;
void proxy_input(void *_this, void *ex_ab, void *ex_ac) {
    orig_input(_this, ex_ab, ex_ac);
    if (!_this) return;
    
    AInputEvent* event = (AInputEvent*)_this;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        int action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);

        switch (action) {
            case AMOTION_EVENT_ACTION_DOWN:
                g_last_touch.x = x;
                g_last_touch.y = y;
                g_last_touch.down = true;
                g_last_touch.clicked = true;
                g_last_touch.released = false;
                break;

            case AMOTION_EVENT_ACTION_MOVE:
                g_last_touch.x = x;
                g_last_touch.y = y;
                g_last_touch.down = true;
                g_last_touch.clicked = false;
                g_last_touch.released = false;
                break;

            case AMOTION_EVENT_ACTION_UP:
                g_last_touch.x = x;
                g_last_touch.y = y;
                g_last_touch.down = false;
                g_last_touch.clicked = false;
                g_last_touch.released = true;
                break;
        }
    }
}