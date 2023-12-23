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

enum SakuraValueType { SAKURA_INT, SAKURA_FLOAT, SAKURA_STRING };

struct SakuraValue {
    enum SakuraValueType type;
    union {
        long long intVal;
        double floatVal;
        char *stringVal;
    } value;
};

struct SakuraState {
    int stack[SAKURA_STACK_SIZE];
    int stackIndex;
    struct SakuraValue *memory;
    size_t memorySize;
    int *globals;
    size_t globalsSize;
    int *callStack;
    size_t callStackSize;
    int callStackIndex;
    SakuraFlag error;
    struct s_str errorMessage;
    SakuraFlag currentState;
};

typedef struct SakuraState SakuraState;

SakuraState *sakura_createState();
void sakura_destroyState(SakuraState *state);

void sakura_push(SakuraState *state, int value);
int sakura_pop(SakuraState *state);