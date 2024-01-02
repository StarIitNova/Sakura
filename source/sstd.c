#include "sstd.h"

#include <stdlib.h>
#include <string.h>

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
            size_t len;

            sprintf(output, "%f", sakura_popNumber(S));

            len = strlen(output);
            for (size_t z = len - 1; z != (size_t)-1; z--) {
                if (output[z] == '0')
                    output[z] = '\0';
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
    int args = (int)sakura_popNumber(S);
    struct s_str source;

    struct TokenStack *tokens;
    struct NodeStack *nodes;
    struct SakuraAssembly *assembly;

    if (args != 1) {
        printf("Error: expected 1 argument, got %d\n", args);
        exit(1);
    }

    source = sakura_popString(S);

    // parse the source
    tokens = sakuraY_analyze(S, &source);
    nodes = sakuraY_parse(S, tokens);
    sakuraX_freeTokStack(tokens);
    assembly = sakuraY_assemble(S, nodes);
    sakuraX_freeNodeStack(nodes);

    // push return value
    sakuraY_push(S, sakuraY_makeTFunc(assembly));

    return 1;
}

int sakuraS_loadfile(SakuraState *S) {
    int args = sakura_popNumber(S);
    struct s_str file, source;

    struct TokenStack *tokens;
    struct NodeStack *nodes;
    struct SakuraAssembly *assembly;

    if (args != 1) {
        printf("Error: expected 1 argument, got %d\n", args);
        exit(1);
    }

    file = sakura_popString(S);
    source = readfile_s(&file);
    if (source.str == NULL) {
        printf("Error: could not read file '%.*s'\n", file.len, file.str);
        exit(1);
    }

    // parse the source
    tokens = sakuraY_analyze(S, &source);
    nodes = sakuraY_parse(S, tokens);
    sakuraX_freeTokStack(tokens);
    assembly = sakuraY_assemble(S, nodes);
    sakuraX_freeNodeStack(nodes);

    // push return value
    sakuraY_push(S, sakuraY_makeTFunc(assembly));

    // cleanup
    s_str_free(&source);

    return 1;
}

int sakuraS_dofile(SakuraState *S) {
    int args = sakura_popNumber(S);
    struct s_str file, source;

    struct TokenStack *tokens;
    struct NodeStack *nodes;
    struct SakuraAssembly *assembly;

    size_t originalOffset;
    int retVals;

    if (args != 1) {
        printf("Error: expected 1 argument, got %d\n", args);
        exit(1);
    }

    file = sakura_popString(S);
    source = readfile_s(&file);
    if (source.str == NULL) {
        printf("Error: could not read file '%.*s'\n", file.len, file.str);
        exit(1);
    }

    // parse the source
    tokens = sakuraY_analyze(S, &source);
    nodes = sakuraY_parse(S, tokens);
    sakuraX_freeTokStack(tokens);
    assembly = sakuraY_assemble(S, nodes);
    sakuraX_freeNodeStack(nodes);

    originalOffset = S->internalOffset;
    S->internalOffset = S->stackIndex;
    retVals = sakuraX_interpret(S, assembly);
    S->internalOffset = originalOffset;

    // cleanup
    s_str_free(&source);
    sakuraX_freeAssembly(assembly);

    return retVals;
}