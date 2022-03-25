#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "minimal.h"

#include <ctype.h>
#include "strlist.h"

#define INPUT_CAP 1024

// TODO: Bette data structure

typedef struct tmenu_data {
  FILE *out;

  char *prgname;
  StrList lines;
  char *key;
  int key_len;
} tmenu;

StrList read_input(char *inpt) {
  StrList lines = strlist_new(INPUT_CAP);

  FILE *fin;
  fin = fopen(inpt, "r");
  if (fin == NULL)
    exit(EXIT_FAILURE);

  // TODO: trim endline char

  char * line = NULL; size_t len = 0;
  while (getline(&line, &len, fin) != -1)
    strlist_add(&lines, line);
  return lines;
}


void list_matches(tmenu *tm) {
  for (int i=0; i < tm->lines.size; ++i) {
    if (strstr(tm->lines.index[i], tm->key))
      fprintf(stdout, "%s", tm->lines.index[i]);
  }
}

void usage(char *prgname, FILE *sink) {
  fprintf(sink, "Usage: %s <input> <output>\n\n", prgname);
}

#define MAX_KEY_LEN 1024

void draw_screen(tmenu *tm) {
  printf("%s", "\x1b[2J"); // Clear screen

  // TODO: remember corsor position. Might want to allow left and right movement
  printf("%s", "\x1b[2;1H");
  list_matches(tm);

  printf("%s", "\x1b[1;1H");
  printf("%s", tm->key);
}

void add_ch(tmenu *tm, char ch) {
  tm->key[tm->key_len++] = ch;
  tm->key[tm->key_len] = 0; 
  
  draw_screen(tm);
}

void del_ch(tmenu *tm) {
  tm->key[--tm->key_len] = 0;
  draw_screen(tm);
}

void push_result(FILE *sink, tmenu *tm) {
  for (int i=0; i < tm->lines.size; ++i) {
    if (strstr(tm->lines.index[i], tm->key)) {
      fprintf(sink, "%s", tm->lines.index[i]);
      return;
    }
  }
}

int main(int args, char **argv) {
  tmenu tm;
  tm.prgname = *(argv++); args--;

  if (args < 1) {
    fprintf(stderr, "%s\n", "ERROR: Not enough arguments!");
    usage(tm.prgname, stderr);
    exit(1);
  }

  char key[MAX_KEY_LEN]; *key = 0;
  tm.key = key;
  tm.key_len = 0;
  tm.lines = read_input(*(argv++)); args--;

  // TODO: If outour file
  // char *out_fn = *(argv++); args--;
  // tm.out = fopen(out_fn, "w"); 
  // if (tm.out == NULL) {
  //   fprintf(stderr, "Could not open output file '%s'\n", out_fn);
  //   exit(EXIT_FAILURE);
  // }

  printf("%s", "\x1B[\?1049h"); // TODO: move into teminal_init
  if (terminal_init()) {
      if (errno == ENOTTY)
          fprintf(stderr, "This program requires a terminal.\n");
      else
          fprintf(stderr, "Cannot initialize terminal: %s.\n", strerror(errno));
      return EXIT_FAILURE;
  }
  
  draw_screen(&tm);

  _Bool quit = 0;
  for (int c; !quit && (c = getc(stdin)) != EOF;) {
    if (isalpha(c) || isdigit(c)) 
        add_ch(&tm, c);

    switch (c) {
      case '\x1b': // Escape
        c = getc(stdin);
        if (c == '\x1b') { quit=1; continue; } // Quit on double escape
        else if(c == '[') { // Escape sequence
          switch(c = getc(stdin)) { // TODO: handle escape sequences properly
            case 'A':  // Arrow Up
            case 'B':  // Arrow Down
            case 'C':  // Arrow Right
            case 'D':  // Arrow Left
            default: continue;
          }
        } else if (isalpha(c) || isdigit(c)) {
          add_ch(&tm, c);
        }
      case '\x0d': // Return
        printf("%s", "\x1B[\?1049l");
        push_result(stdout, &tm);
        return EXIT_SUCCESS;
      case '\x7f': // backspace
        del_ch(&tm);
    }
  }

  printf("%s", "\x1B[\?1049l");
  return EXIT_SUCCESS;
}

