#include <stdint.h>
#include <string.h>
#include <malloc.h>

#include <SDL2/SDL.h>

#include "vga.h"
#include "vgafont.h"

////////////////////////////////////////////////////////////////////////////////
vga_t *vga_init(SDL_Renderer* renderer){

    vga_t *vga = malloc(sizeof(vga_t));
    if (!vga) return NULL;

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

        // Clear the entire screen/window to the set color
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

        vga->fontTexture[charcode] = SDL_CreateTextureFromSurface(renderer, tempSurface);
    }

    SDL_FreeSurface(tempSurface);
    SDL_DestroyRenderer(tempRenderer);

    rect.x = 0;
    rect.y = 0;
    rect.w = 16;
    rect.h = 16;

    SDL_RenderCopy(renderer, vga->fontTexture[65], NULL, &rect);
    rect.x += 16;
    SDL_RenderCopy(renderer, vga->fontTexture[66], NULL, &rect);
    rect.x += 16;
    SDL_RenderCopy(renderer, vga->fontTexture[67], NULL, &rect);
    SDL_RenderPresent(renderer); //updates the renderer

    return vga;
}

////////////////////////////////////////////////////////////////////////////////
void vga_close(vga_t *vga){

    for (int charcode = 0; charcode < 256; charcode++)
        SDL_DestroyTexture(vga->fontTexture[charcode]);

    free(vga);
}
