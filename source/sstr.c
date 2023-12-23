#include "sstr.h"

#include <stdlib.h>
#include <string.h>

struct s_str s_str(const char *str) {
    struct s_str s;
    s.len = strlen(str);
    s.str = malloc(s.len);
    memcpy(s.str, str, s.len);
    return s;
}

struct s_str s_str_n(const char *str, unsigned int len) {
    struct s_str s;
    s.len = len;
    s.str = malloc(len);
    memcpy(s.str, str, s.len);
    return s;
}

struct s_str s_str_concat(const struct s_str *s1, const struct s_str *s2) {
    struct s_str s;
    s.len = s1->len + s2->len;
    s.str = malloc(s.len);
    memcpy(s.str, s1->str, s1->len);
    memcpy(s.str + s1->len, s2->str, s2->len);
    return s;
}

struct s_str s_str_concat_c(const struct s_str *s1, const char *s2) {
    struct s_str s;
    s.len = s1->len + strlen(s2);
    s.str = malloc(s.len);
    memcpy(s.str, s1->str, s1->len);
    memcpy(s.str + s1->len, s2, strlen(s2));
    return s;
}

struct s_str s_str_concat_s(const char *s1, const struct s_str *s2) {
    struct s_str s;
    s.len = strlen(s1) + s2->len;
    s.str = malloc(s.len);
    memcpy(s.str, s1, strlen(s1));
    memcpy(s.str + strlen(s1), s2->str, s2->len);
    return s;
}

struct s_str s_str_concat_cc(const char *s1, const char *s2) {
    struct s_str s;
    s.len = strlen(s1) + strlen(s2);
    s.str = malloc(s.len);
    memcpy(s.str, s1, strlen(s1));
    memcpy(s.str + strlen(s1), s2, strlen(s2));
    return s;
}

int s_str_cmp(const struct s_str *s1, const struct s_str *s2) {
    if (s1->len != s2->len)
        return s1->len - s2->len;
    return memcmp(s1->str, s2->str, s1->len);
}

int s_str_cmp_c(const struct s_str *s1, const char *s2) {
    int len = strlen(s2);
    if (s1->len != len)
        return s1->len - len;
    return memcmp(s1->str, s2, len);
}