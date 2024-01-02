#include "filesystem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct s_str readfile(const char *path) {
    FILE *file = fopen(path, "r");
    if (!file) {
        return SI_NULL_STR;
    }

    struct s_str s = S_NULL_STR;
    char temp[1024];
    while (fgets(temp, 1024, file)) {
        struct s_str ns = s_str_concat_c(&s, temp);
        free(s.str);
        s = ns;
    }

    fclose(file);
    return s;
}

struct s_str readfile_s(const struct s_str *path) {
    char *path_c = malloc(path->len + 1);
    memcpy(path_c, path->str, path->len);
    path_c[path->len] = '\0';
    struct s_str s = readfile(path_c);
    free(path_c);
    return s;
}

int writefile(const char *path, const struct s_str *s) {
    FILE *file = fopen(path, "w");
    if (!file) {
        return 1;
    }

    fwrite(s->str, 1, s->len, file);
    fclose(file);
    return 0;
}

int writefile_c(const char *path, const char *s) {
    FILE *file = fopen(path, "w");
    if (!file) {
        return 1;
    }

    fwrite(s, 1, strlen(s), file);
    fclose(file);
    return 0;
}

int removefile(const char *path) { return remove(path); }