/*
 * keys.c
 *
 *  Created on: 23 de jan. de 2026
 *      Author: milton
 */


#include <SDL2/SDL.h>

#include "act.h"
#include "leds.h"
#include "lcd.h"

SDL_Event event;

////////////////////////////////////////////////////////////////////////////////
void keyb_run(activate_data_t *act){

    leds_refresh(act);
    lcd_refresh(act);

    while (SDL_PollEvent(&event)) {
        printf("poll\n");
        if (event.type == SDL_QUIT) {
            // Handle quit event
        } else if (event.type == SDL_KEYDOWN) {
            printf("EventSym:%d\n",event.key.keysym.sym);
            switch (event.key.keysym.sym) {

                case SDLK_UP:
                    // Handle up arrow key press
                    break;
                case SDLK_DOWN:
                    // Handle down arrow key press
                    break;
                case SDLK_a:
                    // Handle 'a' key press
                    break;
                // ... more keys
            }
        } else if (event.type == SDL_KEYUP) {
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    // Handle escape key release
                    break;
                // ... more keys
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
int keyb_init(activate_data_t *act){

    if (SDL_Init(SDL_INIT_EVENTS|SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0) {
         printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
         return -1; // Exit on error
     }

    act->window = SDL_CreateWindow(
            "SDL Window Example",           // Window title
            SDL_WINDOWPOS_UNDEFINED,        // Initial x position
            SDL_WINDOWPOS_UNDEFINED,        // Initial y position
            act->width,                            // Width in pixels
            act->height,                            // Height in pixels
            SDL_WINDOW_SHOWN                // Flags (SDL_WINDOW_SHOWN is default)
        );


    act->renderer = SDL_CreateRenderer(act->window, 0, SDL_RENDERER_ACCELERATED);
       if (act->renderer == NULL) {
           printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
           SDL_DestroyWindow(act->window);
           SDL_Quit();
           return -1;
       }

       if (SDL_SetRenderDrawColor(act->renderer, 0, 0, 0, 255) < 0) {
           // Handle error (optional)
           SDL_Log("SDL_SetRenderDrawColor failed: %s", SDL_GetError());
       }

       // 2. Clear the entire screen/window to the set color
       if (SDL_RenderClear(act->renderer) < 0) {
           // Handle error (optional)
           SDL_Log("SDL_RenderClear failed: %s", SDL_GetError());
       }

       // 3. Update the screen with the rendering results
       SDL_RenderPresent(act->renderer);

    printf("SDL Init OK\n");
    return 0;
}
