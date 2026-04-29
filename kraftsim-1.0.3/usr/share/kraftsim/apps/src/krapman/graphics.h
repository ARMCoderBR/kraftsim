#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include <stdint.h>

#include "game.h"

#define PORTDATA 0x50
#define PORTADDRL 0x51
#define PORTADDRH 0x52

#define BYTES_PER_LINE 160

void plotasm (int x, int y, char color) __naked __sdcccall(0);

extern const uint8_t inky1up[];
extern const uint8_t inky1down[];
extern const uint8_t inky1left[];
extern const uint8_t inky1right[];

void draw_actor(actor_t *act);

void save_actor_bg(actor_t *act);
void restore_actor_bg(actor_t *act);

void restore_sprite_col0(int scrpos, char *sprite)  __naked __sdcccall(1);
void roll_spleft(char *sprite)  __naked __sdcccall(1);
void read_sprite_col7(int scrpos, char *sprite)  __naked __sdcccall(1);

void restore_sprite_col7(int scrpos, char *sprite)  __naked __sdcccall(1);
void roll_spright(char *sprite)  __naked __sdcccall(1);
void read_sprite_col0(int scrpos, char *sprite)  __naked __sdcccall(1);

void restore_sprite_row0(int scrpos, char *sprite)  __naked __sdcccall(1);
void roll_spup(char *sprite)  __naked __sdcccall(1);
void read_sprite_row15(int scrpos, char *sprite)  __naked __sdcccall(1);

void restore_sprite_row15(int scrpos, char *sprite)  __naked __sdcccall(1);
void roll_spdown(char *sprite)  __naked __sdcccall(1);
void read_sprite_row0(int scrpos, char *sprite)  __naked __sdcccall(1);

void check_sprite_superpos_col7(gamestate_t *g, actor_t *act);
void check_sprite_superpos_col0(gamestate_t *g, actor_t *act);
void check_sprite_superpos_row0(gamestate_t *g, actor_t *act);
void check_sprite_superpos_row15(gamestate_t *g, actor_t *act);

void draw_pwrup_on(gamestate_t *g,uint8_t row,uint8_t col);

void redraw_pwrup_on(gamestate_t *g);
void redraw_pwrup_off(gamestate_t *g);

#endif

