#include "svm.h"

#include <math.h>
#include <stdlib.h>

#include "sakura.h"

#include "disasm.h"

#define REGISTER_BINOP(name, operation)                                                                                \
    reg = instructions[i + 1];                                                                                         \
    TValue val = sakuraY_pop(S);                                                                                       \
    TValue val2 = sakuraY_pop(S);                                                                                      \
    if (val.tt == SAKURA_TNUMFLT) {                                                                                    \
        if (val2.tt == SAKURA_TNUMFLT) {                                                                               \
            double a = val2.value.n;                                                                                   \
            double b = val.value.n;                                                                                    \
            sakuraY_push(S, sakuraY_makeTNumber(operation));                                                           \
        } else {                                                                                                       \
            printf("Error: unknown " name " operands: %d %d\n", val.tt, val2.tt);                                      \
        }                                                                                                              \
    } else {                                                                                                           \
        printf("Error: unknown " name " operands: %d\n", val.tt);                                                      \
    }                                                                                                                  \
    i += 3;

int sakuraX_interpretA(SakuraState *S, struct SakuraAssembly *assembly, int offset) {
    S->currentState = SAKURA_FLAG_RUNTIME;

    sakuraY_mergePools(S, &assembly->pool);

    if (offset > 0) {
        int argcA = sakuraY_peek(S)->value.n;
        // printf("interpretA called with %d args\n", argcA);
        // sakuraDEBUG_dumpStack(S);
    }

    int preStackIdx = S->stackIndex;

    int *instructions = assembly->instructions;
    for (size_t i = 0; i < assembly->size; i++) {
        int reg = -1;
        switch (instructions[i]) {
        case SAKURA_LOADK:
            // ignore the first argument (store reg) as it is NOT needed
            sakuraY_push(S, assembly->pool.constants[-instructions[i + 2] - 1]);
            i += 2;
            break;
        case SAKURA_SETGLOBAL:
            // ignore the first argument (store reg) as it is NOT needed
            sakura_setGlobal(S, &assembly->pool.constants[-instructions[i + 2] - 1].value.s);
            i += 2;
            break;
        case SAKURA_GETGLOBAL:
            // ignore the first argument (store reg) as it is NOT needed
            // printf("function %p pushed\n", S->globals.pairs[instructions[i + 2]].value.value.cfn);
            sakuraY_push(S, S->globals.pairs[instructions[i + 2]].value);
            i += 2;
            break;
        case SAKURA_CLOSURE:
            // ignore the first argument (store reg) as it is NOT needed
            sakuraY_push(S, sakuraY_makeTFunc(assembly->closures[instructions[i + 2]]));
            i += 2;
            break;
        case SAKURA_MOVE:
            S->stack[instructions[i + 1]] = S->stack[instructions[i + 2]];
            if (instructions[i + 1] == S->stackIndex)
                S->stackIndex++;
            i += 2;
            break;
        case SAKURA_ADD: {
            reg = instructions[i + 1];
            TValue val = sakuraY_pop(S);
            TValue val2 = sakuraY_pop(S);
            if (val.tt == SAKURA_TNUMFLT) {
                if (val2.tt == SAKURA_TNUMFLT) {
                    sakuraY_push(S, sakuraY_makeTNumber(val2.value.n + val.value.n));
                } else if (val2.tt == SAKURA_TSTR) {
                    struct s_str v = s_str_concat_d(&val2.value.s, val.value.n);
                    sakuraY_push(S, sakuraY_makeTString(&v));
                    s_str_free(&v);
                } else {
                    printf("Error: unknown addition operands\n");
                }
            } else if (val.tt == SAKURA_TSTR) {
                if (val2.tt == SAKURA_TNUMFLT) {
                    struct s_str v = s_str_concat_dd(val2.value.n, &val.value.s);
                    sakuraY_push(S, sakuraY_makeTString(&v));
                    s_str_free(&v);
                } else if (val2.tt == SAKURA_TSTR) {
                    struct s_str v = s_str_concat(&val2.value.s, &val.value.s);
                    sakuraY_push(S, sakuraY_makeTString(&v));
                    s_str_free(&v);
                } else {
                    printf("Error: unknown addition operands\n");
                }
            } else {
                printf("Error: what the frick is this\ntry again.\n");
            }
            i += 3;
            break;
        }
        case SAKURA_MUL: {
            REGISTER_BINOP("multiplication", a * b);
            break;
        }
        case SAKURA_DIV: {
            REGISTER_BINOP("division", a / b);
            break;
        }
        case SAKURA_MOD: {
            REGISTER_BINOP("modulo", fmod(a, b));
            break;
        }
        case SAKURA_POW: {
            REGISTER_BINOP("power", pow(a, b));
            break;
        }
        case SAKURA_LT: {
            REGISTER_BINOP("less-than", a < b ? 1 : 0);
            break;
        }
        case SAKURA_LE: {
            REGISTER_BINOP("less-than-or-equal-to", a <= b ? 1 : 0);
            break;
        }
        case SAKURA_EQ: {
            REGISTER_BINOP("equal-to", a == b ? 1 : 0);
            break;
        }
        case SAKURA_CALL: {
            int fnLoc = instructions[i + 1] + offset + S->internalOffset;
            int argc = instructions[i + 2];
            TValue *fn = &S->stack[fnLoc];
            if (fn->tt == SAKURA_TCFUNC) {
                int stackidx = S->stackIndex;
                sakuraY_push(S, sakuraY_makeTNumber(argc));
                int ret = fn->value.cfn(S);

                if (stackidx - S->stackIndex + ret != argc) {
                    printf("Warning: C function did not pop all arguments off the stack (%d removed, %d expected)\n",
                           stackidx - S->stackIndex, argc);
                } else {
                    sakuraY_popN(S, fnLoc); // pops the function
                }
            } else if (fn->tt == SAKURA_TFUNC) {
                int stackidx = S->stackIndex;
                sakuraY_push(S, sakuraY_makeTNumber(argc));
                int ret = sakuraX_interpretA(S, fn->value.assembly, S->stackIndex);

                if (stackidx - S->stackIndex + ret != argc) {
                    printf(
                        "Warning: Sakura function did not pop all arguments off the stack (%d removed, %d expected)\n",
                        stackidx - S->stackIndex, argc);
                } else {
                    TValue val = sakuraY_popN(S, fnLoc); // pops the function
                    sakuraY_attemptFreeTValue(&val);
                }
            }
            i += 2;
            break;
        }
        case SAKURA_JMP: {
            i = instructions[i + 1] - 1;
            break;
        }
        case SAKURA_JMPIF: {
            TValue val = sakuraY_pop(S);
            if (val.tt == SAKURA_TNUMFLT) {
                if (val.value.n == 0) {
                    i = instructions[i + 1] - 1;
                } else {
                    i += 2;
                }
            } else {
                printf("Error: unknown jump-if operand\n");
            }
            break;
        }
        case SAKURA_RETURN: {
            if (offset > 0) {
                for (int i = 0; i < S->stackIndex - preStackIdx; i++) {
                    sakuraY_pop(S);
                }
            }
            i += 2;
            break;
        }
        default:
            printf("Error: unknown/unimplemented runtime instruction '%d' @ %d\n", instructions[i], i);
            break;
        }
    }

    if (offset > 0) {
        sakuraY_pop(S); // pop argc off of the stack
    }

    return 0;
}

int sakuraX_interpret(SakuraState *S, struct SakuraAssembly *assembly) { return sakuraX_interpretA(S, assembly, 0); }