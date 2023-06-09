#include "tmenu.h"

#include "minimal.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "strutil.h"

#include <jansson.h>

item *itemnew(tmenu *tm) {
	if (tm->items_ln + 1 >= (tm->items_sz / sizeof(item))) {
		if (!(tm->items = realloc(tm->items, (tm->items_sz += BUFSIZ))))
			die("cannot realloc %u bytes:", tm->items_sz);
  }

  memset(&tm->items[tm->items_ln], 0, sizeof(item));
	return &tm->items[tm->items_ln++];
}

void read_input(tmenu *tm, char *inpt) {
  FILE *fin;
  fin = fopen(inpt, "r");
  if (fin == NULL)
    exit(EXIT_FAILURE);

  item *item;
  char * line = NULL; size_t len = 0;
  while (getline(&line, &len, fin) != -1) {
    item = itemnew(tm);
    str_rtrim(line);
    if (!(item->key = strdup(line)))
        die("cannot strdup %u bytes:", strlen(line) + 1);
		// item->key = line;
  }
}

void read_json(tmenu *tm, const char *path)
{
	json_error_t jerr;

	if (!(tm->json = json_load_file(path, 0, &jerr))) {
		die("%s @ line: %i - %s", jerr.text, jerr.line, path);
  }

  listjson(tm, tm->json);
}

void listjson(tmenu *tm, json_t *obj)
{
	void *iter;
	struct item *item;

	// tm->items_ln = 0;
	iter = json_object_iter(obj);
	while (iter) {
		item = itemnew(tm);
		item->key = (char*) json_object_iter_key(iter);
		item->json = json_object_iter_value(iter);
		iter = json_object_iter_next(obj, iter);
	}
}

void strlwr(char *str) {
  for(; *str; ++str)
    *str = tolower(*str);
}

item *cur_sel(tmenu *tm) {
  return tm->items + tm->matches[tm->sel];
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

  item *item;

  for (; i < tm->matches_count && i < (page+1)*rows; ++i, ++idx) {
    item = tm->items + *idx;

    fprintf(stdout, "%s", (i == tm->sel) ? sel_esc : nrm_esc);
    // fprintf(stdout, "%s", (item == tm->sel) ? sel_esc : nrm_esc);

    // Note: in stead of a list of strings, we should have a list of structs,
    // with a flag, indicating wheter it is selected
    // if (strlist_find(&tm->results, tm->lines.index[*idx])) {
    if (item->selected) {
      fprintf(stdout, " * %.*s\n", tm->out_cols, item->key);
    } else {
      fprintf(stdout, "%.*s\n", tm->out_cols, item->key);
    }
  }

  // Reset at the end
  fprintf(stdout, "\x1b[0m");
}

// TODO: dirty parameter
void draw_screen(tmenu *tm) {
  if (tm->op.pv) {
    fprintf(tm->op.pv, "%s\n", cur_sel(tm)->key);
    fflush(tm->op.pv);
  }

  printf("%s", "\x1b[2J"); // Clear screen

  // TODO: remember cursor position. Might want to allow left and right movement
  printf("%s", "\x1b[2;1H");
  list_matches(tm);

  printf("%s", "\x1b[1;1H");
  if (tm->op.prompt)
    printf("%s ", tm->op.prompt);
  printf("%s", tm->key);
  printf("\x1b[1;%dH", tm->cur);
}

void update_matches(tmenu *tm) {
  int j=0, i=0;
  item *item = tm->items;
  char *(*_strstr)(const char *, const char *);
  _strstr = tm->op.ignore_case ? &strcasestr : strstr;

  static char **tokv;
	static int tokn = 0;
  int tokc = 0;

	char *buf = malloc(sizeof(char) * tm->key_len), *s;
	strcpy(buf, tm->key);

	/* separate input text into tokens to be matched individually */
	for (s = strtok(buf, " "); s; tokv[tokc - 1] = s, s = strtok(NULL, " ")) {
		if (++tokc > tokn && !(tokv = realloc(tokv, ++tokn * sizeof *tokv)))
			die("cannot realloc %u bytes:", tokn * sizeof *tokv);
  }

  for (; i < tm->items_ln; ++i, ++item) {
    int k = 0;
		for (; k < tokc; k++) {
			if (!_strstr(item->key, tokv[k])) 
				break;
    }
    if (k != tokc) /* not all tokens match */
			continue;

    tm->matches[j++] = i;
  }
  free(buf);

  tm->matches_count = j;
}

void add_ch(tmenu *tm, char ch) {
  tm->key[tm->key_len++] = ch;
  tm->key[tm->key_len] = 0;

  tm->cur++;
  update_matches(tm);
  set_sel(tm, 0);
  draw_screen(tm);
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
        set_sel(tm, 0);
        draw_screen(tm);
    }
}

void toggle_select(tmenu *tm) {
    item *item = tm->items + tm->matches[tm->sel];
    item->selected = !item->selected;
}

bool dump_json_value(FILE *sink, json_t *json) {
    switch json_typeof(json){
      case JSON_OBJECT:
      case JSON_ARRAY:
          fprintf(sink, "%s\n", json_dumps(json, 0)); return true;
      case JSON_STRING:
          fprintf(sink, "%s\n", json_string_value(json)); return true;
      case JSON_INTEGER:
          fprintf(sink, "%lld\n", json_integer_value(json)); return true;
      case JSON_REAL:
          fprintf(sink, "%f\n", json_real_value(json)); return true;
      case JSON_TRUE:
          fprintf(sink, "true\n"); return true;
      case JSON_FALSE:
          fprintf(sink, "false\n"); return true;
      default:
          return NULL;
    }
}

void write_results(tmenu *tm) {
    item *item = tm->items;
    for (int idx = 0; idx < tm->items_ln; ++idx, ++item) {
        if (item->key && item->selected) {
            if (!item->json || !dump_json_value(tm->out, item->json))
                fprintf(tm->out, "%s\n", item->key);
        }
    }
    fflush(tm->out);
}

void tmenu_close(tmenu *tm) {
    printf("%s", "\x1B[\?1049l");
}

void set_sel(tmenu *tm, int index) {
    if (index == 0)
        tm->sel = index;
    else if (index < 0 || index > tm->matches_count-1)
        tm->sel = tm->matches_count-1;
    else
        tm->sel = index;

    draw_screen(tm);
}

void move_sel(tmenu *tm, int amount) {
    tm->sel += amount;

    if (tm->sel < 0) tm->sel = 0;
    // else if (tm->sel > tm->out_rows-2 )
    else if (tm->sel > tm->matches_count-1 )
        tm->sel = tm->matches_count-1;

    draw_screen(tm);
}


void move_cur(tmenu *tm, int amount) {
    tm->cur += amount;

    if (tm->cur < 1) tm->cur = 1;
    else if (tm->cur > tm->key_len+1)
        tm->cur = tm->key_len+1;

    draw_screen(tm);
}


int main_loop(tmenu *tm) {
  update_matches(tm);
  draw_screen(tm);

  _Bool quit = 0;
  for (int c; !quit && (c = getc(stdin)) != EOF;) {

    switch (c) {
      case '\x1b': // Escape
        c = getc(stdin);
        if (c == '\x1b') { // Quit on double escape
            tmenu_close(tm);
            return 0; 
        }
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
          // fprintf(tm->out, "%d", modifyer);
        
          int rows;
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
                // toggle_select(tm);
                tmenu_close(tm);
                write_results(tm);
                return 0;
            case '~':
                rows = tm->out_rows - 2;
                // TODO: page function;
                switch (keycode) {
                    case 3: // delete
                        // move_cur(tm,  1);
                        del_ch(tm, tm->cur+1);
                        continue;
                    case 5: // page up
                        // Move one page up
                        move_sel(tm, -rows); continue;
                    case 6: // page down
                        // Movie one page down
                        move_sel(tm, rows); continue;
                    default:
                        printf("%d", keycode);
                        // Not suported
                        continue;
                }
            default:
                continue;
          }
        } else if (c == 13) { // Alt+Return
          // toggle_select(tm);
          write_results(tm);
          return 0;
        }
        else if (isalpha(c) || isdigit(c)) {
           // NOTE: This is alt+<key>
           //  Would be nice to use for custum keybindings
          add_ch(tm, c);
        }
        continue;
      case ' ': // Space
        add_ch(tm, ' ');
        continue;
      case 'J': // Shift+j
      case 'j' & 037: // Ctrl+j
        move_sel(tm, +1); continue;
      case 'k' & 037: // Ctrl+k
        move_sel(tm, -1); continue;
      case '\x0d': // Return
        toggle_select(tm);
        move_sel(tm, +1);
        if (tm->op.ms)
            continue;
        tmenu_close(tm);
        write_results(tm);
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

