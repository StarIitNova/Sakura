#include "disasm.h"

#include <stdlib.h>

char *sakuraX_readTVal(TValue *val) {
    char *allocVal = (char *)malloc(1024 * sizeof(char));

    if (val == NULL) {
        sprintf(allocVal, "[UNKNOWN]");
        return allocVal;
    }

    switch (val->tt) {
    case SAKURA_TNUMFLT:
        sprintf(allocVal, "%f", val->value.n);
        break;
    case SAKURA_TSTR:
        sprintf(allocVal, "'%.*s'", val->value.s.len, val->value.s.str);
        break;
    default:
        sprintf(allocVal, "[Unknown T%d D%f @ %p (%p)]", val->tt, val->value.n, val, &val->tt);
        break;
    }

    return allocVal;
}

void sakuraX_writeDisasm(SakuraState *S, struct SakuraAssembly *assembler, const char *filename, int mode) {
    struct s_str **cachedGlobals;
    struct s_str basicCall;
    size_t idx = 1;
    char *allocVal, *allocVal2, *trueFname;

    printf("%s <%s> (%lld instructions, %lld bytes)\n", mode & 1 << 8 ? "function" : "main", filename, assembler->size,
           assembler->size * sizeof(int));
    printf("%lld registers, %lld closures, %lld constants, %lld functions\n", assembler->highestRegister,
           assembler->closureIdx, assembler->pool.size, assembler->functionsLoaded);

    basicCall = s_str("loaded_function");
    cachedGlobals = (struct s_str **)malloc(sizeof(struct s_str *) * (assembler->functionsLoaded * 2));

    for (size_t i = 0; i < assembler->size; i++) {
        switch (assembler->instructions[i]) {
        case SAKURA_LOADK: {
            allocVal = sakuraX_readTVal(&assembler->pool.constants[-assembler->instructions[i + 2] - 1]);
            printf("    %lld\t(%lld)\t\tLOADK\t\t%d\t\t;; %s into stack pos %d\n", idx, i,
                   assembler->instructions[i + 2], allocVal, assembler->instructions[i + 1]);
            free(allocVal);
            i += 2;
            break;
        }
        case SAKURA_ADD: {
            printf("    %lld\t(%lld)\t\tADD\t\t%d, %d, %d\n", idx, i, assembler->instructions[i + 1],
                   assembler->instructions[i + 2], assembler->instructions[i + 3]);
            i += 3;
            break;
        }
        case SAKURA_SUB: {
            printf("    %lld\t(%lld)\t\tSUB\t\t%d, %d, %d\n", idx, i, assembler->instructions[i + 1],
                   assembler->instructions[i + 2], assembler->instructions[i + 3]);
            i += 3;
            break;
        }
        case SAKURA_MUL: {
            printf("    %lld\t(%lld)\t\tMUL\t\t%d, %d, %d\n", idx, i, assembler->instructions[i + 1],
                   assembler->instructions[i + 2], assembler->instructions[i + 3]);
            i += 3;
            break;
        }
        case SAKURA_DIV: {
            printf("    %lld\t(%lld)\t\tDIV\t\t%d, %d, %d\n", idx, i, assembler->instructions[i + 1],
                   assembler->instructions[i + 2], assembler->instructions[i + 3]);
            i += 3;
            break;
        }
        case SAKURA_POW: {
            printf("    %lld\t(%lld)\t\tPOW\t\t%d, %d, %d\n", idx, i, assembler->instructions[i + 1],
                   assembler->instructions[i + 2], assembler->instructions[i + 3]);
            i += 3;
            break;
        }
        case SAKURA_MOD: {
            printf("    %lld\t(%lld)\t\tMOD\t\t%d, %d, %d\n", idx, i, assembler->instructions[i + 1],
                   assembler->instructions[i + 2], assembler->instructions[i + 3]);
            i += 3;
            break;
        }
        case SAKURA_EQ: {
            printf("    %lld\t(%lld)\t\tEQ\t\t%d, %d, %d\n", idx, i, assembler->instructions[i + 1],
                   assembler->instructions[i + 2], assembler->instructions[i + 3]);
            i += 3;
            break;
        }
        case SAKURA_LT: {
            printf("    %lld\t(%lld)\t\tLT\t\t%d, %d, %d\n", idx, i, assembler->instructions[i + 1],
                   assembler->instructions[i + 2], assembler->instructions[i + 3]);
            i += 3;
            break;
        }
        case SAKURA_LE: {
            printf("    %lld\t(%lld)\t\tLE\t\t%d, %d, %d\n", idx, i, assembler->instructions[i + 1],
                   assembler->instructions[i + 2], assembler->instructions[i + 3]);
            i += 3;
            break;
        }
        case SAKURA_CLOSURE: {
            printf("    %lld\t(%lld)\t\tCLOSURE\t\t%d, %d\t\t;; store fn-%d into stack pos %d\n", idx, i,
                   assembler->instructions[i + 1], assembler->instructions[i + 2], assembler->instructions[i + 2],
                   assembler->instructions[i + 1]);
            i += 2;
            break;
        }
        case SAKURA_CALL: {
            struct s_str *key = cachedGlobals[assembler->instructions[i + 1]];
            if (key == NULL)
                key = &basicCall;
            printf("    %lld\t(%lld)\t\tCALL\t\t%d, %d\t\t;; %.*s(%d args...)\n", idx, i,
                   assembler->instructions[i + 1], assembler->instructions[i + 2], key->len, key->str,
                   assembler->instructions[i + 2]);
            cachedGlobals[assembler->instructions[i + 1]] = NULL;
            i += 2;
            break;
        }
        case SAKURA_GETGLOBAL: {
            struct s_str *key = &S->globals.pairs[assembler->instructions[i + 2]].key;
            cachedGlobals[assembler->instructions[i + 1]] = key;
            printf("    %lld\t(%lld)\t\tGETGLOBAL\t%d\t\t;; store '%.*s' into stack pos %d\n", idx, i,
                   assembler->instructions[i + 2], key->len, key->str, assembler->instructions[i + 1]);
            i += 2;
            break;
        }
        case SAKURA_SETGLOBAL: {
            allocVal = sakuraX_readTVal(&assembler->pool.constants[-assembler->instructions[i + 2] - 1]);
            printf("    %lld\t(%lld)\t\tSETGLOBAL\t%d, %d\t\t;; sets '%s' from stack pos %d\n", idx, i,
                   assembler->instructions[i + 1], assembler->instructions[i + 2], allocVal,
                   assembler->instructions[i + 1]);
            free(allocVal);
            i += 2;
            break;
        }
        case SAKURA_JMP: {
            printf("    %lld\t(%lld)\t\tJMP\t\t%d\n", idx, i, assembler->instructions[i + 1]);
            i += 1;
            break;
        }
        case SAKURA_JMPIF: {
            printf("    %lld\t(%lld)\t\tJMPIF\t\t%d, %d\n", idx, i, assembler->instructions[i + 1],
                   assembler->instructions[i + 2]);
            i += 2;
            break;
        }
        case SAKURA_RETURN: {
            printf("    %lld\t(%lld)\t\tRETURN\t\t%d, %d\n", idx, i, assembler->instructions[i + 1],
                   assembler->instructions[i + 2]);
            i += 2;
            break;
        }
        case SAKURA_NOT: {
            printf("    %lld\t(%lld)\t\tNOT\t\t%d, %d\n", idx, i, assembler->instructions[i + 1],
                   assembler->instructions[i + 2]);
            i += 2;
            break;
        }
        case SAKURA_MOVE: {
            printf("    %lld\t(%lld)\t\tMOVE\t\t%d, %d\n", idx, i, assembler->instructions[i + 1],
                   assembler->instructions[i + 2]);
            i += 2;
            break;
        }
        case SAKURA_NEWTABLE: {
            printf("    %lld\t(%lld)\t\tNEWTABLE\t%d, %d, %d\n", idx, i, assembler->instructions[i + 1], 0,
                   assembler->instructions[i + 2]);
            i += 2;
            break;
        }
        case SAKURA_SETTABLE: {
            allocVal = sakuraX_readTVal(assembler->instructions[i + 2] < 0
                                            ? &assembler->pool.constants[-assembler->instructions[i + 2] - 1]
                                            : NULL);
            allocVal2 = sakuraX_readTVal(assembler->instructions[i + 3] < 0
                                             ? &assembler->pool.constants[-assembler->instructions[i + 3] - 1]
                                             : NULL);
            printf("    %lld\t(%lld)\t\tSETTABLE\t%d, %d, %d \t;; %s %s\n", idx, i, assembler->instructions[i + 1],
                   assembler->instructions[i + 2], assembler->instructions[i + 3], allocVal, allocVal2);
            i += 3;
            free(allocVal);
            free(allocVal2);
            break;
        }
        case SAKURA_GETTABLE: {
            allocVal = sakuraX_readTVal(assembler->instructions[i + 3] < 0
                                            ? &assembler->pool.constants[-assembler->instructions[i + 3] - 1]
                                            : NULL);
            printf("    %lld\t(%lld)\t\tGETTABLE\t%d, %d, %d \t;; tbl@%d[%s] into %d\n", idx, i,
                   assembler->instructions[i + 1], assembler->instructions[i + 2], assembler->instructions[i + 3],
                   assembler->instructions[i + 2], allocVal, assembler->instructions[i + 1]);
            i += 3;
            free(allocVal);
            break;
        }
        }

        idx++;
    }

    free(cachedGlobals);

    if (mode & 1 << 1) {
        printf("Raw Instructions:\n");
        for (size_t i = 0; i < assembler->size; i++)
            printf("%d ", assembler->instructions[i]);
        printf("\n");
    }

    if (mode & 1 << 2) {
        printf("Constants Dump (%p through %p):\n", assembler->pool.constants,
               assembler->pool.constants + assembler->pool.size);
        for (size_t i = 0; i < assembler->pool.size; i++) {
            allocVal = sakuraX_readTVal(&assembler->pool.constants[i]);
            printf("  [%lld] %s\n", i, allocVal);
            free(allocVal);
        }
    }

    for (size_t i = 0; i < assembler->closureIdx; i++) {
        trueFname = (char *)malloc(sizeof(char) * (strlen(filename) + 25));
        sprintf(trueFname, "%s:fn-%lld", filename, i);
        printf("\n");
        sakuraX_writeDisasm(S, assembler->closures[i], trueFname, mode | 1 << 8);
        free(trueFname);
    }

    if (!(mode & 1 << 8))
        printf("=================\n");

    s_str_free(&basicCall);
}