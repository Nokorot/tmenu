#include "main.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

#include "minimal.h"
#include "tmenu.h"

#include "color.h"

#define FLAG_IMPLEMENTATION
#include "flag.h"

void usage(FILE *sink, const char *prgname)
{
    fprintf(sink, "Usage: %s [Options] [--] <input> [output]\n", prgname);
    fprintf(sink, "Options:\n");
    flag_print_options(sink);
}

char *shift_args(int *argc, char ***argv)
{
    assert(*argc > 0);
    char *result = **argv;
    *argv += 1;
    *argc -= 1;
    return result;
}

void color_arg(const char *name, char *value, Color *dst, Color dft){
  if (!value) {
      *dst = dft; return;
  }
  if (color24_from_hex(value, dst) < 0) {
      fprintf(stderr, "ERROR: Color argument '%s' most be of the form '#RRGGBB' \
              or '#RGB' (where R, G and B are hex digits), but got instead '%s'\n",
              name, value);
      exit(1);
  }
}

int main(int argc, char **argv) {
  tmenu tm;
  tm.op.prgname = *argv;

  bool *help      = flag_bool("help",        'h', "Print this help to stdout and exit with 0");
  bool *ign_case  = flag_bool("ignore-case", 'i', "Ignore case");
  bool *json      = flag_bool("json",        'j', "Read input file as a jsonfile");

  bool *ms        = flag_bool("multi-select", 'm', "Multi-select, outputs selected item without exiting, when <Return> is pressed.");

  char **prompt   = flag_str("prompt",  NULL, "Prompt to be displayed");
  flag_set_variant((void *) prompt, "p");
  char **preview       = flag_str("preview", NULL, "Preview, output current selection");
  flag_set_variant((void *) preview, "pv");

  char *key_separator = flag_char("key-separator", ' ', "Set the character that separates the different parts of the search key");
  flag_set_variant(key_separator, "sep");

  char **nb_color = flag_str("nb", NULL, "defines the normal background color.");
  char **nf_color = flag_str("nf", NULL, "defines the normal foreground color.");
  char **sb_color = flag_str("sb", NULL, "defines the selected background color.");
  char **sf_color = flag_str("sf", NULL, "defines the selected foreground color.");

  if (!flag_parse(argc, argv)) { flag_print_error(stderr); exit(1); }
  if (*help) { usage(stdout, tm.op.prgname); exit(0); }
  argv = flag_rest_argv();
  argc = flag_rest_argc();

  if (!(*argv)) {
    fprintf(stderr, "%s\n", "ERROR: Not enough arguments! (see -h)");
    exit(1);
  }

  tm.op.ignore_case = *ign_case;
  tm.op.prompt = *prompt;
  tm.op.key_separator[0] = *key_separator;

  tm.op.ms = *ms;

  color_arg("nb", *nb_color, &tm.op.nb, color4(false, 9)); // Default color
  color_arg("nf", *nf_color, &tm.op.nf, color4(false, 9));
  color_arg("sb", *sb_color, &tm.op.sb, color4(false, 3)); // Yellow
  color_arg("sf", *sf_color, &tm.op.sf, color4(false, 0)); // Black

  // Initialize tmenu_data
  char key[MAX_KEY_LEN]; *key = 0;
  tm.key = key;
  tm.key_len = 0;

  tm.items_sz = 0;
  tm.items_ln = 0;
  tm.items = NULL;

  if (*json) {
    read_json(&tm, *(argv++));
  } else
    read_input(&tm, *(argv++));
  // tm.lines = read_input(*(argv++));

  tm.matches = (int*) malloc(sizeof(int)*tm.items_ln);
  // tm.matches = (int*) malloc(sizeof(int)*tm.lines.size);

  tm.matches_cap = tm.items_ln;
  // tm.matches_cap = tm.lines.size;
  tm.sel = 0;
  tm.cur = 1;

  tm.json = NULL;
  tm.jsondepth = -1;

  tm.out = stdout;
  if (*argv) {
    char *out_fn = *(argv++);
    tm.out = fopen(out_fn, "w");
    if (tm.out == NULL) {
      fprintf(stderr, "Could not open output file '%s'\n", out_fn);
      exit(EXIT_FAILURE);
    }
  }

#ifdef DEBUG_LOG
  tm.debug_log = fopen(DEBUG_LOG, "w");
  if (tm.debug_log == NULL) {
    fprintf(stderr, "Could not debug_log file '%s'\n", DEBUG_LOG);
    exit(EXIT_FAILURE);
  }
#endif

  tm.op.pv = 0;
  if (*preview) {
    tm.op.pv = fopen(*preview, "w");
    if (tm.op.pv == NULL) {
      fprintf(stderr, "Could not open preview file '%s'\n", *preview);
      exit(EXIT_FAILURE);
    }
  }

  // TODO: react to window resize
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

  int ret_val = main_loop(&tm);
  fclose(tm.out);

#ifdef DEBUG_LOG
  fclose(tm.debug_log);
#endif

  return ret_val;
}
