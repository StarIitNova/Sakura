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

union SakuraValue {
    double n;       // TNUMFLT
    struct s_str s; // TSTR
};

// #define SAKURA_TNUMINT 1 // integer tag
#define SAKURA_TNUMFLT 0 // float tag
#define SAKURA_TSTR 2    // string tag

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

struct SakuraState {
    TValue stack[SAKURA_STACK_SIZE];
    int stackIndex;
    SakuraConstantPool pool;
    int *globals;
    size_t globalsSize;
    size_t globalsIndex;
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

// #define SAKURA_TAG_NUMBER 0
// #define SAKURA_TAG_STRING 1

#define SET_TAG(value, tag) ((int)((value) | ((tag) << 29)))
#define GET_TAG(value) ((value) >> 29)
#define REMOVE_TAG(value) ((size_t)((value) & 0x1FFFFFFF))

void sakuraY_mergePools(SakuraState *S, SakuraConstantPool *pool);

TValue sakuraY_makeTNumber(double value);
TValue sakuraY_makeTString(struct s_str *value);

// void sakuraY_push(SakuraState *S, TValue val);
// TValue sakuraY_pop(SakuraState *S);
TValue sakuraY_getStack(SakuraState *S, int index);
// TValue *sakuraY_peak(SakuraState *S);

// int sakuraL_isNumber(SakuraState *S);
// int sakuraL_isString(SakuraState *S);