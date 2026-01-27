/*
 * leds.h
 *
 *  Created on: 21 de jan. de 2026
 *      Author: milton
 */

#ifndef LEDS_H_
#define LEDS_H_

#include <stdint.h>

typedef struct {

    uint8_t leds_port;
    uint8_t leds_port_old;
    SDL_TimerID ledsTimer;
    int ledsTick;
    int x,y;
    SDL_Renderer* renderer;
} leds_t;

leds_t *leds_init(int x, int y, SDL_Renderer* renderer);

void leds_out(leds_t *leds, uint8_t value);

void leds_refresh(leds_t *leds, int force);

#endif /* LEDS_H_ */
