/*
 * keys.c
 *
 *  Created on: 23 de jan. de 2026
 *      Author: milton
 */


#include "leds.h"
#include "lcd.h"
#include "main.h"


////////////////////////////////////////////////////////////////////////////////
void keyb_run(main_data_t *maindata){

    leds_refresh(maindata->leds,0);
    lcd_refresh(maindata->lcd, 0);
    vga_refresh(maindata->vga, 0);

    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        //printf("poll\n");
        if (event.type == SDL_QUIT) {
            // Handle quit event
        } else if (event.type == SDL_KEYDOWN) {
            printf("DOWN:EventSym:%d\n",event.key.keysym.sym);
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
            printf("UP:EventSym:%d\n",event.key.keysym.sym);
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
int keyb_init(main_data_t *maindata){

    return 0;
}
