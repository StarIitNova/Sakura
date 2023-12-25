#include "sakura.h"

#include <stdlib.h>

#include "assembler.h"

unsigned int hash(const char *key, size_t len, size_t capacity) {
    unsigned int hashValue = 0;
    size_t p = 0;
    while (p < len)
        hashValue = (hashValue << 5) + key[p++];
    return hashValue % capacity;
}

SakuraState *sakura_createState() {
    SakuraState *state = malloc(sizeof(SakuraState));
    if (state != NULL) {
        state->stackIndex = 0;
        state->currentState = SAKURA_FLAG_ENDED;
        state->error = SAKURA_EFLAG_NONE;

        state->pool.size = 0;
        state->pool.capacity = 8;
        state->pool.constants = malloc(state->pool.capacity * sizeof(TValue));

        state->callStack = malloc(128 * sizeof(int));
        state->callStackSize = 128;
        state->callStackIndex = 0;

        sakuraX_initializeTVMap(&state->globals, 16);

        // initialize registry
        state->registry.rax.tt = SAKURA_TNUMFLT;
        state->registry.rax.value.n = 0;
        state->registry.rbx.tt = SAKURA_TNUMFLT;
        state->registry.rbx.value.n = 0;
        state->registry.rcx.tt = SAKURA_TNUMFLT;
        state->registry.rcx.value.n = 0;
        state->registry.rdx.tt = SAKURA_TNUMFLT;
        state->registry.rdx.value.n = 0;

        for (size_t i = 0; i < 64; i++) {
            state->registry.args[i].tt = SAKURA_TNUMFLT;
            state->registry.args[i].value.n = 0;
        }
    }

    return state;
}

void sakura_destroyState(SakuraState *state) {
    if (state != NULL) {
        sakuraX_destroyTVMap(&state->globals);
        free(state->pool.constants);
        state->pool.constants = NULL;
        state->pool.size = 0;
        free(state->callStack);
        state->callStack = NULL;
        state->callStackSize = 0;
        state->callStackIndex = 0;
        free(state);
    }
}

void sakuraX_initializeTVMap(struct TVMap *map, size_t initCapacity) {
    map->pairs = malloc(initCapacity * sizeof(struct TVMapPair));
    map->size = 0;
    map->capacity = initCapacity;

    for (size_t i = 0; i < initCapacity; i++) {
        map->pairs[i].init = 0;
    }
}

void sakuraX_resizeTVMap(struct TVMap *map, size_t newCapacity) {
    map->pairs = realloc(map->pairs, newCapacity * sizeof(struct TVMapPair));
    map->capacity = newCapacity;
}

void sakuraX_TVMapInsert(struct TVMap *map, const struct s_str *key, TValue value) {
    if ((double)map->size / map->capacity >= 0.75) {
        sakuraX_resizeTVMap(map, map->capacity * 2);
    }

    size_t idx = hash(key->str, key->len, map->capacity);
    map->pairs[idx].key = s_str_copy(key);
    map->pairs[idx].value = value;
    map->pairs[idx].init = 1;
    map->size++;
}

TValue *sakuraX_TVMapGet(struct TVMap *map, const struct s_str *key) {
    size_t idx = hash(key->str, key->len, map->capacity);
    return map->pairs[idx].init == 1 ? &map->pairs[idx].value : NULL;
}

TValue *sakuraX_TVMapGet_c(struct TVMap *map, const char *key) {
    size_t idx = hash(key, strlen(key), map->capacity);
    return map->pairs[idx].init == 1 ? &map->pairs[idx].value : NULL;
}

void sakuraX_destroyTVMap(struct TVMap *map) {
    for (size_t i = 0; i < map->size; i++) {
        s_str_free(&map->pairs[i].key);
        if (map->pairs[i].value.tt == SAKURA_TSTR) {
            s_str_free(&map->pairs[i].value.value.s);
        }
    }
    free(map->pairs);
    map->pairs = NULL;
    map->size = 0;
    map->capacity = 0;
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

TValue sakuraY_makeTCFunc(int (*fnPtr)(SakuraState *)) {
    TValue val;
    val.tt = SAKURA_TCFUNC;
    val.value.cfn = fnPtr;
    return val;
}

void sakura_setGlobal(SakuraState *S, const struct s_str *name) {
    TValue *val = sakuraX_TVMapGet(&S->globals, name);
    if (val == NULL) {
        sakuraX_TVMapInsert(&S->globals, name, sakuraY_pop(S));
    } else {
        *val = sakuraY_pop(S);
    }
}

void sakuraY_push(SakuraState *S, TValue val) {
    if (S->stackIndex >= SAKURA_STACK_SIZE) {
        printf("Error: stack overflow\n");
        exit(1);
    }
    S->stack[S->stackIndex++] = val;
}

TValue sakuraY_pop(SakuraState *S) {
    if (S->stackIndex <= 0) {
        printf("Error: stack underflow\n");
        exit(1);
    }
    return S->stack[--S->stackIndex];
}

TValue *sakuraY_peak(SakuraState *S) {
    if (S->stackIndex <= 0) {
        printf("Error: stack underflow\n");
        exit(1);
    }

    return &(S->stack[S->stackIndex - 1]);
}

int sakura_peak(SakuraState *S) {
    if (S->stackIndex <= 0) {
        printf("Error: stack underflow\n");
        exit(1);
    }

    return (int)(S->stack[S->stackIndex - 1].value.n);
}

int sakura_isNumber(SakuraState *S) { return sakuraY_peak(S)->tt == SAKURA_TNUMFLT; }
int sakura_isString(SakuraState *S) { return sakuraY_peak(S)->tt == SAKURA_TSTR; }

double sakura_popNumber(SakuraState *S) {
    TValue val = sakuraY_pop(S);
    if (val.tt != SAKURA_TNUMFLT) {
        printf("Error: expected number, got %d\n", val.tt);
        exit(1);
    }
    return val.value.n;
}

struct s_str sakura_popString(SakuraState *S) {
    TValue val = sakuraY_pop(S);
    if (val.tt != SAKURA_TSTR) {
        printf("Error: expected string, got %d\n", val.tt);
        exit(1);
    }
    return val.value.s;
}

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