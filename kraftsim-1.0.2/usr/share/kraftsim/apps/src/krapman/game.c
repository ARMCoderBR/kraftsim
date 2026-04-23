#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "map.h"
#include "graphics.h"

#include "inputs.h"
#include "timer.h"

////////////////////////////////////////////////////////////////////////////////
void init_actors (gamestate_t *g){

	g->blinky.selfId = ID_BLINKY;
	g->blinky.startCounter = 0;	// Not used
	g->blinky.X = TILE_FACTOR*12+(TILE_FACTOR/2);
	g->blinky.Y = TILE_FACTOR*10;
	g->blinky.Dir = DIR_W;
	g->blinky.isMoving = 0;	// Not used
	g->blinky.Xph = g->blinky.X + MAPXOFFSET + 4;
	g->blinky.Addr = g->blinky.Y*BYTES_PER_LINE + g->blinky.Xph/2;
	g->blinky.spriteIdx = 0;
	g->blinky.newState = 1;
	save_actor_bg(&g->blinky);

	g->pinky.selfId = ID_PINKY;
	g->pinky.startCounter = 0;	// Not used
	g->pinky.X = TILE_FACTOR*12+(TILE_FACTOR/2);
	g->pinky.Y = TILE_FACTOR*13+(TILE_FACTOR/2);
	g->pinky.Dir = DIR_N;
	g->pinky.isMoving = 0;	// Not used
	g->pinky.Xph = g->pinky.X + MAPXOFFSET + 4;
	g->pinky.Addr = g->pinky.Y*BYTES_PER_LINE + g->pinky.Xph/2;
	g->pinky.spriteIdx = 12;
	g->pinky.newState = 1;
	save_actor_bg(&g->pinky);

	g->inky.selfId = ID_INKY;
	g->inky.startCounter = 20;
	g->inky.X = TILE_FACTOR*10+(TILE_FACTOR/2);
	g->inky.Y = TILE_FACTOR*13+(TILE_FACTOR/2);
	g->inky.Dir = DIR_N;
	g->inky.isMoving = 0;	// Not used
	g->inky.Xph = g->inky.X + MAPXOFFSET + 4;
	g->inky.Addr = g->inky.Y*BYTES_PER_LINE + g->inky.Xph/2;
	g->inky.spriteIdx = 24;
	g->inky.newState = 1;
	save_actor_bg(&g->inky);

	g->clyde.selfId = ID_CLYDE;
	g->clyde.startCounter = 40;
	g->clyde.X = TILE_FACTOR*14+(TILE_FACTOR/2);
	g->clyde.Y = TILE_FACTOR*13+(TILE_FACTOR/2);
	g->clyde.Dir = DIR_N;
	g->clyde.isMoving = 0;	// Not used
	g->clyde.Xph = g->clyde.X + MAPXOFFSET + 4;
	g->clyde.Addr = g->clyde.Y*BYTES_PER_LINE + g->clyde.Xph/2;
	g->clyde.spriteIdx = 36;
	g->clyde.newState = 1;
	save_actor_bg(&g->clyde);

	g->pacman.selfId = ID_PACMAN;
	g->pacman.startCounter = 0;	// Not used
	g->pacman.X = TILE_FACTOR*12+(TILE_FACTOR/2);	//12.5*8
	g->pacman.Y = TILE_FACTOR*22;	//22*8
	g->pacman.Dir = DIR_W;
	g->pacman.isMoving = 1;
	g->pacman.Xph = g->pacman.X + MAPXOFFSET + 4;
	g->pacman.Addr = g->pacman.Y*BYTES_PER_LINE + g->pacman.Xph/2;
	g->pacman.spriteIdx = 0;	// Not used
	save_actor_bg(&g->pacman);

	g->moveTicker = 0;
	g->ghostStates = GHO_SCATTER;
	set_timer(300*7);
}

////////////////////////////////////////////////////////////////////////////////
void move_draw_actor(gamestate_t *g, actor_t *act){

	switch (act->Dir){

		case DIR_E:
			act->X++;
			act->Xph++;
			if (!(act->Xph & 1)){
				restore_sprite_col0(act->Addr, act->bg);
				act->Addr++;
				roll_spleft(act->bg);
				read_sprite_col7(act->Addr, act->bg);
				check_sprite_superpos_col7(g,act);
			}
			break;

		case DIR_N:
			restore_sprite_row15(act->Addr, act->bg);
			act->Y--;
			act->Addr -= BYTES_PER_LINE;
			roll_spdown(act->bg);
			read_sprite_row0(act->Addr, act->bg);
			check_sprite_superpos_row0(g, act);
			break;

		case DIR_W:
			act->X--;
			act->Xph--;
			if (act->Xph & 1){
				restore_sprite_col7(act->Addr, act->bg);
				act->Addr--;
				roll_spright(act->bg);
				read_sprite_col0(act->Addr, act->bg);
				check_sprite_superpos_col0(g,act);
			}
			break;

		default:	//DIR_S
			restore_sprite_row0(act->Addr, act->bg);
			act->Y++;
			act->Addr += BYTES_PER_LINE;
			roll_spup(act->bg);
			read_sprite_row15(act->Addr, act->bg);
			check_sprite_superpos_row15(g, act);
			break;
	}

	draw_actor(act);
}

////////////////////////////////////////////////////////////////////////////////
void check_cross_tunnel(actor_t *act){

	if (act->X == 204){
		restore_actor_bg(act);
		act->X = -4;
		act->Xph = act->X + MAPXOFFSET + 4;
		act->Addr = act->Y*BYTES_PER_LINE + act->Xph/2;
		memset(act->bg,0,sizeof(act->bg));
	}
	else
	if (act->X == -4){
		restore_actor_bg(act);
		act->X = 204;
		act->Xph = act->X + MAPXOFFSET + 4;
		act->Addr = act->Y*BYTES_PER_LINE + act->Xph/2;
		memset(act->bg,0,sizeof(act->bg));
	}
}

////////////////////////////////////////////////////////////////////////////////
void move_ghost(gamestate_t *g, actor_t *act){

	move_draw_actor(g, act);

	switch(g->ghostStates){
		case GHO_SCATTER:
			if (!get_timer()){
				g->ghostStates = GHO_CHASE;
				set_timer(300*20);
			}
			break;			

		case GHO_CHASE:
			if (!get_timer()){
				g->ghostStates = GHO_SCATTER;
				set_timer(300*7);
			}
			break;			
	}

	////////////////////////////////////////////////////////////////////////
	const uint8_t dirctl[] = { DIRC0, DIRC1, DIRC2, DIRC3 };
	uint8_t diropts;

	if ((act->X | act->Y) & 3) return;

	int now_x = act->X;
	int now_y = act->Y;
	int tgt_x = 0, tgt_y = 0;

	int col = now_x / TILE_FACTOR;
	int row = now_y / TILE_FACTOR;

	// Starting point for INKY
	if (act->X == TILE_FACTOR*10+(TILE_FACTOR/2)){

		if (act->Y == TILE_FACTOR*13-(TILE_FACTOR/2)){
			if (act->startCounter){
				act->startCounter--;
				act->Dir = DIR_S;
			}
			else
				act->Dir = DIR_E;
			return;
		}
		else
		if (act->Y == TILE_FACTOR*13+(TILE_FACTOR/2)){
			act->Dir = DIR_N;
			return;
		}
	}

	// Starting points for CLYDE
	if (act->X == TILE_FACTOR*14+(TILE_FACTOR/2)){

		if (act->Y == TILE_FACTOR*13-(TILE_FACTOR/2)){
			if (act->startCounter){
				act->startCounter--;
				act->Dir = DIR_S;
			}
			else
				act->Dir = DIR_W;
			return;
		}
		else
		if (act->Y == TILE_FACTOR*13+(TILE_FACTOR/2)){
			act->Dir = DIR_N;
			return;
		}
	}

	// INKY and CLYDE heading to the box exit
	if ((act->X == TILE_FACTOR*12+(TILE_FACTOR/2)) && (act->Y == TILE_FACTOR*13-(TILE_FACTOR/2))){

		act->Dir = DIR_N;
		return;
	}

	// When the ghosts leave the center box,
	// they must take an unnatural turn
	if ((act->X == TILE_FACTOR*12+(TILE_FACTOR/2)) && (act->Y == TILE_FACTOR*10) && (act->Dir == DIR_N)){

		diropts = DIRC0|DIRC2;
		goto proc_diropts;
	}

	if (!((act->X|act->Y) & 0x07)){	// Possible intersection

#define BLINKY_SCATTER_TILE_X	24
#define BLINKY_SCATTER_TILE_Y	-1

#define PINKY_SCATTER_TILE_X	2
#define PINKY_SCATTER_TILE_Y	-1

#define INKY_SCATTER_TILE_X	25
#define INKY_SCATTER_TILE_Y	29

#define CLYDE_SCATTER_TILE_X	1
#define CLYDE_SCATTER_TILE_Y	29

		uint8_t dot = get_dot(g->dots, row, col);

		diropts = dot & 0xF0;

		if (diropts){	// Found an intersection
proc_diropts:
			switch(g->ghostStates){

			case GHO_SCATTER:

				switch(act->selfId){
				case ID_BLINKY:
					tgt_x = BLINKY_SCATTER_TILE_X;
					tgt_y = BLINKY_SCATTER_TILE_Y;
					break;

				case ID_PINKY:
					tgt_x = PINKY_SCATTER_TILE_X;
					tgt_y = PINKY_SCATTER_TILE_Y;
					break;

				case ID_INKY:
					tgt_x = INKY_SCATTER_TILE_X;
					tgt_y = INKY_SCATTER_TILE_Y;
					break;

				case ID_CLYDE:
					tgt_x = CLYDE_SCATTER_TILE_X;
					tgt_y = CLYDE_SCATTER_TILE_Y;
					break;
				}
				break;

			case GHO_CHASE:

				switch(act->selfId){
				case ID_BLINKY:
					tgt_x = g->pacman.X / TILE_FACTOR;
					tgt_y = g->pacman.Y / TILE_FACTOR;
					break;

				case ID_PINKY:
					tgt_x = g->pacman.X / TILE_FACTOR;
					tgt_y = g->pacman.Y / TILE_FACTOR;
					break;

				case ID_INKY:
					tgt_x = g->pacman.X / TILE_FACTOR;
					tgt_y = g->pacman.Y / TILE_FACTOR;
					break;

				case ID_CLYDE:
					tgt_x = g->pacman.X / TILE_FACTOR;
					tgt_y = g->pacman.Y / TILE_FACTOR;
					break;
				}
				break;
			}

			// Prevents reversing direction
			if (!act->newState)
				diropts &= ~dirctl[act->Dir ^ 2];
			else
				act->newState = 0;

			////////////////////////////////////////////////////////

			int dx2E = tgt_x - (col+1);	dx2E = dx2E*dx2E;
			int dy2E = tgt_y - row;		dy2E = dy2E*dy2E;

			int dx2N = tgt_x - col;		dx2N = dx2N*dx2N;
			int dy2N = tgt_y - (row-1);	dy2N = dy2N*dy2N;

			int dx2W = tgt_x - (col-1);	dx2W = dx2W*dx2W;
			int dy2W = dy2E;

			int dx2S = dx2N;
			int dy2S = tgt_y - (row+1);	dy2S = dy2S*dy2S;

			int d2E = dx2E+dy2E; if (!(diropts & DIRC0)) d2E = 0x7fff;
			int d2N = dx2N+dy2N; if (!(diropts & DIRC1)) d2N = 0x7fff;
			int d2W = dx2W+dy2W; if (!(diropts & DIRC2)) d2W = 0x7fff;
			int d2S = dx2S+dy2S; if (!(diropts & DIRC3)) d2S = 0x7fff;

			int dirMax = 0x7fff;
			int newDir = DIR_E;
			if (d2N < dirMax){
				dirMax = d2N;
				newDir = DIR_N;
			}
			if (d2W < dirMax){
				dirMax = d2W;
				newDir = DIR_W;
			}
			if (d2S < dirMax){
				dirMax = d2S;
				newDir = DIR_S;
			}
			if (d2E < dirMax){
				newDir = DIR_E;
			}
			
			act->Dir = newDir;
		}
	}

go_dir:
	check_cross_tunnel(act);
}

////////////////////////////////////////////////////////////////////////////////
void check_player_reversal(actor_t *act, uint8_t joydir){

	if ((joydir & 0x80) && (act->Dir == DIR_W))
		{act->Dir = DIR_E; act->isMoving = 1; } // W to E

	if ((joydir & 0x40) && (act->Dir == DIR_S))
		{act->Dir = DIR_N; act->isMoving = 1; } // S to N

	if ((joydir & 0x20) && (act->Dir == DIR_E))
		{act->Dir = DIR_W; act->isMoving = 1; } // E to W

	if ((joydir & 0x10) && (act->Dir == DIR_N))
		{act->Dir = DIR_S; act->isMoving = 1; } // N to S
}

////////////////////////////////////////////////////////////////////////////////
void move_player(gamestate_t *g){

	actor_t *act = &g->actor[4];

	uint8_t joydir = update_inputs();

	if (act->isMoving)
		move_draw_actor(g, act);

	////////////////////////////////////////////////////////////////////////
	const uint8_t dirctl[] = { DIRC0, DIRC1, DIRC2, DIRC3 };
	uint8_t diropts;

	if ((act->X | act->Y) & 3) return;

	if (!((act->X|act->Y) & 0x07)){

		int col = act->X >> 3;
		int row = act->Y >> 3;

		uint8_t dot = get_dot(g->dots, row, col);

		// PAC MAN eating a dot
		if (dot & DOT){

			act->bg[DOTYOFFSET*8+DOTXOFFSET] = 0;
			act->bg[DOTYOFFSET*8+DOTXOFFSET-8] = 0;
			act->bg[DOTYOFFSET*8+DOTXOFFSET+1] = 0;
			act->bg[DOTYOFFSET*8+DOTXOFFSET-8+1] = 0;
			clr_dot(g->dots, row, col);
			g->remainDots--;
			if (!g->remainDots){
				g->state = GAM_COMPLETED;
				return;
			}
		}

		// PAC MAN eating a Power dot
		if (dot & DOTPWR){

			memset (act->bg,0,sizeof(act->bg));
			clr_dot(g->dots, row, col);

			if (row == 2){
				if (col == 0) g->pw0 = 0;
				else
				if (col == 25) g->pw1 = 0;
			}
			else
			if (row == 22){
				if (col == 0) g->pw2 = 0;
				else
				if (col == 25) g->pw3 = 0;
			}

			g->remainDots--;
			if (!g->remainDots){
				g->state = GAM_COMPLETED;
				return;
			}
		}

		diropts = dot & 0xF0;

		if (diropts){

			// Make PAC MAN stop if he finds a wall
			if (!(diropts & dirctl[act->Dir]))
				act->isMoving = 0;

			// Joystick inputs
			if ((joydir & 0x80) && (diropts&DIRC0))
				{ act->Dir = DIR_E; act->isMoving = 1; } // E
			if ((joydir & 0x40) && (diropts&DIRC1))
				{ act->Dir = DIR_N; act->isMoving = 1; } // N
			if ((joydir & 0x20) && (diropts&DIRC2))
				{ act->Dir = DIR_W; act->isMoving = 1; } // W
			if ((joydir & 0x10) && (diropts&DIRC3))
				{ act->Dir = DIR_S; act->isMoving = 1; } // S
		}
		else
			check_player_reversal(act,joydir);
	}
	else
		check_player_reversal(act,joydir);

	check_cross_tunnel(act);
}

////////////////////////////////////////////////////////////////////////////////
void check_die(gamestate_t *g){

	uint8_t pacX = g->actor[4].X;
	uint8_t pacY = g->actor[4].Y;

	int delta;

	for (int i = 0; i < 4; i++){

		delta = g->actor[i].X - pacX;
		if (delta > 8) continue;
		if (delta < -8) continue;

		delta = g->actor[i].Y - pacY;
		if (delta > 8) continue;
		if (delta < -8) continue;

		g->state = GAM_PLDIE;
		return;
	}
}

////////////////////////////////////////////////////////////////////////////////
void delay(int count){	// In 1/10 seconds

	set_timer(30*count);
	while (get_timer());
}

////////////////////////////////////////////////////////////////////////////////
void init_game(gamestate_t *g){

	draw_mapwalls();
	fill_dots(g);
	draw_dots(g);
	init_actors(g);
	g->state = GAM_RUNNING;
}

////////////////////////////////////////////////////////////////////////////////
void run_game(gamestate_t *g){

	const uint8_t speedcontrol[]={0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1};

	switch(g->state){

	case GAM_RUNNING:

		if (speedcontrol[g->moveTicker & 0x0f]){
			move_ghost(g, &g->blinky);
			move_ghost(g, &g->pinky);
			move_ghost(g, &g->inky);
			move_ghost(g, &g->clyde);
		}

		move_player(g);
		check_die(g);

		g->moveTicker++;
		if ((g->moveTicker ^ g->pwrBlinker) & 0x10){

			if (g->moveTicker & 0x10)
				redraw_pwrup_on(g);
			else
				redraw_pwrup_off(g);

			g->pwrBlinker = g->moveTicker;
		}
		break;

	case GAM_PLDIE:
		for (int i = 0; i <= 4; i++)
			restore_actor_bg(&g->actor[i]);

		draw_actor(&g->pacman);
		delay(10);
		g->state = GAM_PLDIE1;
		break;

	case GAM_PLDIE1:

		for (int i = 4; i <= 15; i++){

			delay(2);
			g->pacman.Dir = i;
			draw_actor(&g->pacman);
		}
		g->state = GAM_PLDIE2;

	case GAM_PLDIE2:

		delay(20);
		init_actors(g);
		draw_dots(g);
		g->state = GAM_RUNNING;
		break;

	case GAM_COMPLETED:
		for (int i = 0; i <= 4; i++)
			restore_actor_bg(&g->actor[i]);

		draw_actor(&g->pacman);
		delay(20);
		restore_actor_bg(&g->pacman);
		init_game(g);
		break;
	}
}

