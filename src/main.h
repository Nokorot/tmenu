#ifndef MAIN_H__
#define MAIN_H__

#include <stdbool.h>
#include <stdio.h>

typedef struct options {
  char *prgname;

  bool ignore_case;
  char *prompt;
} options;

void usage(FILE *sink, const char *prgname);
int main(int args, char **argv);

#endif
