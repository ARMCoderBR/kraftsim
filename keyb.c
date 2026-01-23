/*
 * keys.c
 *
 *  Created on: 23 de jan. de 2026
 *      Author: milton
 */


#include <SDL2/SDL.h>

SDL_Event event;

////////////////////////////////////////////////////////////////////////////////
void __keyb_run(void){

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

void keyb_run(void){

    for (;;){
        __keyb_run();
        usleep(100000);
    }
}

////////////////////////////////////////////////////////////////////////////////
void keyb_init(){

    if (SDL_Init(SDL_INIT_EVENTS) != 0) {
         printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
         return; // Exit on error
     }

    printf("SDL Init OK\n");
}
