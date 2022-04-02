#ifndef TMENU_H__
#define TMENU_H__

#include <stdio.h>

#include "main.h"
#include "strlist.h"

#define MAX_KEY_LEN 1024
#define INPUT_CAP 1024

// TODO: Bette data structure
typedef struct tmenu_data {
  options op;

  FILE *out;

  int out_rows, out_cols;
  int sel;

  StrList lines; // TODO: Store mathces
  StrList results;

  char *key;
  int key_len;
} tmenu;

void draw_screen(tmenu *tm);

void list_matches(tmenu *tm);
void add_ch(tmenu *tm, char ch);
void del_ch(tmenu *tm);

int main_loop();

StrList read_input(char *inpt);
void push_result(tmenu *tm);

// String utils
_Bool str_contains(char *str, char c);
void str_rtrim(char *str);
#endif
