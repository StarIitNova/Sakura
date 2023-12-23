#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sakura.h"
#include "sap.h"

#ifndef SAKURA_VERSION
#define SAKURA_VERSION "UNKNOWN"
#endif

int main(int argc, const char **argv) {
    if (argc <= 1) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    const char *filename = 0;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (strcmp(argv[i], "-h") == 0) {
                printf("Usage: %s <file>\n", argv[0]);
                return 0;
            } else if (strcmp(argv[i], "-v") == 0) {
                printf("Sakura version %s\n", SAKURA_VERSION);
                return 1;
            }
        } else {
            filename = argv[i];
        }
    }

    if (!filename) {
        printf("Usage: %s <file>\n", argv[0]);
        printf("Error: no file specified\n");
        return 1;
    }

    SakuraState *S = sakura_createState();

    sakuraL_loadfile(S, filename);

    sakura_destroyState(S);
    return 0;
}