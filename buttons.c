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

#define BUTTONS_SIDE  32
#define BUTTONS_LEADSIDE  4
#define BUTTONS_STEP  48

////////////////////////////////////////////////////////////////////////////////
static void draw_buttonsback(SDL_Renderer* renderer, int x, int y) {

    SDL_Rect rect;

    for (int i = 0; i < 9; i++){

        int ipos = i;
        if (ipos == 8)
            ipos = 9;

        rect.x = x + ipos * BUTTONS_STEP;
        rect.y = y;
        rect.w = BUTTONS_SIDE;
        rect.h = BUTTONS_SIDE;

        SDL_SetRenderDrawColor(renderer, 124, 124, 124, 255);
        SDL_RenderFillRect(renderer, &rect);

        SDL_SetRenderDrawColor(renderer, 164, 164, 84, 255);
        rect.x = x + ipos * BUTTONS_STEP - BUTTONS_LEADSIDE;
        rect.y = y + BUTTONS_LEADSIDE;
        rect.w = BUTTONS_LEADSIDE;
        rect.h = BUTTONS_LEADSIDE;
        SDL_RenderFillRect(renderer, &rect);
        rect.y = y + BUTTONS_SIDE - 2*BUTTONS_LEADSIDE;
        SDL_RenderFillRect(renderer, &rect);
        rect.x = x + ipos * BUTTONS_STEP + BUTTONS_SIDE;
        SDL_RenderFillRect(renderer, &rect);
        rect.y = y + BUTTONS_LEADSIDE;
        SDL_RenderFillRect(renderer, &rect);

        SDL_SetRenderDrawColor(renderer, 16, 16, 16, 255);
        rect.x = x + ipos * BUTTONS_STEP + BUTTONS_LEADSIDE;
        rect.y = y + BUTTONS_LEADSIDE;
        SDL_RenderFillRect(renderer, &rect);
        rect.y = y + BUTTONS_SIDE - 2*BUTTONS_LEADSIDE;
        SDL_RenderFillRect(renderer, &rect);
        rect.x = x + ipos * BUTTONS_STEP + BUTTONS_SIDE - 2*BUTTONS_LEADSIDE;
        SDL_RenderFillRect(renderer, &rect);
        rect.y = y + BUTTONS_LEADSIDE;
        SDL_RenderFillRect(renderer, &rect);
    }

    SDL_RenderPresent(renderer);
}

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
    buttons->buttons_state = 0x1ff;
    buttons->buttons_state_old = 0x00;

    buttons->buttonsTimer = SDL_AddTimer(50, buttons_set_tick, buttons);

    draw_buttonsback(renderer, x, y);

    return buttons;
}

////////////////////////////////////////////////////////////////////////////////
void buttons_refresh(buttons_t *buttons, int force){

    if (!force){

        if (!buttons->buttonsTick) return;
        if (buttons->buttons_state == buttons->buttons_state_old) return;
    }
    else
        draw_buttonsback(buttons->renderer, buttons->baseX, buttons->baseY);

    buttons->buttonsTick = 0;

    uint16_t mask = 0x01;
    for (int i = 0; i < 9; i++){

        int ipos = i;
        if (ipos == 8)
            ipos = 9;

        if (force||((buttons->buttons_state ^ buttons->buttons_state_old) & mask)){

            SDL_Rect rect;

            if (!(buttons->buttons_state & mask)){  // Pressed
                rect.x = buttons->baseX+ipos*BUTTONS_STEP+8;
                rect.y = buttons->baseY+3+8;
                rect.w = 16;
                rect.h = 10;
                SDL_SetRenderDrawColor(buttons->renderer, 0, 160, 0, 160);
                SDL_RenderFillRect(buttons->renderer, &rect);
                rect.x = buttons->baseX+ipos*BUTTONS_STEP+3+8;
                rect.y = buttons->baseY+8;
                rect.w = 10;
                rect.h = 16;
                SDL_SetRenderDrawColor(buttons->renderer, 0, 160, 0, 160);
                SDL_RenderFillRect(buttons->renderer, &rect);
                rect.x = buttons->baseX+ipos*BUTTONS_STEP+3+8;
                rect.y = buttons->baseY+3+8;
                rect.w = 10;
                rect.h = 10;
                SDL_SetRenderDrawColor(buttons->renderer, 0, 255, 0, 255);
                SDL_RenderFillRect(buttons->renderer, &rect);
            }
            else{                                   // Not pressed
                rect.x = buttons->baseX+ipos*BUTTONS_STEP+8;
                rect.y = buttons->baseY+3+8;
                rect.w = 16;
                rect.h = 10;
                SDL_SetRenderDrawColor(buttons->renderer, 61, 61, 61, 255);
                SDL_RenderFillRect(buttons->renderer, &rect);
                rect.x = buttons->baseX+ipos*BUTTONS_STEP+3+8;
                rect.y = buttons->baseY+8;
                rect.w = 10;
                rect.h = 16;
                SDL_SetRenderDrawColor(buttons->renderer, 61, 61, 61, 255);
                SDL_RenderFillRect(buttons->renderer, &rect);
                rect.x = buttons->baseX+ipos*BUTTONS_STEP+3+8;
                rect.y = buttons->baseY+3+8;
                rect.w = 10;
                rect.h = 10;
                SDL_SetRenderDrawColor(buttons->renderer, 0, 0, 0, 45);
                SDL_RenderFillRect(buttons->renderer, &rect);
            }
        }

        mask <<= 1;
    }

    SDL_RenderPresent(buttons->renderer);

    buttons->buttons_state_old = buttons->buttons_state;
}

////////////////////////////////////////////////////////////////////////////////
void buttons_update(buttons_t *buttons, uint16_t bstate){

    buttons->buttons_state = bstate;
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

