#ifndef MAIN_H__
#define MAIN_H__

#include <stdbool.h>
#include <stdio.h>
#include "color.h"

typedef struct options {
  char *prgname;

  bool ms;

  bool ignore_case;
  char *prompt;

  FILE *pv;

  Color nb, nf, sb, sf;
} options;

void usage(FILE *sink, const char *prgname);
int main(int args, char **argv);

#endif
