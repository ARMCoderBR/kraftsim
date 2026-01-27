#include <malloc.h>

#include "sdlclient.h"

////////////////////////////////////////////////////////////////////////////////
sdldata_t *sdl_init(int width, int height){

    sdldata_t *sdl = malloc(sizeof(sdldata_t));
    if (!sdl) return NULL;

    if (SDL_Init(SDL_INIT_EVENTS|SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return NULL; // Exit on error
    }

    sdl->window = SDL_CreateWindow(
            "Kraft80 Monitor",           // Window title
            SDL_WINDOWPOS_UNDEFINED,        // Initial x position
            SDL_WINDOWPOS_UNDEFINED,        // Initial y position
            width,                            // Width in pixels
            height,                            // Height in pixels
            SDL_WINDOW_SHOWN                // Flags (SDL_WINDOW_SHOWN is default)
        );

    sdl->renderer = SDL_CreateRenderer(sdl->window, 0, SDL_RENDERER_ACCELERATED);
    if (sdl->renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(sdl->window);
        goto sdliniterr;
    }

    if (SDL_SetRenderDrawColor(sdl->renderer, 0, 0, 0, 255) < 0) {
        // Handle error (optional)
        SDL_Log("SDL_SetRenderDrawColor failed: %s", SDL_GetError());
        goto sdliniterr;
    }

    // 2. Clear the entire screen/window to the set color
    if (SDL_RenderClear(sdl->renderer) < 0) {
        // Handle error (optional)
        SDL_Log("SDL_RenderClear failed: %s", SDL_GetError());
sdliniterr:
        sdl_close(sdl);
        return NULL;
    }

    // 3. Update the screen with the rendering results
    SDL_RenderPresent(sdl->renderer);

    printf("SDL Init OK\n");

    return sdl;
}

////////////////////////////////////////////////////////////////////////////////
void sdl_close(sdldata_t *sdl){

    SDL_Quit();
    free(sdl);
}
