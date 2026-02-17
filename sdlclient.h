/*
 * sdlclient.h
 *
 *  Created on: 26 de jan. de 2026
 *      Author: milton
 */

#ifndef SDLCLIENT_H_
#define SDLCLIENT_H_

#include <SDL2/SDL.h>

typedef struct {

    int width;
    int height;
    SDL_Window* window_main;
    SDL_Renderer* renderer_main;
    int wminimized_main;
    int repaint_window_main;
} sdldata_t;

sdldata_t *sdl_init(int width, int height);
void sdl_drawpanelback(sdldata_t *sdl);

void sdl_close(sdldata_t *sdl);

#endif /* SDLCLIENT_H_ */
