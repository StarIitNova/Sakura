#pragma once

#include "assembler.h"

char *sakuraX_readTVal(TValue *val);
void sakuraX_writeDisasm(SakuraState *S, struct SakuraAssembly *assembler, const char *filename, int mode);