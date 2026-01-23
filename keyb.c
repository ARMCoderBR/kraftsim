/*
 * keys.c
 *
 *  Created on: 23 de jan. de 2026
 *      Author: milton
 */


#include <SDL2/SDL.h>

SDL_Event event;
void keyb_run(void){

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            // Handle quit event
        } else if (event.type == SDL_KEYDOWN) {
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
