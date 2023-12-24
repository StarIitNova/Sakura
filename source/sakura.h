#pragma once

#include <stdio.h>

#include "sstr.h"

#define SAKURA_STACK_SIZE 100
#define SAKURA_GLOBAL_VARS_SIZE 100

typedef unsigned short SakuraFlag;

#define SAKURA_FLAG_LEXER 0
#define SAKURA_FLAG_PARSER 1
#define SAKURA_FLAG_RUNTIME 2
#define SAKURA_FLAG_ENDED 3

#define SAKURA_EFLAG_NONE 0
#define SAKURA_EFLAG_SYNTAX 1
#define SAKURA_EFLAG_RUNTIME 2
#define SAKURA_EFLAG_FATAL 3

union SakuraValue {
    double numberVal;
    struct s_str stringVal;
};

struct SakuraState {
    int stack[SAKURA_STACK_SIZE];
    int stackIndex;
    union SakuraValue *memory;
    size_t memorySize;
    size_t memoryIndex;
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

#define SAKURA_TAG_NUMBER 0
#define SAKURA_TAG_STRING 1

#define SET_TAG(value, tag) ((int)((value) | ((tag) << 29)))
#define GET_TAG(value) ((value) >> 29)
#define REMOVE_TAG(value) ((size_t)((value) & 0x1FFFFFFF))

void sakuraY_push(SakuraState *state, union SakuraValue mem, int tag);
union SakuraValue sakuraY_pop(SakuraState *state);
union SakuraValue sakuraY_peak(SakuraState *state);

void sakuraL_pushNumber(SakuraState *state, double value);
void sakuraL_pushString(SakuraState *state, const char *value);
void sakuraL_pushString_s(SakuraState *state, struct s_str *value);

double sakuraL_popNumber(SakuraState *state);
struct s_str sakuraL_popString(SakuraState *state);

double sakuraL_peakNumber(SakuraState *state);
struct s_str sakuraL_peakString(SakuraState *state);

int sakuraL_isNumber(SakuraState *state);
int sakuraL_isString(SakuraState *state);