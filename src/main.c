#include "main.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>

#include "minimal.h"
#include "tmenu.h"

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

int main(int argc, char **argv) {
  options op;
  op.prgname = *argv;

  bool *help      = flag_bool("help", false, "Print this help to stdout and exit with 0");
  bool *ign_case  = flag_bool("i", false, "Ignore case");
  char **prompt   = flag_str("p", NULL, "Prompt to be displayed");

  if (!flag_parse(argc, argv)) { usage(stderr, op.prgname); flag_print_error(stderr); exit(1); }
  if (*help) { usage(stdout, op.prgname); exit(0); }
  argv = flag_rest_argv();
  argc = flag_rest_argc();

  if (!(*argv)) {
    fprintf(stderr, "%s\n\n", "ERROR: Not enough arguments!");
    usage(stderr, op.prgname);
    exit(1);
  }

  op.ignore_case = *ign_case;
  op.prompt = *prompt;

  tmenu tm;
  tm.op = op;
  char key[MAX_KEY_LEN]; *key = 0;
  tm.key = key;
  tm.key_len = 0;
  tm.lines = read_input(*(argv++));

  tm.sel = 0;
  // tm.sel_bg = 47; // White
  tm.sel_bg = 43; //Yellow

  tm.out = 0;
  if (*argv) {
    char *out_fn = *(argv++);
    tm.out = fopen(out_fn, "w"); 
    if (tm.out == NULL) {
      fprintf(stderr, "Could not open output file '%s'\n", out_fn);
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
  
  draw_screen(&tm);
  return main_loop(&tm);
}

