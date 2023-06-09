#ifndef TMENU_H__
#define TMENU_H__

#include <stdio.h>

#include "main.h"
#include <jansson.h>

// TODO: Trim line to this length ?
#define MAX_LINE_LENGTH 1024
#define MAX_KEY_LEN BUFSIZ
#define INPUT_CAP 1024*1024

typedef struct item {
    char *key;
    bool selected;
    json_t *json;

    // struct item *next_match;
    // // This will make it posible to preserve the order of adding
    // struct item *next_selected;
} item;

typedef struct tmenu_data {
    options op;
    FILE *out;
    // TODO: Resize
    int out_rows, out_cols;

    item *items;
    size_t items_sz;
    size_t items_ln;

    // cursor
    int cur;
    size_t sel;
    // item *sel;

    int *matches;
    int matches_count;
    int matches_cap;
    // item *matches, matches_end;

    json_t *json;
    int jsondepth;

#ifdef DEBUG_LOG
    FILE *debug_log;

#endif

    char *key;
    int key_len;
} tmenu;

#ifdef DEBUG_LOG
    #define dlog(tm, format, ...) \
        do { \
            fprintf(tm->debug_log, format, ##__VA_ARGS__); \
            fflush(tm->debug_log); \
        } while(0)
#else
    #define dlog(tm, format, ...) \
        do { \
        } while(0)
#endif


item *itemnew(tmenu *tm);
void read_input(tmenu *tm, char *inpt);
void read_json(tmenu *tm, const char *path);

void listjson(tmenu *tm, json_t *obj);


void draw_screen(tmenu *tm);

void list_matches(tmenu *tm);
void add_ch(tmenu *tm, char ch);
void del_ch(tmenu *tm, int index);

void set_sel(tmenu *tm, int index);

int main_loop(tmenu *tm);

// StrList read_input(char *inpt);
void push_result(tmenu *tm);

#endif
