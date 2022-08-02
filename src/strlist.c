#include "strlist.h"

#include <stdio.h>
#include <assert.h>

#include <string.h>


// TODO: Maybe. Index with linked list to keep track of the order.
//            Problematic, with mathces
// TODO: Store the strings in a buffer.

// This might be nice
// struct StrNode {
//     int len;
//     char *data;
// }

int wrap_strcmp(char **a, char **b) {
    return strcmp(*a, *b);
}

StrList strlist_new(size_t cap) {
    StrList sl;
    sl.index = (char **) malloc(sizeof(char *)*cap);
    sl.bt_index = bitree_new((bitree_cmp_func) wrap_strcmp);
    sl.size = 0;
    sl.flags = STRLIST_UNIQUE;
    sl.count = 0;
    sl.capacity = cap;
    return sl;
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
void strlist_add(StrList *sl, char *str, int flags) {
    assert(sl->size < sl->capacity);

    BiNode *res;
    bool added = (char *) bitree_add(sl->bt_index, (void *) &str, &res);

    //  Uniqeness:
    // if (sl->flags & STRLIST_UNIQUE &&
    if(!added)
       return;

    // if (flags & STRLIST_COPY)
    {
        // TODO: Copy into a big buffer;
        char *buff = (char *) malloc(sizeof(char)*(strlen(str) + 1));
        strcpy(buff, str);
        str = buff;
    }

    // TODO: If not unique, res->data sould be a list.
    //  This way only the last added is remomered.
    res->data = sl->index + sl->size;
    sl->index[sl->size++] = str;
}

// To avoid restructuring the list many times,
//  This function could set the node to NULL / Or have an ignore flag.
//  Followed by calling a refactor to remove the NULL nodes.
void strlist_rm_idx(StrList *sl, int idx) {

    // TODO: Need to delete it from the BiTree

    while (++idx < sl->size)
        sl->index[idx-1] = sl->index[idx];
    sl->size--;
}

void strlist_free(StrList *sl) {
    while (sl->size-- > 0)
        free(sl->index[sl->size]);
    free(sl->index);
}

char **strlist_find(StrList *sl, const char *pattern) {
    BiNode *res;
    bool exits = (char *) bitree_find(sl->bt_index, (void *) &pattern, &res);

    if (exits)
        return res->data; // idx = (res->data - sl->index) / sizeof(char *);
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
