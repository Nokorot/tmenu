#include "tmenu.h"

#include "minimal.h"
#include <ctype.h>
#include <string.h>
#include <utils.h>

StrList read_input(char *inpt) {
  StrList lines = strlist_new(INPUT_CAP);

  FILE *fin;
  fin = fopen(inpt, "r");
  if (fin == NULL)
    exit(EXIT_FAILURE);

  char * line = NULL; size_t len = 0;
  while (getline(&line, &len, fin) != -1) {
    str_rtrim(line);
    strlist_add(&lines, line, 2);
  }

  return lines;
}

void strlwr(char *str) {
  for(; *str; ++str)
    *str = tolower(*str);
}

char *cur_sel(tmenu *tm) {
  int idx = tm->matches[tm->sel];
  return tm->lines.index[idx];
}

void list_matches(tmenu *tm) {

  // TODO: This shuld be done somewhere else.
  int off;
  char sel_esc[40], nrm_esc[40];
  off = getEscCode(tm->op.sf, FOREGROUND_MODE, sel_esc);
  getEscCode(tm->op.sb, BACKGROUND_MODE, sel_esc+off);
  off = getEscCode(tm->op.nf, FOREGROUND_MODE, nrm_esc);
  getEscCode(tm->op.nb, BACKGROUND_MODE, nrm_esc+off);

  // TODO: Option
  int rows = tm->out_rows - 2;
  int page = tm->sel / rows;

  int i=page*rows , *idx = tm->matches+i;

  for (; i < tm->matches_count && i < (page+1)*rows; ++i, ++idx) {
    fprintf(stdout, "%s", (i == tm->sel) ? sel_esc : nrm_esc);

    // Note: in stead of a list of strings, we should have a list of structs,
    // with a flag, indicating wheter it is selected
    if (strlist_find(&tm->results, tm->lines.index[*idx])) {
      fprintf(stdout, " * %.*s\n", tm->out_cols, tm->lines.index[*idx]);
    } else {
      fprintf(stdout, "%.*s\n", tm->out_cols, tm->lines.index[*idx]);
    }
  }

  // Reset at the end
  fprintf(stdout, "\x1b[0m");
}

void draw_screen(tmenu *tm) {
  // if (!tm->dirty)
  //     return;

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

  tm->dirty = false;
}

void update_matches(tmenu *tm) {
  tm->matches_count = strlist_match(&tm->lines, tm->key, tm->matches, \
                (tm->op.ignore_case) ? STRLIST_IGNORE_CASE : 0);
  if (tm->sel > tm->matches_count)
      tm->sel = tm->matches_count;
  tm->dirty = true;
}


void add_ch(tmenu *tm, char ch) {
  tm->key[tm->key_len++] = ch;
  tm->key[tm->key_len] = 0;

  tm->cur++;
  update_matches(tm);
}

void del_ch(tmenu *tm, int index) {
  if (index < 0)
      index = tm->cur;

  // The first case sould imply the second, if not someting is wrong
  if (index > 0 && index < tm->key_len+2 && tm->key_len > 0) {
    for (int i=index; ++i < tm->key_len;)
        tm->key[i-1] = tm->key[i];
    tm->key[--tm->key_len] = 0;

    update_matches(tm);
  }
}

void push_result(tmenu *tm) {
  char *sel = cur_sel(tm);

  if (!strlist_rm(&tm->results, sel)) {
    strlist_add(&tm->results, sel, 2);
    // TODO: This does not make sense when you can deselet.
    //          There shuld be a flag, for doing this. And then the line sould disapere
    // if (tm->out)
    //   fprintf(tm->out, "%s\n", sel);
  }
}

void set_sel(tmenu *tm, int index) {
    if (index < 0 || index > tm->matches_count-1)
        tm->sel = tm->matches_count-1;
    else
        tm->sel = index;

    tm->dirty = true;
}

void move_sel(tmenu *tm, int amount) {
    tm->sel += amount;

    if (tm->sel < 0) tm->sel = 0;
    // else if (tm->sel > tm->out_rows-2 )
    else if (tm->sel > tm->matches_count-1 )
        tm->sel = tm->matches_count-1;

    tm->dirty = true;
}


void move_cur(tmenu *tm, int amount) {
    tm->cur += amount;

    if (tm->cur < 1) tm->cur = 1;
    else if (tm->cur > tm->key_len+1)
        tm->cur = tm->key_len+1;

    tm->dirty = true;
}

int main_loop(tmenu *tm) {
  update_matches(tm);
  draw_screen(tm);

  _Bool quit = 0;
  for (int c; !quit && (c = getc(stdin)) != EOF; draw_screen(tm)) {
    switch (c) {
      case '\x1b': // Escape
        c = getc(stdin);
        if (c == '\x1b') return 0; // Quit on double escape
        else if(c == '[') { // Escape sequence
          c = getc(stdin);

          int keycode = 1;
          if (isdigit(c)) {
              keycode = c-'0';
              c = getc(stdin);
              while (isdigit(c)) {
                keycode = keycode*10 + c-'0';
                c = getc(stdin);
              }
          }

          int modifyer = 1;
          if (c == ';') {
              modifyer = 0;
              while (isdigit(c = getc(stdin))) {
                modifyer = modifyer*10 + c-'0';
              }
          }

          switch(c) { // TODO: handle escape sequences properly
            case 'A':  // Arrow Up
                move_sel(tm, -1); continue;
            case 'B':  // Arrow Down
                move_sel(tm,  1); continue;
            case 'C': // Arrow Right
                move_cur(tm,  1); continue;
            case 'D':  // Arrow Left
                move_cur(tm, -1); continue;
            case 'F': // End
                set_sel(tm, -1); continue;
            case 'H': // Home
                set_sel(tm, 0); continue;
            case 10: // Alt + Retrun
                // Might want to use this for someting else.
                if (tm->op.ms) {
                  push_result(tm);
                  move_sel(tm, +1);
                }
                continue;
            case '~':
                int rows = tm->out_rows - 2;
                // TODO: page function;
                switch (keycode) {
                    case 3: // delete
                        del_ch(tm, tm->cur+1);  continue;
                    case 5: // page up
                        move_sel(tm, -rows);    continue;
                    case 6: // page down
                        move_sel(tm, rows);     continue;
                    default: // Not suported
                        printf("%d", keycode);  continue;
                }
            default:
                continue;
          }
        } else if (isalpha(c) || isdigit(c)) {
           // NOTE: This is alt+<key>
           //  Would be nice to use for custum keybindings

          add_ch(tm, c);
        }
      case ' ': // Space
        if (tm->op.ms) {
          push_result(tm);
          move_sel(tm, +1);
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
        del_ch(tm, tm->cur);
        move_cur(tm, -1);
        continue;
    }

    if (isprint(c)) {
        add_ch(tm, c);
        continue;
    }

  }
  return 0;
}
