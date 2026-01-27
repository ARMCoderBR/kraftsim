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
} vga_t;

vga_t *vga_init(SDL_Renderer* renderer);

void vga_out(uint8_t value);

void vga_close(vga_t *vga);

#endif /* VGA_H_ */
