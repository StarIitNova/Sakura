#pragma once

#include <stdio.h>

#include "sstr.h"
#include "sstructures.h"

SakuraState *sakura_createState(void);
void sakura_destroyState(SakuraState *state);

void sakuraDEBUG_dumpStack(SakuraState *S);
void sakuraDEBUG_dumpTokens(struct TokenStack *tokens);

unsigned int sakuraX_hashForTVMap(const char *key, size_t len, size_t capacity);
void sakuraX_initializeTVMap(struct TVMap *map, size_t initCapacity);
void sakuraX_resizeTVMap(struct TVMap *map, size_t newCapacity);
void sakuraX_TVMapInsert(struct TVMap *map, const struct s_str *key, TValue value);
int sakuraX_TVMapGetIndex(struct TVMap *map, const struct s_str *key);
TValue *sakuraX_TVMapGet(struct TVMap *map, const struct s_str *key);
TValue *sakuraX_TVMapGet_c(struct TVMap *map, const char *key);
void sakuraX_destroyTVMap(struct TVMap *map);

void copyTValue(TValue *dest, TValue *src);

#define SET_TAG(value, tag) ((int)((value) | ((tag) << 29)))
#define GET_TAG(value) ((value) >> 29)
#define REMOVE_TAG(value) ((size_t)((value) & 0x1FFFFFFF))

void sakuraY_mergePools(SakuraState *S, SakuraConstantPool *pool);
void sakuraY_mergePoolsA(SakuraConstantPool *into, SakuraConstantPool *from);

void sakuraY_attemptFreeTValue(TValue *val);

TValue sakuraY_makeTNumber(double value);
TValue sakuraY_makeTString(struct s_str *value);
TValue sakuraY_makeTCFunc(int (*fnPtr)(SakuraState *));
TValue sakuraY_makeTFunc(struct SakuraAssembly *assembly);
TValue sakuraY_makeTTable(void);

void sakura_setGlobal(SakuraState *S, const struct s_str *name);

void sakuraY_push(SakuraState *S, TValue val);
TValue sakuraY_pop(SakuraState *S);
TValue sakuraY_popN(SakuraState *S, int n);
TValue *sakuraY_peek(SakuraState *S);
int sakura_peek(SakuraState *S);

void sakuraY_storeLocal(SakuraState *S, const struct s_str *name, int idx);

int sakura_isNumber(SakuraState *S);
int sakura_isString(SakuraState *S);

double sakura_popNumber(SakuraState *S);
struct s_str sakura_popString(SakuraState *S);

unsigned int sakuraX_hashTValue(const TValue *key, size_t capacity);
int sakuraX_compareTValues(const TValue *a, const TValue *b);