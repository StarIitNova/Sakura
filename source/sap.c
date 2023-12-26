#include "sap.h"

#include "assembler.h"
#include "disasm.h"
#include "filesystem.h"
#include "parser.h"
#include "svm.h"

int sakuraS_print(SakuraState *S) {
    int args = sakura_peak(S);
    sakuraY_pop(S);

    for (int i = 0; i < args; i++) {
        if (sakura_isNumber(S)) {
            char output[50];
            sprintf(output, "%f", sakura_popNumber(S));

            size_t len = strlen(output);
            for (size_t i = len - 1; i >= 0; i--) {
                if (output[i] == '0')
                    output[i] = '\0';
                else
                    break;
            }

            len = strlen(output);
            if (output[len - 1] == '.')
                output[len - 1] = '\0';

            printf("%s    ", output);
        } else if (sakura_isString(S)) {
            struct s_str val = sakura_popString(S);
            printf("%.*s    ", val.str, val.len);
        } else {
            printf("?    ");
        }
    }

    printf("\n");
    return 0;
}

void sakuraL_loadStdlib(SakuraState *S) { sakura_register(S, "print", sakuraS_print); }

void sakuraL_loadfile(SakuraState *S, const char *file) {
    struct s_str source = readfile(file);
    if (source.str == NULL) {
        printf("Error: could not read file %s\n", file);
        return;
    }

    sakuraL_loadstring(S, &source);

    s_str_free(&source);
}

void sakuraL_loadstring(SakuraState *S, struct s_str *source) {
    sakuraL_loadStdlib(S);

    struct TokenStack *tokens = sakuraY_analyze(S, source);
    struct NodeStack *nodes = sakuraY_parse(S, tokens);
    struct SakuraAssembly *assembly = sakuraY_assemble(S, nodes);

    // sakuraX_writeDisasm(S, assembly, "test.sa");
    sakuraX_interpret(S, assembly);

    sakuraX_freeAssembly(assembly);
    sakuraX_freeNodeStack(nodes);
    sakuraX_freeTokStack(tokens);
}

void sakuraL_loadstring_c(SakuraState *S, const char *str) {
    struct s_str source = s_str(str);
    sakuraL_loadstring(S, &source);
    s_str_free(&source);
}

void sakuraL_registerGlobalFn(SakuraState *S, const char *name, int (*fnPtr)(SakuraState *)) {
    struct s_str nameStr = s_str(name);
    TValue val = sakuraY_makeTCFunc(fnPtr);
    sakuraY_push(S, val);
    sakura_setGlobal(S, &nameStr);
    s_str_free(&nameStr);
}