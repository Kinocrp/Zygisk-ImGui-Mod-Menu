#pragma once

#include "menu-value.h"
#include <jni.h>

#pragma pack(push, 8)
struct ReservedData {
    const char *game_data_dir;
    MenuValue *menu_value;
};
#pragma pack(pop)
extern ReservedData reserved;