#include "filesystem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct s_str readfile(const char *path) {
    char temp[1024];
    struct s_str s, ns;
    FILE *file = fopen(path, "r");

    if (!file) {
        return SI_NULL_STR;
    }

    s = (struct s_str)S_NULL_STR;
    while (fgets(temp, 1024, file)) {
        ns = s_str_concat_c(&s, temp);
        free(s.str);
        s = ns;
    }

    fclose(file);
    return s;
}

struct s_str readfile_s(const struct s_str *path) {
    struct s_str s;
    char *path_c = (char *)malloc(path->len + 1);
    memcpy(path_c, path->str, path->len);
    path_c[path->len] = '\0';
    s = readfile(path_c);
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