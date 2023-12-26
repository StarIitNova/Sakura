#include "svm.h"

#include <stdlib.h>

#include "sakura.h"

void sakuraX_interpret(SakuraState *S, struct SakuraAssembly *assembly) {
    S->currentState = SAKURA_FLAG_RUNTIME;

    sakuraY_mergePools(S, &assembly->pool);

    int *instructions = assembly->instructions;
    for (size_t i = 0; i < assembly->size; i++) {
        int reg = -1;
        switch (instructions[i]) {
        case SAKURA_LOADK:
            // ignore the first argument (store reg) as it is NOT needed
            sakuraY_push(S, assembly->pool.constants[-instructions[i + 2] - 1]);
            i += 2;
            break;
        case SAKURA_GETGLOBAL:
            // ignore the first argument (store reg) as it is NOT needed
            sakuraY_push(S, S->globals.pairs[instructions[i + 2]].value);
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
            reg = instructions[i + 1];
            TValue val = sakuraY_pop(S);
            TValue val2 = sakuraY_pop(S);
            if (val.tt == SAKURA_TNUMFLT) {
                if (val2.tt == SAKURA_TNUMFLT) {
                    sakuraY_push(S, sakuraY_makeTNumber(val2.value.n * val.value.n));
                } else {
                    printf("Error: unknown multiplication operands: %d %d\n", val.tt, val2.tt);
                }
            } else {
                printf("Error: unknown multiplication operands: %d\n", val.tt);
            }
            i += 3;
            break;
        }
        case SAKURA_CALL: {
            int fnLoc = instructions[i + 1];
            int argc = instructions[i + 2];
            TValue *fn = &S->stack[fnLoc];
            if (fn->tt == SAKURA_TCFUNC) {
                sakuraY_push(S, sakuraY_makeTNumber(argc));
                int ret = fn->value.cfn(S);
                if (ret != 0) {
                    printf("Error: C function returned non-zero (%d) value\n", ret);
                    exit(1);
                }
            } else if (fn->tt == SAKURA_TFUNC) {
                printf("Error: function calls are not implemented yet\n");
            }
            i += 2;
            break;
        }
        default:
            printf("Error: unknown/unimplemented runtime instruction '%d'\n", instructions[i]);
            break;
        }
    }
}