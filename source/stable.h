#pragma once

#include "sakura.h"

struct SakuraTTable *sakuraX_initializeTTable(void);
void sakuraX_resizeTTable(struct SakuraTTable *table, ull newCapacity);
void sakuraX_setTTable(struct SakuraTTable *table, const TValue *key, const TValue *value);
TValue sakuraX_getTTable(struct SakuraTTable *table, const TValue *key);
void sakuraX_freeTTable(struct SakuraTTable *table);