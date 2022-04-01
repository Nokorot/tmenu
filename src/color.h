#ifndef COLOR_H__
#define COLOR_H__

#include <stdbool.h>

typedef enum {
    COLOR_4BIT = 0,
    COLOR_8BIT,
    COLOR_24BIT,
} ColorType;

typedef struct {
    _Bool bright;
    unsigned char n;
} Color4Data;

typedef struct {
    unsigned char n;
} Color8Data;

typedef struct {
    unsigned char r, g, b;
} Color24Data;

typedef union {
    Color4Data  as4bit;
    Color8Data  as8bit;
    Color24Data as24bit;
} ColorData;

typedef struct {
  ColorType type;
  ColorData data;
} Color;



Color color4(_Bool bright, unsigned char n);
Color color8(unsigned char n);
Color color24(unsigned char r, unsigned char g, unsigned b);


int color24_from_hex(char *hex, Color *cl);

#define FOREGROUND_MODE 0
#define BACKGROUND_MODE 1
int getEscCode(Color cl, int mode, char *buff);
#endif
