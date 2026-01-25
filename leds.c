#include <stdint.h>
#include <string.h>
#include <ncurses.h>

#include "leds.h"


uint8_t leds_port;
uint8_t leds_port_old;
SDL_TimerID ledsTimer;
int ledsTick;

#define LEDS_X_OFFSET 380
//#define LEDS_Y_OFFSET 505

////////////////////////////////////////////////////////////////////////////////
void leds_refresh(void *userdata){

    if (!ledsTick) return;
    ledsTick = 0;

    activate_data_t *act = userdata;

    if (leds_port == leds_port_old);

    uint8_t mask = 0x80;
    for (int i = 0; i < 8; i++){

        if ((leds_port ^ leds_port_old) & mask){

            SDL_Rect rect;
            rect.x = LEDS_X_OFFSET+20*i;
            rect.y = act->height - 25;
            rect.w = 16;
            rect.h = 16;

            if (leds_port & mask)
                SDL_SetRenderDrawColor(act->renderer, 255, 255, 0, 255);
            else
                SDL_SetRenderDrawColor(act->renderer, 51, 51, 0, 255);

            SDL_RenderFillRect(act->renderer, &rect);
            //act->ledsRender = 1;
        }

        mask >>= 1;
    }

    SDL_RenderPresent(act->renderer);

    leds_port_old = leds_port;
}

////////////////////////////////////////////////////////////////////////////////
Uint32 leds_set_tick(Uint32 interval, void *param){

    ledsTick = 1;
    return interval;
}

////////////////////////////////////////////////////////////////////////////////
void leds_init(activate_data_t *act){

    leds_port = 0;
    leds_port_old = 0xff;

    ledsTimer = SDL_AddTimer(11, leds_set_tick, NULL);
}

////////////////////////////////////////////////////////////////////////////////
void leds_out(uint8_t value){

    leds_port = value;
}

