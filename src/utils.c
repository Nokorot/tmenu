
#include <utils.h>

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
