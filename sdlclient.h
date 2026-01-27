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

    SDL_Window* window;
    SDL_Renderer* renderer;
} sdldata_t;

sdldata_t *sdl_init(int width, int height);

void sdl_close(sdldata_t *sdl);

#endif /* SDLCLIENT_H_ */
