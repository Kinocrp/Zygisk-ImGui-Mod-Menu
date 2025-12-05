#pragma once

#include "menu-value.h"

#pragma pack(push, 8)
struct ReservedData {
    const char *game_data_dir;
    void *data;
    size_t length;
    JavaVM *vm;

    MenuValue *menu_value;
};
#pragma pack(pop)
extern ReservedData reserved;