#include "strlist.h"

#include <stdio.h>
#include <assert.h>

StrList strlist_new(size_t cap) {
    StrList slst;
    slst.index = (char **) malloc(sizeof(char *)*cap);
    slst.size = 0;
    slst.capacity = cap;
    return slst;
}

// TODO: This data structure is terible
void strlist_add(StrList *slst, const char *str) {
    // for(int i=0; i<slst->size; i++) {
    //     if (strcmp(slst->index[i], str) == 0)
    //         return;
    // }
    // 

    // assert(slst.size < slst.capacity);
    size_t len = strlen(str);
    char *buff = (char *) malloc(sizeof(char)*(len+1));
    strcpy(buff, str);
    slst->index[slst->size++] = buff;
}

void strlist_rm_idx(StrList *sl, int idx) {
    while (++idx < sl->size)
        sl->index[idx-1] = sl->index[idx];
    sl->size--;
}

void strlist_del(StrList *slst) {
    while (slst->size-- > 0)
        free(slst->index[slst->size]);
    free(slst->index);
}

char **strlist_find(StrList *sl, const char *str) {
  char **idx = sl->index, **end = idx + sl->size;
  for  (;idx < end; ++idx)
      if (strcmp(*idx, str) == 0)
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
