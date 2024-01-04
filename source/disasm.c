#include "disasm.h"

#include <stdlib.h>

void sakuraX_writeDisasm(SakuraState *S, struct SakuraAssembly *assembler, const char *filename, int mode) {
    struct s_str **cachedGlobals;
    struct s_str basicCall;
    size_t idx = 1;
    char *allocVal, *allocVal2, *trueFname;

    LOG_CALL();

    S->currentState = SAKURA_FLAG_DISASSEMBLING;

    sakura_printf("\x1b[36m%s \x1b[35m<%s>\x1b[0m (\x1b[33m%lld\x1b[0m instructions, \x1b[33m%lld\x1b[0m bytes)\n",
                  mode & 1 << 8 ? "function" : "main", filename, assembler->size, assembler->size * sizeof(int));
    sakura_printf("\x1b[33m%lld\x1b[0m registers, \x1b[33m%lld\x1b[0m closures, \x1b[33m%lld\x1b[0m constants, "
                  "\x1b[33m%lld\x1b[0m functions\n",
                  assembler->highestRegister, assembler->closureIdx, assembler->pool.size, assembler->functionsLoaded);

    basicCall = s_str("loaded_function");
    cachedGlobals = (struct s_str **)malloc(sizeof(struct s_str *) * (assembler->functionsLoaded * 2));

    for (size_t i = 0; i < assembler->size; i++) {
        switch (assembler->instructions[i]) {
        case SAKURA_LOADK: {
            allocVal = sakuraX_readTValC(&assembler->pool.constants[-assembler->instructions[i + 2] - 1]);
            sakura_printf("    \x1b[1;32m%lld\x1b[0m\t(%lld)\t\tLOADK\t\t%d\t\t\x1b[30m;;\x1b[0m %s into stack pos "
                          "\x1b[33m%d\x1b[0m\n",
                          idx, i, assembler->instructions[i + 2], allocVal, assembler->instructions[i + 1]);
            free(allocVal);
            i += 2;
            break;
        }
        case SAKURA_ADD: {
            sakura_printf("    \x1b[1;32m%lld\x1b[0m\t(%lld)\t\tADD\t\t%d, %d, %d\n", idx, i,
                          assembler->instructions[i + 1], assembler->instructions[i + 2],
                          assembler->instructions[i + 3]);
            i += 3;
            break;
        }
        case SAKURA_SUB: {
            sakura_printf("    \x1b[1;32m%lld\x1b[0m\t(%lld)\t\tSUB\t\t%d, %d, %d\n", idx, i,
                          assembler->instructions[i + 1], assembler->instructions[i + 2],
                          assembler->instructions[i + 3]);
            i += 3;
            break;
        }
        case SAKURA_MUL: {
            sakura_printf("    \x1b[1;32m%lld\x1b[0m\t(%lld)\t\tMUL\t\t%d, %d, %d\n", idx, i,
                          assembler->instructions[i + 1], assembler->instructions[i + 2],
                          assembler->instructions[i + 3]);
            i += 3;
            break;
        }
        case SAKURA_DIV: {
            sakura_printf("    \x1b[1;32m%lld\x1b[0m\t(%lld)\t\tDIV\t\t%d, %d, %d\n", idx, i,
                          assembler->instructions[i + 1], assembler->instructions[i + 2],
                          assembler->instructions[i + 3]);
            i += 3;
            break;
        }
        case SAKURA_POW: {
            sakura_printf("    \x1b[1;32m%lld\x1b[0m\t(%lld)\t\tPOW\t\t%d, %d, %d\n", idx, i,
                          assembler->instructions[i + 1], assembler->instructions[i + 2],
                          assembler->instructions[i + 3]);
            i += 3;
            break;
        }
        case SAKURA_MOD: {
            sakura_printf("    \x1b[1;32m%lld\x1b[0m\t(%lld)\t\tMOD\t\t%d, %d, %d\n", idx, i,
                          assembler->instructions[i + 1], assembler->instructions[i + 2],
                          assembler->instructions[i + 3]);
            i += 3;
            break;
        }
        case SAKURA_EQ: {
            sakura_printf("    \x1b[1;32m%lld\x1b[0m\t(%lld)\t\tEQ\t\t%d, %d, %d\n", idx, i,
                          assembler->instructions[i + 1], assembler->instructions[i + 2],
                          assembler->instructions[i + 3]);
            i += 3;
            break;
        }
        case SAKURA_LT: {
            sakura_printf("    \x1b[1;32m%lld\x1b[0m\t(%lld)\t\tLT\t\t%d, %d, %d\n", idx, i,
                          assembler->instructions[i + 1], assembler->instructions[i + 2],
                          assembler->instructions[i + 3]);
            i += 3;
            break;
        }
        case SAKURA_LE: {
            sakura_printf("    \x1b[1;32m%lld\x1b[0m\t(%lld)\t\tLE\t\t%d, %d, %d\n", idx, i,
                          assembler->instructions[i + 1], assembler->instructions[i + 2],
                          assembler->instructions[i + 3]);
            i += 3;
            break;
        }
        case SAKURA_CLOSURE: {
            sakura_printf("    \x1b[1;32m%lld\x1b[0m\t(%lld)\t\tCLOSURE\t\t%d, %d\t\t\x1b[30m;;\x1b[0m store fn-%d "
                          "into stack pos "
                          "\x1b[33m%d\x1b[0m\n",
                          idx, i, assembler->instructions[i + 1], assembler->instructions[i + 2],
                          assembler->instructions[i + 2], assembler->instructions[i + 1]);
            i += 2;
            break;
        }
        case SAKURA_CALL: {
            struct s_str *key = NULL;
            if (cachedGlobals + assembler->instructions[i + 1] >= cachedGlobals)
                key = cachedGlobals[assembler->instructions[i + 1]];
            if (key == NULL)
                key = &basicCall;
            sakura_printf(
                "    \x1b[1;32m%lld\x1b[0m\t(%lld)\t\tCALL\t\t%d, %d\t\t\x1b[30m;;\x1b[0m \x1b[34m%.*s(\x1b[0m%d "
                "args...\x1b[34m)\x1b[0m\n",
                idx, i, assembler->instructions[i + 1], assembler->instructions[i + 2], key->len, key->str,
                assembler->instructions[i + 2]);
            cachedGlobals[assembler->instructions[i + 1]] = NULL;
            i += 2;
            break;
        }
        case SAKURA_GETGLOBAL: {
            struct s_str *key = &S->globals.pairs[assembler->instructions[i + 2]].key;
            cachedGlobals[assembler->instructions[i + 1]] = key;
            sakura_printf("    \x1b[1;32m%lld\x1b[0m\t(%lld)\t\tGETGLOBAL\t%d\t\t\x1b[30m;;\x1b[0m store "
                          "\x1b[32m'%.*s'\x1b[0m into stack pos "
                          "\x1b[33m%d\x1b[0m\n",
                          idx, i, assembler->instructions[i + 2], key->len, key->str, assembler->instructions[i + 1]);
            i += 2;
            break;
        }
        case SAKURA_SETGLOBAL: {
            allocVal = sakuraX_readTValC(&assembler->pool.constants[-assembler->instructions[i + 2] - 1]);
            sakura_printf(
                "    \x1b[1;32m%lld\x1b[0m\t(%lld)\t\tSETGLOBAL\t%d, %d\t\t\x1b[30m;;\x1b[0m sets '%s' from stack pos "
                "\x1b[33m%d\x1b[0m\n",
                idx, i, assembler->instructions[i + 1], assembler->instructions[i + 2], allocVal,
                assembler->instructions[i + 1]);
            free(allocVal);
            i += 2;
            break;
        }
        case SAKURA_JMP: {
            sakura_printf("    \x1b[1;32m%lld\x1b[0m\t(%lld)\t\tJMP\t\t%d\n", idx, i, assembler->instructions[i + 1]);
            i += 1;
            break;
        }
        case SAKURA_JMPIF: {
            sakura_printf("    \x1b[1;32m%lld\x1b[0m\t(%lld)\t\tJMPIF\t\t%d, %d\n", idx, i,
                          assembler->instructions[i + 1], assembler->instructions[i + 2]);
            i += 2;
            break;
        }
        case SAKURA_RETURN: {
            sakura_printf("    \x1b[1;32m%lld\x1b[0m\t(%lld)\t\tRETURN\t\t%d, %d\n", idx, i,
                          assembler->instructions[i + 1], assembler->instructions[i + 2]);
            i += 2;
            break;
        }
        case SAKURA_NOT: {
            sakura_printf("    \x1b[1;32m%lld\x1b[0m\t(%lld)\t\tNOT\t\t%d, %d\n", idx, i,
                          assembler->instructions[i + 1], assembler->instructions[i + 2]);
            i += 2;
            break;
        }
        case SAKURA_MOVE: {
            sakura_printf("    \x1b[1;32m%lld\x1b[0m\t(%lld)\t\tMOVE\t\t%d, %d\n", idx, i,
                          assembler->instructions[i + 1], assembler->instructions[i + 2]);
            i += 2;
            break;
        }
        case SAKURA_NEWTABLE: {
            sakura_printf("    \x1b[1;32m%lld\x1b[0m\t(%lld)\t\tNEWTABLE\t%d, %d, %d\n", idx, i,
                          assembler->instructions[i + 1], 0, assembler->instructions[i + 2]);
            i += 2;
            break;
        }
        case SAKURA_SETTABLE: {
            allocVal = sakuraX_readTValC(assembler->instructions[i + 2] < 0
                                             ? &assembler->pool.constants[-assembler->instructions[i + 2] - 1]
                                             : NULL);
            allocVal2 = sakuraX_readTValC(assembler->instructions[i + 3] < 0
                                              ? &assembler->pool.constants[-assembler->instructions[i + 3] - 1]
                                              : NULL);
            sakura_printf("    \x1b[1;32m%lld\x1b[0m\t(%lld)\t\tSETTABLE\t%d, %d, %d \t\x1b[30m;;\x1b[0m %s %s\n", idx,
                          i, assembler->instructions[i + 1], assembler->instructions[i + 2],
                          assembler->instructions[i + 3], allocVal, allocVal2);
            i += 3;
            free(allocVal);
            free(allocVal2);
            break;
        }
        case SAKURA_GETTABLE: {
            allocVal = sakuraX_readTValC(assembler->instructions[i + 3] < 0
                                             ? &assembler->pool.constants[-assembler->instructions[i + 3] - 1]
                                             : NULL);
            sakura_printf(
                "    \x1b[1;32m%lld\x1b[0m\t(%lld)\t\tGETTABLE\t%d, %d, %d \t\x1b[30m;;\x1b[0m tbl@%d[%s] into "
                "\x1b[33m%d\x1b[0m\n",
                idx, i, assembler->instructions[i + 1], assembler->instructions[i + 2], assembler->instructions[i + 3],
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
        sakura_printf("Constants Dump (\x1b[33m%p\x1b[0m through \x1b[33m%p\x1b[0m):\n", assembler->pool.constants,
                      assembler->pool.constants + assembler->pool.size);
        for (size_t i = 0; i < assembler->pool.size; i++) {
            allocVal = sakuraX_readTValC(&assembler->pool.constants[i]);
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
        sakura_printf("\x1b[30m=============================================================================\x1b[0m\n");

    s_str_free(&basicCall);

    LOG_POP();
}