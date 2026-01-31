/*
 * vga.h
 *
 *  Created on: 22 de jan. de 2026
 *      Author: milton
 */

#ifndef VGA_H_
#define VGA_H_

#include <stdint.h>

#include "sdlclient.h"

typedef struct {

    SDL_Texture *fontTexture[256];
    uint8_t *displayBuffer;
    SDL_Renderer* renderer;
    uint8_t mode;
    SDL_TimerID vgaTimer;
    int vgaTick;
    int rdaddr;
    int wraddr;
    int scrollreg;
    uint8_t cursor;
    int resetmode;
} vga_t;

vga_t *vga_init(SDL_Renderer* renderer);
void vga_refresh(vga_t *vga, int force);
void vga_out(vga_t *vga, uint8_t port, uint8_t value, int wminimized);
uint8_t vga_in(vga_t *vga, uint8_t port);
void vga_reset(vga_t *vga);
void vga_close(vga_t *vga);

#endif /* VGA_H_ */
