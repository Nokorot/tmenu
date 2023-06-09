#ifndef MAIN_H__
#define MAIN_H__

#include <stdbool.h>
#include <stdio.h>
#include "color.h"

// Here there should be a comment
typedef struct options {
  char *prgname;

  bool ms;

  bool ignore_case;
  char *prompt;

  FILE *pv;

  char key_separate;

  Color nb, nf, sb, sf;
} options;

void usage(FILE *sink, const char *prgname);
int main(int args, char **argv);

#endif
