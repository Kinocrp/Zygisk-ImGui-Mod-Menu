//
// Created by Perfare on 2020/7/4.
//

#ifndef ZYGISK_MOD_MENU_HACK_H
#define ZYGISK_MOD_MENU_HACK_H

#include <stddef.h>

void hack_prepare(const char *game_data_dir, void *data, size_t length);
#define HOOKAF(ret, func, ...) \
    ret (*orig##func)(__VA_ARGS__); \
    ret my##func(__VA_ARGS__)

#endif //ZYGISK_MOD_MENU_HACK_H
