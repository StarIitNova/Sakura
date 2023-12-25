#include "sap.h"

#include "assembler.h"
#include "disasm.h"
#include "filesystem.h"
#include "parser.h"
#include "svm.h"

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
    struct TokenStack *tokens = sakuraY_analyze(S, source);
    struct NodeStack *nodes = sakuraY_parse(S, tokens);
    struct SakuraAssembly *assembly = sakuraY_assemble(S, nodes);

    // sakuraX_writeDisasm(assembly, "test.sa");
    sakuraX_interpret(S, assembly);

    for (size_t i = 0; i < S->stackIndex; i++) {
        TValue val = S->stack[i];
        switch (val.tt) {
        case SAKURA_TNUMFLT:
            printf("%f\n", val.value.n);
            break;
        case SAKURA_TSTR:
            printf("%.*s\n", val.value.s.len, val.value.s.str);
            break;
        }
    }

    sakuraX_freeAssembly(assembly);
    sakuraX_freeNodeStack(nodes);
    sakuraX_freeTokStack(tokens);
}

void sakuraL_loadstring_c(SakuraState *S, const char *str) {
    struct s_str source = s_str(str);
    sakuraL_loadstring(S, &source);
    s_str_free(&source);
}