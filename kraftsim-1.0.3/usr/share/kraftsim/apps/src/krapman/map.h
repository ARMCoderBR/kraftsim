#ifndef __MAP_H__
#define __MAP_H__

#include <stdint.h>

#define MAZECOLOR	0x01
#define DOORCOLOR	0x08
#define MAPXOFFSET	46
#define MAZEWIDTH	224
#define MAZEHEIGHT	240

#define DOTXOFFSET 11
#define DOTYOFFSET 7

#define DOTPWXOFFSET 8
#define DOTPWYOFFSET 4


#include "game.h"

void draw_mapwalls();
void fill_dots(gamestate_t *g);
void draw_dots(gamestate_t *g);
uint8_t get_dot(char *dots,int rowNum,int colNum);
void clr_dot(char *dots,int rowNum,int colNum);

#endif

