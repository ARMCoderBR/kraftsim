/*
 * buttons.c
 *
 *  Created on: 16 de fev. de 2026
 *      Author: milton
 */

#include <stdint.h>
#include <string.h>
#include <ncurses.h>
#include <SDL2/SDL.h>
#include <malloc.h>

#include "buttons.h"

////////////////////////////////////////////////////////////////////////////////
static Uint32 buttons_set_tick(Uint32 interval, void *param){

    buttons_t *buttons = param;

    buttons->buttonsTick = 1;
    return interval;
}

////////////////////////////////////////////////////////////////////////////////
buttons_t *buttons_init(int x, int y, SDL_Renderer* renderer){

    buttons_t *buttons = malloc(sizeof(buttons_t));

    buttons->baseX = x;
    buttons->baseY = y;
    buttons->renderer = renderer;
    buttons->buttons_state = 0;
    buttons->buttons_state_old = 0xff;

    buttons->buttonsTimer = SDL_AddTimer(11, buttons_set_tick, buttons);

    return buttons;
}

////////////////////////////////////////////////////////////////////////////////
void buttons_refresh(buttons_t *buttons, int force){

    if (!force){

        if (!buttons->buttonsTick) return;
        if (buttons->buttons_state == buttons->buttons_state_old) return;
    }

    buttons->buttonsTick = 0;

    uint8_t mask = 0x80;
    for (int i = 0; i < 8; i++){

        if (force||((buttons->buttons_state ^ buttons->buttons_state_old) & mask)){

            SDL_Rect rect;

            if (buttons->buttons_state & mask){
                rect.x = buttons->baseX+20*i;
                rect.y = buttons->baseY+3;
                rect.w = 16;
                rect.h = 10;
                SDL_SetRenderDrawColor(buttons->renderer, 160, 160, 0, 160);
                SDL_RenderFillRect(buttons->renderer, &rect);
                rect.x = buttons->baseX+20*i+3;
                rect.y = buttons->baseY;
                rect.w = 10;
                rect.h = 16;
                SDL_SetRenderDrawColor(buttons->renderer, 160, 160, 0, 160);
                SDL_RenderFillRect(buttons->renderer, &rect);
                rect.x = buttons->baseX+20*i+3;
                rect.y = buttons->baseY+3;
                rect.w = 10;
                rect.h = 10;
                SDL_SetRenderDrawColor(buttons->renderer, 255, 255, 0, 255);
                SDL_RenderFillRect(buttons->renderer, &rect);

            }
            else{
                rect.x = buttons->baseX+20*i;
                rect.y = buttons->baseY+3;
                rect.w = 16;
                rect.h = 10;
                SDL_SetRenderDrawColor(buttons->renderer, 51, 51, 0, 255);
                SDL_RenderFillRect(buttons->renderer, &rect);
                rect.x = buttons->baseX+20*i+3;
                rect.y = buttons->baseY;
                rect.w = 10;
                rect.h = 16;
                SDL_SetRenderDrawColor(buttons->renderer, 51, 51, 0, 255);
                SDL_RenderFillRect(buttons->renderer, &rect);
                rect.x = buttons->baseX+20*i+3;
                rect.y = buttons->baseY+3;
                rect.w = 10;
                rect.h = 10;
                SDL_SetRenderDrawColor(buttons->renderer, 45, 45, 0, 45);
                SDL_RenderFillRect(buttons->renderer, &rect);
            }
        }

        mask >>= 1;
    }

    SDL_RenderPresent(buttons->renderer);

    buttons->buttons_state_old = buttons->buttons_state;
}

////////////////////////////////////////////////////////////////////////////////
void buttons_out(buttons_t *buttons, uint8_t value){

    buttons->buttons_state = value;
}

////////////////////////////////////////////////////////////////////////////////
void buttons_reset(buttons_t *buttons){

    buttons->buttons_state = 0;
}

////////////////////////////////////////////////////////////////////////////////
void buttons_close(buttons_t *buttons){

    SDL_RemoveTimer(buttons->buttonsTimer);

    free(buttons);
}

