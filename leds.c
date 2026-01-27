#include <stdint.h>
#include <string.h>
#include <ncurses.h>
#include <SDL2/SDL.h>
#include <malloc.h>

#include "leds.h"

#define LEDS_X_OFFSET 380

////////////////////////////////////////////////////////////////////////////////
void leds_refresh(leds_t *leds, int force){

    if (!leds->ledsTick) return;
    leds->ledsTick = 0;

    if (leds->leds_port == leds->leds_port_old) return;

    uint8_t mask = 0x80;
    for (int i = 0; i < 8; i++){

        if ((leds->leds_port ^ leds->leds_port_old) & mask){

            SDL_Rect rect;
            rect.x = leds->baseX+20*i;
            rect.y = leds->baseY;
            rect.w = 16;
            rect.h = 16;

            if (leds->leds_port & mask)
                SDL_SetRenderDrawColor(leds->renderer, 255, 255, 0, 255);
            else
                SDL_SetRenderDrawColor(leds->renderer, 51, 51, 0, 255);

            SDL_RenderFillRect(leds->renderer, &rect);
        }

        mask >>= 1;
    }

    SDL_RenderPresent(leds->renderer);

    leds->leds_port_old = leds->leds_port;
}

////////////////////////////////////////////////////////////////////////////////
static Uint32 leds_set_tick(Uint32 interval, void *param){

    leds_t *leds = param;

    leds->ledsTick = 1;
    return interval;
}

////////////////////////////////////////////////////////////////////////////////
leds_t *leds_init(int x, int y, SDL_Renderer* renderer){

    leds_t *leds = malloc(sizeof(leds_t));

    leds->baseX = x;
    leds->baseY = y;
    leds->renderer = renderer;
    leds->leds_port = 0;
    leds->leds_port_old = 0xff;

    leds->ledsTimer = SDL_AddTimer(11, leds_set_tick, leds);

    return leds;
}

////////////////////////////////////////////////////////////////////////////////
void leds_out(leds_t *leds, uint8_t value){

    leds->leds_port = value;
}

