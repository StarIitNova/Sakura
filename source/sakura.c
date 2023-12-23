#include "sakura.h"

#include <stdlib.h>

SakuraState *sakura_createState() {
    SakuraState *state = malloc(sizeof(SakuraState));
    if (state != NULL) {
        state->memory = malloc(128 * sizeof(struct SakuraValue));
        state->memorySize = 128;
        state->globals = malloc(128 * sizeof(int));
        state->globalsSize = 128;
        state->callStack = malloc(128 * sizeof(int));
        state->callStackSize = 128;
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

void sakura_push(SakuraState *state, int value) {
    if (state->stackIndex < SAKURA_STACK_SIZE) {
        state->stack[state->stackIndex++] = value;
    } else {
        printf("Error: stack overflow\n");
    }
}

int sakura_pop(SakuraState *state) {
    if (state->stackIndex > 0) {
        return state->stack[--state->stackIndex];
    } else {
        printf("Error: stack underflow\n");
        return 0;
    }
}