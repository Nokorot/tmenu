#ifndef STRLIST_H_
#define STRLIST_H_

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


// #define STRLIST_UNIQUE 0
#define STRLIST_UNIQUE 1
#define STRLIST_COPY   2

typedef struct {
    // int flags; // Uniqe entries?

    char **index;
    size_t size; // Includes empty entries
    size_t count; // Does not includes empty entries
    size_t capacity;
} StrList;

// Creates a new empty StrList
StrList strlist_new(size_t cap);

// Appends 'str at the end of the list
// // Flags:
//    Uniqe=1, Copy=2
void strlist_add(StrList *slst, char *str, int flags);


// Searches the list for entries matching the 'pattern
// The index of the strings matching the 'pattern is stored in 'matches
int strlist_match(const StrList *slist, const char *pattern, int *matches);

// TODO: -1 means at the end
// void strlist_insert(StrList *slst, const char *str, int index, int flags);

// Removes the string at the index 'idx.
//  The order is preserved
void strlist_rm_idx(StrList *slist, int idx);

// Frees all the strings and the index.
void strlist_free(StrList *slst);

// Note: Only the first. Even if there are more.
// Searches the list for the first entry whos string equals (ie. strcmp) 'pattern
char **strlist_find(StrList *slist, const char *pattern);

// Removes the first entry whos string equals (ie. strcmp) 'pattern
bool strlist_rm(StrList *slist, const char *str);

#endif
