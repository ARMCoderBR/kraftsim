#include <malloc.h>

#include "sdlclient.h"

////////////////////////////////////////////////////////////////////////////////
sdldata_t *sdl_init(int width, int height){

    sdldata_t *sdl = malloc(sizeof(sdldata_t));
    if (!sdl) return NULL;

    if (SDL_Init(SDL_INIT_EVENTS|SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_AUDIO) != 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return NULL; // Exit on error
    }

    sdl->width = width;
    sdl->height = height;

    sdl->window_main = SDL_CreateWindow(
            "Kraft80 Monitor",           // Window title
            SDL_WINDOWPOS_UNDEFINED,     // Initial x position
            SDL_WINDOWPOS_UNDEFINED,     // Initial y position
            width,                       // Width in pixels
            height,                      // Height in pixels
            SDL_WINDOW_SHOWN             // Flags (SDL_WINDOW_SHOWN is default)
        );


    sdl->renderer_main = SDL_CreateRenderer(sdl->window_main, -1, SDL_RENDERER_SOFTWARE);
    if (sdl->renderer_main == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(sdl->window_main);
        goto sdliniterr;
    }


    if (SDL_SetRenderDrawColor(sdl->renderer_main, 0, 0, 0, 255) < 0) {
        // Handle error (optional)
        SDL_Log("SDL_SetRenderDrawColor failed: %s", SDL_GetError());
        goto sdliniterr;
    }

    // Clear the entire screen/window to the set color
    if (SDL_RenderClear(sdl->renderer_main) < 0) {
        // Handle error (optional)
        SDL_Log("SDL_RenderClear failed: %s", SDL_GetError());
sdliniterr:
        sdl_close(sdl);
        return NULL;
    }

    // Update the screen with the rendering results
    SDL_RenderPresent(sdl->renderer_main);

    sdl->wminimized_main = 0;
    sdl->repaint_window_main = 0;

    //printf("SDL Init OK\n");

    return sdl;
}

////////////////////////////////////////////////////////////////////////////////
void sdl_close(sdldata_t *sdl){

    SDL_DestroyRenderer(sdl->renderer_main);
    SDL_DestroyWindow(sdl->window_main);
    SDL_Quit();
    free(sdl);
}
