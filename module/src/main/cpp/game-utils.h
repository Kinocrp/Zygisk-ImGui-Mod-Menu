#pragma once

#include <jni.h>
#include "menu-value.h"

void RestartProcess(JavaVM *vm);
void SetScreenSize(JavaVM *vm, MenuValue *menu);
