/*
 * buttons.h
 *
 *  Created on: 16 de fev. de 2026
 *      Author: milton
 */

#ifndef BUTTONS_H_
#define BUTTONS_H_

#include <stdint.h>
#include <SDL2/SDL.h>

typedef struct {

    uint8_t buttons_state;
    uint8_t buttons_state_old;
    SDL_TimerID buttonsTimer;
    int buttonsTick;
    int baseX,baseY;
    SDL_Renderer* renderer;
} buttons_t;

////////////////////////////////////////////////////////////////////////////////

buttons_t *buttons_init(int x, int y, SDL_Renderer* renderer);
void buttons_refresh(buttons_t *buttons, int force);
void buttons_out(buttons_t *buttons, uint8_t value);
void buttons_reset(buttons_t *buttons);
void buttons_close(buttons_t *buttons);

#endif /* BUTTONS_H_ */
