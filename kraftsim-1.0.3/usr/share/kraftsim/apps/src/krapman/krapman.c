/*
KRAPMAN.C
Example program for the KRAFT 80
2026 - ARM Coder
*/

#include <stdio.h>
#include <ctype.h>
#include <stdint.h>

#include "io-kraft80.h"

#include "graphics.h"
#include "map.h"
#include "game.h"

#include "inputs.h"
#include "timer.h"

#pragma codeseg CODE

////////////////////////////////////////////////////////////////////////////////
void main (void){

	gamestate_t game;

	video_begin(1);

	init_inputs();

	init_timer();

	init_game(&game);

	for (;;){
		run_game(&game);
	}
}

////////////////////////////////////////////////////////////////////////////////
void menu_update(void){}

