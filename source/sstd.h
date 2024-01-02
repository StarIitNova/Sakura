#pragma once

#include "sakura.h"

int sakuraS_print(SakuraState *S);
int sakuraS_loadstring(SakuraState *S);
int sakuraS_loadfile(SakuraState *S);
int sakuraS_dofile(SakuraState *S);