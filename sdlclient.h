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
    SDL_Window* window;
    SDL_Renderer* renderer;
    int wminimized;
    int repaint_window;
} sdldata_t;

sdldata_t *sdl_init(int width, int height);
void sdl_drawlowerborder(sdldata_t *sdl, int bheight);

void sdl_close(sdldata_t *sdl);

#endif /* SDLCLIENT_H_ */
