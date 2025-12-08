#pragma once

#include "menu-data.h"
#include <jni.h>

struct ReservedData {
    const char *game_data_dir;
    JavaVM *vm;
    MenuData *menu;
};
extern ReservedData *reserved;
