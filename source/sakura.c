#include "sakura.h"

#include <stdlib.h>

#include "assembler.h"

SakuraState *sakura_createState() {
    SakuraState *state = malloc(sizeof(SakuraState));
    if (state != NULL) {
        state->stackIndex = 0;
        state->currentState = SAKURA_FLAG_ENDED;
        state->error = SAKURA_EFLAG_NONE;

        state->pool.size = 0;
        state->pool.capacity = 8;
        state->pool.constants = malloc(state->pool.capacity * sizeof(TValue));

        state->globals = malloc(128 * sizeof(int));
        state->globalsSize = 128;
        state->globalsIndex = 0;
        state->callStack = malloc(128 * sizeof(int));
        state->callStackSize = 128;
        state->callStackIndex = 0;
    }

    return state;
}

void sakura_destroyState(SakuraState *state) {
    if (state != NULL) {
        free(state->pool.constants);
        state->pool.constants = NULL;
        state->pool.size = 0;
        free(state->globals);
        state->globals = NULL;
        state->globalsSize = 0;
        state->globalsIndex = 0;
        free(state->callStack);
        state->callStack = NULL;
        state->callStackSize = 0;
        state->callStackIndex = 0;
        free(state);
    }
}

TValue sakuraY_makeTNumber(double value) {
    TValue val;
    val.tt = SAKURA_TNUMFLT;
    val.value.n = value;
    return val;
}

TValue sakuraY_makeTString(struct s_str *value) {
    TValue val;
    val.tt = SAKURA_TSTR;
    val.value.s = s_str_copy(value);
    return val;
}

// void sakuraY_push(SakuraState *S, TValue val) {
//     if (S->stackIndex >= SAKURA_STACK_SIZE) {
//         printf("Error: stack overflow\n");
//         exit(1);
//     }
//     S->stack[S->stackIndex++] = val;
// }

// TValue sakuraY_pop(SakuraState *S) {
//     if (S->stackIndex <= 0) {
//         printf("Error: stack underflow\n");
//         exit(1);
//     }
//     return S->stack[--S->stackIndex];
// }

TValue sakuraY_getStack(SakuraState *S, int index) {
    if (index < 0) {
        int tidx = -index - 1;
        if (tidx >= S->pool.size) {
            printf("Error: constant index out of bounds\n");
            exit(1);
        }
        return S->pool.constants[tidx];
    }

    if (index >= SAKURA_STACK_SIZE) {
        printf("Error: stack index out of bounds\n");
        exit(1);
    }

    return S->stack[index];
}

// TValue *sakuraY_peak(SakuraState *S) {
//     if (S->stackIndex <= 0) {
//         printf("Error: stack underflow\n");
//         exit(1);
//     }

//     return &(S->stack[S->stackIndex - 1]);
// }

// int sakuraL_isNumber(SakuraState *S) { return sakuraY_peak(S)->tt == SAKURA_TNUMFLT; }

// int sakuraL_isString(SakuraState *S) { return sakuraY_peak(S)->tt == SAKURA_TSTR; }

void copyTValue(TValue *dest, TValue *src) {
    dest->tt = src->tt;
    if (src->tt == SAKURA_TNUMFLT) {
        dest->value.n = src->value.n;
    } else if (src->tt == SAKURA_TSTR) {
        dest->value.s = s_str_copy(&src->value.s);
    } else {
        // Handle other types as needed
    }
}

void sakuraY_mergePools(SakuraState *S, SakuraConstantPool *pool) {
    S->pool.capacity += pool->capacity;
    S->pool.constants = realloc(S->pool.constants, S->pool.capacity * sizeof(TValue));
    for (size_t i = 0; i < pool->size; i++) {
        copyTValue(&S->pool.constants[S->pool.size + i], &pool->constants[i]);
    }
    S->pool.size += pool->size;
}