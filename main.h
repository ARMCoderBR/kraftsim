/*
 * act.h
 *
 *  Created on: 20 de jan. de 2026
 *      Author: milton
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>
#include <pthread.h>
#include <SDL2/SDL.h>

#include "z80.h"
#include "sdlclient.h"
#include "vga.h"
#include "panel.h"

////////////////////////////////////////////////////////////////////////////////
typedef struct {

    sdldata_t *sdl;
    int width;
    int height;
    uint8_t *rom;
    uint8_t *ram;
    z80_t z;
    pthread_t z80thread;
    ios_t *ios;
    vga_t *vga;
    panel_t *panel;
} main_data_t;

#endif /* MAIN_H_ */
