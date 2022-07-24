#include "strlist.h"

#include <stdio.h>
#include <assert.h>

#include <string.h>

// TODO: Make a BiTree index, in order to ensure uniqeness.
// TODO: Maybe. Index with linked list to keep track of the order.
//            Problematic, with mathces
// TODO: Store the strings in a buffer.

// This might be nice
// struct StrNode {
//     int len;
//     char *data;
// }

StrList strlist_new(size_t cap) {
    StrList slst;
    slst.index = (char **) malloc(sizeof(char *)*cap);
    slst.size = 0;
    slst.count = 0;
    slst.capacity = cap;
    return slst;
}

// The index of the strings matching the 'pattern is stored in 'matches
int strlist_match(const StrList *slist, const char *pattern, int *matches, int flags) {
    int j=0, i=0;
    char **str = slist->index;
    char *(*_strcmp)(const char *, const char *);
    _strcmp = (flags & STRLIST_IGNORE_CASE) ? &strcasestr : strstr;

    for (; i<slist->size; ++i, ++str) {
        // Checking if str is null. Thus allowing for empty entries
        if (str && _strcmp(*str, pattern))
            matches[j++] = i;
    }
    matches[j] = -1;
    return j;
}

// Flags:
//    Uniqe=1, Copy=2
void strlist_add(StrList *slst, char *str, int flags) {
    assert(slst->size < slst->capacity);

    //  Uniqeness:
    //  This is slow
    if (flags & STRLIST_UNIQUE) {
        for(int i=0; i<slst->size; i++) {
            if (strcmp(slst->index[i], str) == 0)
                return;
        }
    }

    if (flags & STRLIST_COPY) {
        // TODO: Copy into a big buffer;
        char *buff = (char *) malloc(sizeof(char)*(strlen(str) + 1));
        strcpy(buff, str);
        str = buff;
    }

    slst->index[slst->size++] = str;
}

// To avoid restructuring the list many times,
//  This function could set the node to NULL / Or have an ignore flag.
//  Followed by calling a refactor to remove the NULL nodes.
void strlist_rm_idx(StrList *sl, int idx) {
    while (++idx < sl->size)
        sl->index[idx-1] = sl->index[idx];
    sl->size--;
}

void strlist_free(StrList *slst) {
    while (slst->size-- > 0)
        free(slst->index[slst->size]);
    free(slst->index);
}

char **strlist_find(StrList *sl, const char *pattern) {
  char **idx = sl->index, **end = idx + sl->size;
  for  (;idx < end; ++idx)
      if (strcmp(*idx, pattern) == 0)
          return idx;
  return NULL;
}

bool strlist_rm(StrList *slist, const char *str) {
  char **idx = strlist_find(slist, str);
  if (idx) {
    strlist_rm_idx(slist, idx - slist->index);
    return true;
  }
  return false;
}
