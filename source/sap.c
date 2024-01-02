#include "sap.h"

#include <stdlib.h>

#include "assembler.h"
#include "disasm.h"
#include "filesystem.h"
#include "parser.h"
#include "sstd.h"
#include "svm.h"

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
    struct TokenStack *tokens;
    struct NodeStack *nodes;
    struct SakuraAssembly *assembly;

    sakuraL_loadStdlib(S);

    tokens = sakuraY_analyze(S, source);
    nodes = sakuraY_parse(S, tokens);
    sakuraX_freeTokStack(tokens); // may need to move this to after assembly
    assembly = sakuraY_assemble(S, nodes);
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
