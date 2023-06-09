#include "color.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Expects 0 <= n <= 7
Color color4(_Bool bright, unsigned char n) {
  if (n != 9 && (n < 0 || n > 7)) {
    fprintf(stderr, "ERROR: color4 ecspects a color in the range 0, ..., 7, but got %d\n", n);
    exit(1);
  }

  Color cl;
  cl.type = COLOR_4BIT;
  cl.data.as4bit = (Color4Data) { bright, n };
  return cl;
}

Color color8(unsigned char n) {
  Color cl;
  cl.type = COLOR_8BIT;
  cl.data.as8bit = (Color8Data) {n};
  return cl;
}

Color color24(unsigned char r, unsigned char g, unsigned b) {
  Color cl;
  cl.type = COLOR_24BIT;
  cl.data.as24bit =  (Color24Data) { r, g, b };
  return cl;
}


int color24_from_hex(char *hex, Color *cl) {
  int n = strlen(hex);
  if (*hex != '#' || (n != 4 && n != 7)) {
    // fprintf(stderr, "color24_from_hex expects a string of the form '#RRGGBB' or '#RGB' (where R, G and B are hex digits), but got instead '%s'\n", hex);
    return -1;
  }

  for (char *c=hex; (('0' <= *c && *c <= '9') || ('a' <= *c && *c <= 'f')) && *(c++);)
  if (*c) return -2;

  unsigned int r,g,b;
  if (n == 4) {
    sscanf(hex, "#%1x%1x%1x", &r, &g, &b);
    *cl = color24(r << 4,g << 4,b << 4);
  } else {
    sscanf(hex, "#%2x%2x%2x", &r, &g, &b);
    *cl = color24(r,g,b);
  }
  return 0;
}

int getEscCode(Color cl, int mode, char *buff) {
  switch (cl.type) {
    case COLOR_4BIT:
#define AS4 cl.data.as4bit
      if (mode == FOREGROUND_MODE)
        return sprintf(buff, "\x1b[%dm", AS4.n + (AS4.bright ? 90 : 30));
      else if (mode == BACKGROUND_MODE)
        return sprintf(buff, "\x1b[%dm", AS4.n + (AS4.bright ? 100 : 40));
      break;
#undef AS4
    case COLOR_8BIT:
#define AS8 cl.data.as8bit
      if (mode == FOREGROUND_MODE)
        return sprintf(buff, "\x1b[38;5;%dm", AS8.n);
      else if (mode == BACKGROUND_MODE)
        return sprintf(buff, "\x1b[48;5;%dm", AS8.n);
      break;
#undef AS8
    case COLOR_24BIT:
#define AS24 cl.data.as24bit
      if (mode == FOREGROUND_MODE)
        return sprintf(buff, "\x1b[38;2;%d;%d;%dm", AS24.r, AS24.g, AS24.b);
      else if (mode == BACKGROUND_MODE)
        return sprintf(buff, "\x1b[48;2;%d;%d;%dm", AS24.r, AS24.g, AS24.b);
      break;
#undef AS24
  }
  return '\0';
}



