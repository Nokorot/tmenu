#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "minimal.h"

#include <sys/ioctl.h>
#include <unistd.h>

#include <ctype.h>
#include "strlist.h"

#define INPUT_CAP 1024

// TODO: Bette data structure
typedef struct tmenu_data {
  FILE *out;

  int out_rows, out_cols;
  int sel, sel_bg; // White

  char *prgname;
  StrList lines;
  char *key;
  int key_len;
} tmenu;

_Bool str_contains(char *str, char c) {
  for (int ch = *str; (ch = *(str++));)
    if (ch == c) return 1;
  return 0;
}

void str_rtrim(char *str) {
  char *mx = str;
  for (int ch = *str; (ch = *(str++));)
    if (!str_contains(" \n\r\t", ch))
        mx = str;
  *mx = '\0';
}

StrList read_input(char *inpt) {
  StrList lines = strlist_new(INPUT_CAP);

  FILE *fin;
  fin = fopen(inpt, "r");
  if (fin == NULL)
    exit(EXIT_FAILURE);

  // TODO: trim endline char
  char * line = NULL; size_t len = 0;
  while (getline(&line, &len, fin) != -1) {
    str_rtrim(line);
    strlist_add(&lines, line);
  }
  return lines;
}

void list_matches(tmenu *tm) {
  char buff[20];
  sprintf(buff, "%s.%ds\n", "%", tm->out_cols);
  // TODO: The plus 2 is the input line and a padding at the end. 
  //            This should be abrtracted
  int n = 0; // Line count
  for (int i=0; i < tm->lines.size && n+2 < tm->out_rows; ++i) { 
    if (strstr(tm->lines.index[i], tm->key)) {
      if (n == tm->sel) fprintf(stdout, "\x1b[%dm", tm->sel_bg);
      fprintf(stdout, buff, tm->lines.index[i]);
      if (n == tm->sel) fprintf(stdout, "\x1b[0m");
      ++n;
    }
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
  int n = 0;
  for (int i=0; i < tm->lines.size; ++i) {
    if (strstr(tm->lines.index[i], tm->key)) {
      if (n++ >= tm->sel) {
        fprintf(sink, "%s\n", tm->lines.index[i]);
        return;
      }
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

  tm.sel = 0;
  tm.sel_bg = 47; // White

  // TODO: If outour file
 tm.out = 0;
  if (args > 0) {
    char *out_fn = *(argv++); args--;
    tm.out = fopen(out_fn, "w"); 
    if (tm.out == NULL) {
      fprintf(stderr, "Could not open output file '%s'\n", out_fn);
      exit(EXIT_FAILURE);
    }
  } 


  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

  tm.out_rows = w.ws_row;
  tm.out_cols = w.ws_col;

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
                tm.sel--; if (tm.sel < 0) tm.sel = 0; 
                draw_screen(&tm); continue; 
            case 'B':  // Arrow Down
                tm.sel++; if (tm.sel >= tm.out_rows-2 ) tm.sel = tm.out_rows-2; 
                draw_screen(&tm); continue;
            case 'C':  // Arrow Right
            case 'D':  // Arrow Left
            default: continue;
          }
        } else if (isalpha(c) || isdigit(c)) {
          add_ch(&tm, c);
        }
      case '\x0d': // Return
        printf("%s", "\x1B[\?1049l");
        if (tm.out) {
            push_result(tm.out, &tm);
            fclose(tm.out);
        } else {
            push_result(stdout, &tm);
        }
        return EXIT_SUCCESS;
      case '\x7f': // backspace
        del_ch(&tm);
    }
  }

  return EXIT_SUCCESS;
}

