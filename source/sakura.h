#pragma once

#include <stdio.h>

#include "sstr.h"

#define SAKURA_STACK_SIZE 100
#define SAKURA_GLOBAL_VARS_SIZE 100

typedef unsigned short SakuraFlag;

#define SAKURA_FLAG_LEXER 0
#define SAKURA_FLAG_PARSER 1
#define SAKURA_FLAG_ASSEMBLING 2
#define SAKURA_FLAG_RUNTIME 3
#define SAKURA_FLAG_ENDED 4

#define SAKURA_EFLAG_NONE 0
#define SAKURA_EFLAG_SYNTAX 1
#define SAKURA_EFLAG_RUNTIME 2
#define SAKURA_EFLAG_FATAL 3

// #define SAKURA_TNUMINT 1 // integer tag
#define SAKURA_TNUMFLT 0 // float tag
#define SAKURA_TSTR 2    // string tag
#define SAKURA_TCFUNC 3  // C function tag
#define SAKURA_TFUNC 5   // function tag
#define SAKURA_TNIL 4    // nil tag

struct SakuraState;

union SakuraValue {
    double n;                         // TNUMFLT
    struct s_str s;                   // TSTR
    int (*cfn)(struct SakuraState *); // TCFUNC
    int nil;                          // TNIL
};

// TValue represents a tagged value
typedef struct {
    int tt; // type tag
    union SakuraValue value;
} TValue;

typedef struct {
    TValue *constants; // contents of the constant pool
    size_t size;
    size_t capacity;
} SakuraConstantPool;

typedef struct {
    TValue rax;
    TValue rbx;
    TValue rcx;
    TValue rdx;

    TValue args[64];
} SakuraRegistry;

struct TVMapPair {
    struct s_str key;
    TValue value;
    int init;
};

struct TVMap {
    struct TVMapPair *pairs;
    size_t size;
    size_t capacity;
};

struct SakuraState {
    TValue stack[SAKURA_STACK_SIZE];
    int stackIndex;
    SakuraRegistry registry;
    SakuraConstantPool pool;
    struct TVMap globals;
    int *callStack;
    size_t callStackSize;
    size_t callStackIndex;
    SakuraFlag error;
    struct s_str errorMessage;
    SakuraFlag currentState;
};

typedef struct SakuraState SakuraState;

SakuraState *sakura_createState();
void sakura_destroyState(SakuraState *state);

void sakuraX_initializeTVMap(struct TVMap *map, size_t initCapacity);
void sakuraX_resizeTVMap(struct TVMap *map, size_t newCapacity);
void sakuraX_TVMapInsert(struct TVMap *map, const struct s_str *key, TValue value);
int sakuraX_TVMapGetIndex(struct TVMap *map, const struct s_str *key);
TValue *sakuraX_TVMapGet(struct TVMap *map, const struct s_str *key);
TValue *sakuraX_TVMapGet_c(struct TVMap *map, const char *key);
void sakuraX_destroyTVMap(struct TVMap *map);

#define SET_TAG(value, tag) ((int)((value) | ((tag) << 29)))
#define GET_TAG(value) ((value) >> 29)
#define REMOVE_TAG(value) ((size_t)((value) & 0x1FFFFFFF))

void sakuraY_mergePools(SakuraState *S, SakuraConstantPool *pool);

TValue sakuraY_makeTNumber(double value);
TValue sakuraY_makeTString(struct s_str *value);
TValue sakuraY_makeTCFunc(int (*fnPtr)(SakuraState *));

void sakura_setGlobal(SakuraState *S, const struct s_str *name);

void sakuraY_push(SakuraState *S, TValue val);
TValue sakuraY_pop(SakuraState *S);
TValue *sakuraY_peak(SakuraState *S);
int sakura_peak(SakuraState *S);

int sakura_isNumber(SakuraState *S);
int sakura_isString(SakuraState *S);

double sakura_popNumber(SakuraState *S);
struct s_str sakura_popString(SakuraState *S);