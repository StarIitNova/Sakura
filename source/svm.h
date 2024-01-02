#pragma once

#include "assembler.h"
#include "stable.h"

int sakuraX_interpretA(SakuraState *S, struct SakuraAssembly *assembly, int offset);
int sakuraX_interpret(SakuraState *S, struct SakuraAssembly *assembly);