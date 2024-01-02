#include "sap.h"

#include <stdlib.h>

#include "assembler.h"
#include "disasm.h"
#include "filesystem.h"
#include "parser.h"
#include "svm.h"

int sakuraS_print(SakuraState *S) {
    int args = sakura_peek(S);
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
            printf("%.*s    ", val.len, val.str);
        } else {
            printf("[%p]    ", sakuraY_peek(S));
        }
    }

    printf("\n");
    return 0;
}

int sakuraS_loadstring(SakuraState *S) {
    int args = sakura_peek(S);
    sakuraY_pop(S);

    if (args != 1) {
        printf("Error: expected 1 argument, got %d\n", args);
        exit(1);
    }

    struct s_str source = sakura_popString(S);

    // parse the source
    struct TokenStack *tokens = sakuraY_analyze(S, &source);
    struct NodeStack *nodes = sakuraY_parse(S, tokens);
    sakuraX_freeTokStack(tokens);
    struct SakuraAssembly *assembly = sakuraY_assemble(S, nodes);
    sakuraX_freeNodeStack(nodes);

    // push return value
    sakuraY_push(S, sakuraY_makeTFunc(assembly));

    return 1;
}

int sakuraS_loadfile(SakuraState *S) {
    int args = sakura_popNumber(S);

    if (args != 1) {
        printf("Error: expected 1 argument, got %d\n", args);
        exit(1);
    }

    struct s_str file = sakura_popString(S);

    struct s_str source = readfile_s(&file);
    if (source.str == NULL) {
        printf("Error: could not read file '%.*s'\n", file.len, file.str);
        exit(1);
    }

    // parse the source
    struct TokenStack *tokens = sakuraY_analyze(S, &source);
    struct NodeStack *nodes = sakuraY_parse(S, tokens);
    sakuraX_freeTokStack(tokens);
    struct SakuraAssembly *assembly = sakuraY_assemble(S, nodes);
    sakuraX_freeNodeStack(nodes);

    // push return value
    sakuraY_push(S, sakuraY_makeTFunc(assembly));

    // cleanup
    s_str_free(&source);

    return 1;
}

int sakuraS_dofile(SakuraState *S) {
    int args = sakura_popNumber(S);

    if (args != 1) {
        printf("Error: expected 1 argument, got %d\n", args);
        exit(1);
    }

    struct s_str file = sakura_popString(S);

    struct s_str source = readfile_s(&file);
    if (source.str == NULL) {
        printf("Error: could not read file '%.*s'\n", file.len, file.str);
        exit(1);
    }

    // parse the source
    struct TokenStack *tokens = sakuraY_analyze(S, &source);
    struct NodeStack *nodes = sakuraY_parse(S, tokens);
    sakuraX_freeTokStack(tokens);
    struct SakuraAssembly *assembly = sakuraY_assemble(S, nodes);
    sakuraX_freeNodeStack(nodes);

    size_t originalOffset = S->internalOffset;
    S->internalOffset = S->stackIndex;
    int vals = sakuraX_interpret(S, assembly);
    S->internalOffset = originalOffset;

    // cleanup
    s_str_free(&source);
    sakuraX_freeAssembly(assembly);

    return vals;
}

void sakuraL_loadStdlib(SakuraState *S) {
    sakura_register(S, "print", sakuraS_print);
    sakura_register(S, "loadstring", sakuraS_loadstring);
    sakura_register(S, "loadfile", sakuraS_loadfile);
    sakura_register(S, "dofile", sakuraS_dofile);
}

void sakuraL_loadfile(SakuraState *S, const char *file, int showDisasm) {
    struct s_str source = readfile(file);
    if (source.str == NULL) {
        printf("Error: could not read file %s\n", file);
        return;
    }

    sakuraL_loadstring(S, &source, showDisasm);

    s_str_free(&source);
}

void sakuraL_loadstring(SakuraState *S, struct s_str *source, int showDisasm) {
    sakuraL_loadStdlib(S);

    struct TokenStack *tokens = sakuraY_analyze(S, source);
    struct NodeStack *nodes = sakuraY_parse(S, tokens);
    sakuraX_freeTokStack(tokens); // may need to move this to after assembly
    struct SakuraAssembly *assembly = sakuraY_assemble(S, nodes);
    sakuraX_freeNodeStack(nodes);

    if (showDisasm >= 1)
        sakuraX_writeDisasm(S, assembly, "test.sa", showDisasm);
    sakuraX_interpret(S, assembly);

    sakuraX_freeAssembly(assembly);
}

void sakuraL_loadstring_c(SakuraState *S, const char *str, int showDisasm) {
    struct s_str source = s_str(str);
    sakuraL_loadstring(S, &source, showDisasm);
    s_str_free(&source);
}

void sakuraL_registerGlobalFn(SakuraState *S, const char *name, int (*fnPtr)(SakuraState *)) {
    struct s_str nameStr = s_str(name);
    TValue val = sakuraY_makeTCFunc(fnPtr);
    sakuraY_push(S, val);
    sakura_setGlobal(S, &nameStr);
    s_str_free(&nameStr);
}
