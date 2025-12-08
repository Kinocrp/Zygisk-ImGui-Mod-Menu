#include "menu-input.h"
#include <android/input.h>

InputEvent input;

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
                input.x = x;
                input.y = y;
                input.down = true;
                input.clicked = true;
                input.released = false;
                break;

            case AMOTION_EVENT_ACTION_MOVE:
                input.x = x;
                input.y = y;
                input.down = true;
                input.clicked = false;
                input.released = false;
                break;

            case AMOTION_EVENT_ACTION_UP:
                input.x = x;
                input.y = y;
                input.down = false;
                input.clicked = false;
                input.released = true;
                break;
        }
    }
}