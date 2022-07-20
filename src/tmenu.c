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

char *cur_sel(tmenu *tm) {
  int n = 0;
  for (int i=0; i < tm->lines.size; ++i) {
    if (strstr(tm->lines.index[i], tm->key)) {
      if (n++ == tm->sel) {
        return tm->lines.index[i];
      }
    }
  }
  return NULL;
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

      if (strlist_find(&tm->results, tmp)) {
        fprintf(stdout, " * %.*s\n", tm->out_cols, tm->lines.index[i]);
      } else {
        fprintf(stdout, "%.*s\n", tm->out_cols, tm->lines.index[i]);
      }
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

// TODO: dirty paramenter

void draw_screen(tmenu *tm) {
  if (tm->op.pv) {
    fprintf(tm->op.pv, "%s\n", cur_sel(tm));
    fflush(tm->op.pv);
  }

  printf("%s", "\x1b[2J"); // Clear screen

  // TODO: remember corsor position. Might want to allow left and right movement
  printf("%s", "\x1b[2;1H");
  list_matches(tm);

  printf("%s", "\x1b[1;1H");
  if (tm->op.prompt)
    printf("%s ", tm->op.prompt);
  printf("%s", tm->key);
  printf("\x1b[1;%dH", tm->cur);
}

void add_ch(tmenu *tm, char ch) {
  tm->key[tm->key_len++] = ch;
  tm->key[tm->key_len] = 0;

  tm->cur++;
  draw_screen(tm);
}

void del_ch(tmenu *tm) {
  // The first case sould imply the second, if not someting is wrong
  if (tm->cur > 0 && tm->key_len > 0) {
    for (int i=tm->cur--; ++i < tm->key_len;)
        tm->key[i-1] = tm->key[i];
    tm->key[--tm->key_len] = 0;
    draw_screen(tm);
  }
}

void push_result(tmenu *tm) {
  char *sel = cur_sel(tm);

  if (!strlist_rm(&tm->results, sel)) {
    strlist_add(&tm->results, sel);
    // TODO: This does not make sense when you can deselet.
    //          There shuld be a flag, for doing this. And then the line sould disapere
    // if (tm->out)
    //   fprintf(tm->out, "%s\n", sel);
  }
}

void move_sel(tmenu *tm, int amount) {
    tm->sel += amount;

    if (tm->sel < 0)
        tm->sel = 0;
    else if (tm->sel > tm->out_rows-2 )
        tm->sel = tm->out_rows-2;

    draw_screen(tm);
}

void move_cur(tmenu *tm, int amount) {
    tm->cur += amount;

    if (tm->cur < 0)
        tm->cur = 0;
    else if (tm->cur > tm->key_len+1)
        tm->cur = tm->key_len+1;

    draw_screen(tm);
}

int main_loop(tmenu *tm) {
  _Bool quit = 0;
  for (int c; !quit && (c = getc(stdin)) != EOF;) {
    switch (c) {
      case '\x1b': // Escape
        c = getc(stdin);
        if (c == '\x1b') return 0; // Quit on double escape
        else if(c == '[') { // Escape sequence
          switch(c = getc(stdin)) { // TODO: handle escape sequences properly
            case 'A':  // Arrow Up
                move_sel(tm, -1); continue;
            case 'B':  // Arrow Down
                move_sel(tm,  1); continue;
            case 'C': // Arrow Right
                move_cur(tm,  1); continue;
            case 'D':  // Arrow Left
                move_cur(tm, -1); continue;
            default:
                continue;
          }
        } else if (isalpha(c) || isdigit(c)) {
          add_ch(tm, c);
        }
      case ' ': // Space
        // Shift space to write space
        if (tm->op.ms) {
          push_result(tm);
          move_sel(tm, +1);
          // NOTE: Not needed, since it is done by move_sel
          // draw_screen(tm);
        } else {
            add_ch(tm, ' ');
        }
        continue;
      case 'j' & 037: // Ctrl+j
        move_sel(tm, +1); continue;
      case 'k' & 037: // Ctrl+k
        move_sel(tm, -1); continue;
      case '\x0d': // Return
        if (!tm->op.ms)
          push_result(tm);

        printf("%s", "\x1B[\?1049l");
        char **str = tm->results.index, **end = str + tm->results.size;
        if (tm->out) {
          for (; str < end; ++str)
            fprintf(tm->out, "%s\n", *str);
          fclose(tm->out);
        } else {
          for (; str < end; ++str)
            printf("%s\n", *str);
        }
        return 0;
      case '\x7f': // backspace
        del_ch(tm);
        continue;
    }

    if (isprint(c)) {
        add_ch(tm, c);
        continue;
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
