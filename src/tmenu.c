#include "tmenu.h"

#include "minimal.h"
#include <ctype.h>
#include <string.h>

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

void strlwr(char *str) {
  for(; *str; ++str)
    *str = tolower(*str);
}

void list_matches(tmenu *tm) {
  char *last;

  // TODO: This shuld be done somewhere else.
  int off;
  char sel_esc[40], nrm_esc[40];
  off = getEscCode(tm->op.sf, FOREGROUND_MODE, sel_esc);
  getEscCode(tm->op.sb, BACKGROUND_MODE, sel_esc+off);
  off = getEscCode(tm->op.nf, FOREGROUND_MODE, nrm_esc);
  getEscCode(tm->op.nb, BACKGROUND_MODE, nrm_esc+off);

  char key[128], tmp[128];

  strcpy(key, tm->key);
  if (tm->op.ignore_case) strlwr(key);


  // TODO: The plus 2 is the input line and a padding at the end.
  //            This should be abrtracted
  int n = 0; // Line count
  for (int i=0; i < tm->lines.size && n+2 < tm->out_rows; ++i) {
    strcpy(tmp, tm->lines.index[i]);
    if (tm->op.ignore_case) strlwr(tmp);

    if (strstr(tmp, key)) {
      fprintf(stdout, "%s", (n == tm->sel) ? sel_esc : nrm_esc);
      fprintf(stdout, "%.*s\n", tm->out_cols, tm->lines.index[i]);
      last = tm->lines.index[i];
      ++n;
    }
  }

  // TODO: FIX: This is a hack
  if (n <= tm->sel && n > 0) {
    tm->sel = n-1;

    fprintf(stdout, "\x1b[A");
    fprintf(stdout, "%s%.*s", sel_esc, tm->out_cols, last);
  }

  // Reset at the end
  fprintf(stdout, "\x1b[0m");
}

void draw_screen(tmenu *tm) {
  printf("%s", "\x1b[2J"); // Clear screen

  // TODO: remember corsor position. Might want to allow left and right movement
  printf("%s", "\x1b[2;1H");
  list_matches(tm);

  printf("%s", "\x1b[1;1H");
  if (tm->op.prompt)
    printf("%s ", tm->op.prompt);
  printf("%s", tm->key);
}

void add_ch(tmenu *tm, char ch) {
  tm->key[tm->key_len++] = ch;
  tm->key[tm->key_len] = 0; 
  
  draw_screen(tm);
}

void del_ch(tmenu *tm) {
  if (tm->key_len > 0) {
    tm->key[--tm->key_len] = 0;
    draw_screen(tm);
  }
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

int main_loop(tmenu *tm) {
  _Bool quit = 0;
  for (int c; !quit && (c = getc(stdin)) != EOF;) {
    if (isalpha(c) || isdigit(c))
        add_ch(tm, c);
  
    switch (c) {
      case '\x1b': // Escape
        c = getc(stdin);
        if (c == '\x1b') return 0; // Quit on double escape
        else if(c == '[') { // Escape sequence
          switch(c = getc(stdin)) { // TODO: handle escape sequences properly
            case 'A':  // Arrow Up
                tm->sel--; if (tm->sel < 0) tm->sel = 0; 
                draw_screen(tm); continue; 
            case 'B':  // Arrow Down
                tm->sel++; if (tm->sel >= tm->out_rows-2 ) tm->sel = tm->out_rows-2; 
                draw_screen(tm); continue;
            case 'C':  // Arrow Right
            case 'D':  // Arrow Left
            default: continue;
          }
        } else if (isalpha(c) || isdigit(c)) {
          add_ch(tm, c);
        }
      case '\x0d': // Return
        printf("%s", "\x1B[\?1049l");
        if (tm->out) {
            push_result(tm->out, tm);
            fclose(tm->out);
        } else {
            push_result(stdout, tm);
        }
        return 0;
      case '\x7f': // backspace
        del_ch(tm);
    }
  }
  return 0;
}



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
