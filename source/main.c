#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sstr.h"

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

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        printf("Failed to open file: %s\n", argv[1]);
        return 1;
    }

    struct s_str s = S_NULL_STR;
    char temp[1024];
    while (fgets(temp, 1024, file)) {
        struct s_str ns = s_str_concat_c(&s, temp);
        free(s.str);
        s = ns;
    }

    printf("%.*s", s.len, s.str);

    free(s.str);
    return 0;
}