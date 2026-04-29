#ifndef __GAME_H__
#define __GAME_H__

#include <stdint.h>

#define DOTS_SX 26
#define DOTS_SY 29
#define DOTS_TOTAL (DOTS_SX*DOTS_SY)

#define DIR_E	0
#define DIR_N	1
#define DIR_W	2
#define DIR_S	3

#define DOT	0x01
#define DOTPWR	0x02
#define DIRC0	0x10
#define DIRC1	0x20
#define DIRC2	0x40
#define DIRC3	0x80

#define TILE_FACTOR	8

typedef struct {

	uint8_t selfId;
	int	X;
	int	Xph;
	uint8_t Y;
	uint8_t Dir;
	uint8_t isMoving;
	uint8_t startCounter;
	int	Addr;
	uint8_t spriteIdx;
	uint8_t newState;
	uint8_t bg[128];
} actor_t;

#define LASTACTOR 4

typedef struct {

	uint8_t state;
	actor_t actor[1+LASTACTOR];	//blinky, pinky, inky, clyde, pac
	uint8_t	dots[DOTS_TOTAL];
	uint8_t	remainDots;
	uint8_t	ghostStates;
	uint8_t moveTicker;
	uint8_t pwrBlinker;
	int	AddrPW0;
	uint8_t pw0;
	int	AddrPW1;
	uint8_t pw1;
	int	AddrPW2;
	uint8_t pw2;
	int	AddrPW3;
	uint8_t pw3;
} gamestate_t;

#define GAM_RUNNING	1
#define GAM_PLDIE	2
#define GAM_PLDIE1	3
#define GAM_PLDIE2	4
#define GAM_COMPLETED	5
#define GAM_OVER	99

#define GHO_SCATTER	0
#define GHO_CHASE	1
#define GHO_SCARED	2

#define ID_BLINKY	0
#define ID_PINKY	1
#define ID_INKY	2
#define ID_CLYDE	3
#define ID_PACMAN	4

#define blinky		actor[ID_BLINKY]
#define pinky		actor[ID_PINKY]
#define inky		actor[ID_INKY]
#define clyde		actor[ID_CLYDE]
#define pacman		actor[ID_PACMAN]

void init_game (gamestate_t *g);
void run_game(gamestate_t *g);

#endif

