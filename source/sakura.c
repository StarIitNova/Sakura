#include "sakura.h"

#include <stdlib.h>

SakuraState *sakura_createState() {
    SakuraState *state = malloc(sizeof(SakuraState));
    if (state != NULL) {
        state->memory = malloc(128 * sizeof(union SakuraValue));
        state->memorySize = 128;
        state->memoryIndex = 0;
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
        free(state->memory);
        free(state->globals);
        free(state->callStack);
        free(state);
    }
}

void sakuraY_push(SakuraState *state, union SakuraValue mem, int tag) {
    if (state->stackIndex < SAKURA_STACK_SIZE) {
        if (state->memoryIndex >= state->memorySize) {
            state->memorySize *= 2;
            state->memory = realloc(state->memory, state->memorySize);
        }
        state->memory[state->memoryIndex++] = mem;
        state->stack[state->stackIndex++] = SET_TAG(state->memoryIndex - 1, tag);
    } else {
        printf("Error: stack overflow\n");
    }
}

union SakuraValue sakuraY_pop(SakuraState *state) {
    if (state->stackIndex > 0) {
        return state->memory[REMOVE_TAG(state->stack[--state->stackIndex])];
    } else {
        printf("Error: stack underflow\n");
        return (union SakuraValue){0.0};
    }
}

union SakuraValue sakuraY_peak(SakuraState *state) {
    if (state->stackIndex > 0) {
        return state->memory[REMOVE_TAG(state->stack[state->stackIndex - 1])];
    } else {
        printf("Error: stack underflow\n");
        return (union SakuraValue){0.0};
    }
}

void sakuraL_pushNumber(SakuraState *state, double value) {
    union SakuraValue val;
    val.numberVal = value;
    sakuraY_push(state, val, SAKURA_TAG_NUMBER);
}

void sakuraL_pushString(SakuraState *state, const char *value) {
    union SakuraValue val;
    val.stringVal = s_str(value);
    sakuraY_push(state, val, SAKURA_TAG_STRING);
}

void sakuraL_pushString_s(SakuraState *state, struct s_str *value) {
    union SakuraValue val;
    val.stringVal = s_str_copy(value);
    sakuraY_push(state, val, SAKURA_TAG_STRING);
}

double sakuraL_popNumber(SakuraState *state) { return sakuraY_pop(state).numberVal; }
struct s_str sakuraL_popString(SakuraState *state) { return sakuraY_pop(state).stringVal; }

double sakuraL_peakNumber(SakuraState *state) { return sakuraY_peak(state).numberVal; }
struct s_str sakuraL_peakString(SakuraState *state) { return sakuraY_peak(state).stringVal; }

int sakuraL_isNumber(SakuraState *state) { return GET_TAG(state->stack[state->stackIndex - 1]) == SAKURA_TAG_NUMBER; }
int sakuraL_isString(SakuraState *state) { return GET_TAG(state->stack[state->stackIndex - 1]) == SAKURA_TAG_STRING; }