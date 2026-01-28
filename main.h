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
#include "leds.h"
#include "sdlclient.h"
#include "vga.h"
#include "lcd.h"

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
    leds_t *leds;
    lcd_t *lcd;
    vga_t *vga;
} main_data_t;

#endif /* MAIN_H_ */
