#include <stdint.h>
#include <string.h>
#include <ncurses.h>
#include <SDL2/SDL.h>

#include "vga.h"
#include "vgafont.h"

static SDL_Texture *stamp_surface[256];

////////////////////////////////////////////////////////////////////////////////
void vga_init(activate_data_t *act){

    SDL_Surface* tempSurface = SDL_CreateRGBSurface
        (0,     /*Uint32 flags*/
         16,    /*int width*/
         16,    /*int height*/
         32,    /*int depth*/
         0,     /*Uint32 Rmask*/
         0,     /*Uint32 Gmask*/
         0,     /*Uint32 Bmask*/
         0      /*Uint32 Amask*/
         );

    SDL_Rect rect;
    rect.w = 2;
    rect.h = 2;

    SDL_Renderer* tempRenderer = SDL_CreateSoftwareRenderer(tempSurface);

    int vgafont_offset = 0;
    for (int charcode = 0; charcode < 256; charcode++){

        if (SDL_SetRenderDrawColor(tempRenderer, 0, 0, 0, 255) < 0) {
            // Handle error (optional)
            SDL_Log("SDL_SetRenderDrawColor failed: %s", SDL_GetError());
            exit(0);
        }

        // 2. Clear the entire screen/window to the set color
        if (SDL_RenderClear(tempRenderer) < 0) {
            // Handle error (optional)
            SDL_Log("SDL_RenderClear failed: %s", SDL_GetError());
            exit(0);
        }

        if (SDL_SetRenderDrawColor(tempRenderer, 0, 255, 0, 255) < 0) {
            // Handle error (optional)
            SDL_Log("SDL_SetRenderDrawColor failed: %s", SDL_GetError());
            exit(0);
        }

        for (int row = 0; row < 7; row++){

            uint8_t b = vgafont[vgafont_offset+row];
            uint8_t mask = 128;

            for (int col = 0; col < 7; col++){

                if (b & mask){

                    rect.x = col*2;
                    rect.y = row*2;

                    SDL_RenderFillRect(tempRenderer, &rect);
                }

                mask >>= 1;
            }
        }
        vgafont_offset += 8;

        stamp_surface[charcode] = SDL_CreateTextureFromSurface(act->renderer, tempSurface);
    }

    SDL_FreeSurface(tempSurface);

    rect.x = 0;
    rect.y = 0;
    rect.w = 16;
    rect.h = 16;

    SDL_RenderCopy(act->renderer, stamp_surface[65], NULL, &rect);
    rect.x += 16;
    SDL_RenderCopy(act->renderer, stamp_surface[66], NULL, &rect);
    rect.x += 16;
    SDL_RenderCopy(act->renderer, stamp_surface[67], NULL, &rect);
    SDL_RenderPresent(act->renderer); //updates the renderer

    SDL_DestroyRenderer(tempRenderer);
}

