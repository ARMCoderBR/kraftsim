#include <stdint.h>
#include <string.h>
#include <ncurses.h>
#include <SDL2/SDL.h>
#include <malloc.h>

#include "leds.h"

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
void leds_refresh(leds_t *leds, int force){

    if (!force){

        if (!leds->ledsTick) return;
        if (leds->leds_port == leds->leds_port_old) return;
    }

    leds->ledsTick = 0;

    uint8_t mask = 0x01;
    for (int i = 0; i < 8; i++){

        if (force||((leds->leds_port ^ leds->leds_port_old) & mask)){

            SDL_Rect rect;

            if (leds->leds_port & mask){
                rect.x = leds->baseX;
                rect.y = leds->baseY+20*i+3;
                rect.w = 16;
                rect.h = 10;
                SDL_SetRenderDrawColor(leds->renderer, 180, 180, 0, 255);
                SDL_RenderFillRect(leds->renderer, &rect);
                rect.x = leds->baseX+3;
                rect.y = leds->baseY+20*i;
                rect.w = 10;
                rect.h = 16;
                SDL_SetRenderDrawColor(leds->renderer, 180, 180, 0, 255);
                SDL_RenderFillRect(leds->renderer, &rect);
                rect.x = leds->baseX+3;
                rect.y = leds->baseY+20*i+3;
                rect.w = 10;
                rect.h = 10;
                SDL_SetRenderDrawColor(leds->renderer, 255, 255, 210, 255);
                SDL_RenderFillRect(leds->renderer, &rect);
            }
            else{
                rect.x = leds->baseX;
                rect.y = leds->baseY+20*i+3;
                rect.w = 16;
                rect.h = 10;
                SDL_SetRenderDrawColor(leds->renderer, 61, 61, 0, 255);
                SDL_RenderFillRect(leds->renderer, &rect);
                rect.x = leds->baseX+3;
                rect.y = leds->baseY+20*i;
                rect.w = 10;
                rect.h = 16;
                SDL_SetRenderDrawColor(leds->renderer, 61, 61, 0, 255);
                SDL_RenderFillRect(leds->renderer, &rect);
                rect.x = leds->baseX+3;
                rect.y = leds->baseY+20*i+3;
                rect.w = 10;
                rect.h = 10;
                SDL_SetRenderDrawColor(leds->renderer, 95, 95, 0, 255);
                SDL_RenderFillRect(leds->renderer, &rect);
                rect.x = leds->baseX+5;
                rect.y = leds->baseY+20*i+5;
                rect.w = 3;
                rect.h = 3;
                SDL_SetRenderDrawColor(leds->renderer, 180, 180, 160, 255);
                SDL_RenderFillRect(leds->renderer, &rect);
            }
        }

        mask <<= 1;
    }

    SDL_RenderPresent(leds->renderer);

    leds->leds_port_old = leds->leds_port;
}

////////////////////////////////////////////////////////////////////////////////
void leds_out(leds_t *leds, uint8_t value){

    leds->leds_port = value;
}

////////////////////////////////////////////////////////////////////////////////
void leds_reset(leds_t *leds){

    leds->leds_port = 0;
    leds->leds_port_old = 0xff;
}

////////////////////////////////////////////////////////////////////////////////
void leds_close(leds_t *leds){

    SDL_RemoveTimer(leds->ledsTimer);

    free(leds);
}
