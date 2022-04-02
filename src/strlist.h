#ifndef STRLIST_H_
#define STRLIST_H_

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    char **index;
    size_t size;
    size_t capacity;
} StrList;

StrList strlist_new(size_t cap);
void strlist_add(StrList *slst, const char *str);
void strlist_rm_idx(StrList *slist, int idx);

void strlist_del(StrList *slst);

char **strlist_find(StrList *slist, const char *str);
bool strlist_rm(StrList *slist, const char *str);

#endif
