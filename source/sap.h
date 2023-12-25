#pragma once

#include "sakura.h"

#define sakura_register(S, name, ptr) sakuraL_registerGlobalFn(S, name, ptr)

void sakuraL_loadStdlib(SakuraState *S);

void sakuraL_loadfile(SakuraState *S, const char *file);
void sakuraL_loadstring(SakuraState *S, struct s_str *source);
void sakuraL_loadstring_c(SakuraState *S, const char *source);

void sakuraL_registerGlobalFn(SakuraState *S, const char *name, int (*fnPtr)(SakuraState *));