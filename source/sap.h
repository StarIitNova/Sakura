#pragma once

#include "sakura.h"

void sakuraL_loadfile(SakuraState *S, const char *file);
void sakuraL_loadstring(SakuraState *S, struct s_str *source);
void sakuraL_loadstring_c(SakuraState *S, const char *source);