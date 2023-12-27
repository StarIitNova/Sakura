#pragma once

#include "sakura.h"

#define sakura_register(S, name, ptr) sakuraL_registerGlobalFn(S, name, ptr)
#define sakura_loadstring(S, source) sakuraL_loadstring_c(S, source, 0)
#define sakura_loadfile(S, file) sakuraL_loadfile(S, file, 0)

void sakuraL_loadStdlib(SakuraState *S);

void sakuraL_loadfile(SakuraState *S, const char *file, int showDisasm);
void sakuraL_loadstring(SakuraState *S, struct s_str *source, int showDisasm);
void sakuraL_loadstring_c(SakuraState *S, const char *source, int showDisasm);

void sakuraL_registerGlobalFn(SakuraState *S, const char *name, int (*fnPtr)(SakuraState *));