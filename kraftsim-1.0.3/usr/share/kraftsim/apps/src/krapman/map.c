#include "map.h"
#include "graphics.h"
#include "game.h"

////////////////////////////////////////////////////////////////////////////////
void draw_hline (int x, int y, int len){

	int xp = x+MAPXOFFSET;
	int xpm = (MAZEWIDTH-1-x)+MAPXOFFSET;

	for (int i = 0; i < len; i++){
	
		plotasm (xp++, y, MAZECOLOR);
		plotasm (xpm--, y, MAZECOLOR);
	}
}

////////////////////////////////////////////////////////////////////////////////
void draw_vline (int x, int y, int len){

	int xp = x+MAPXOFFSET;
	int xpm = (MAZEWIDTH-1-x)+MAPXOFFSET;
	int yp = y;

	for (int i = 0; i < len; i++){
	
		plotasm (xp, yp, MAZECOLOR);
		plotasm (xpm, yp++, MAZECOLOR);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void draw_plot (int x, int y){

	int xp = x+MAPXOFFSET;
	int xpm = (MAZEWIDTH-1-x)+MAPXOFFSET;

	plotasm (xp, y, MAZECOLOR);
	plotasm (xpm, y, MAZECOLOR);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void draw_mapwalls(){

	draw_plot (4,1);
	draw_plot (107,1);

	draw_vline(0,0,72);
	draw_vline(3,2,69);

	draw_hline(5,0,103);

	draw_vline(108,2,28);

	draw_plot (107,1);

	draw_plot (21,17);
	draw_plot (42,17);
	draw_plot (21,30);
	draw_plot (42,30);

	draw_hline(22,16,20);
	draw_hline(22,31,20);

	draw_vline(20,18,12);
	draw_vline(43,18,12);

	draw_hline(110,31,2);

	draw_plot (109,30);

	draw_plot (61,17);
	draw_plot (90,17);
	draw_plot (61,30);
	draw_plot (90,30);

	draw_hline(62,16,28);
	draw_hline(62,31,28);

	draw_vline(60,18,12);
	draw_vline(91,18,12);

	draw_plot (21,49);
	draw_plot (42,49);
	draw_plot (21,54);
	draw_plot (42,54);

	draw_hline(22,48,20);
	draw_hline(22,55,20);

	draw_vline(20,50,4);
	draw_vline(43,50,4);

	draw_hline(86,48,26);
	draw_hline(86,55,21);

	draw_vline(84,50,4);

	draw_plot (85,49);
	draw_plot (85,54);
	draw_plot (107,56);

	draw_vline(108,57,21);

	draw_plot (109,78);

	draw_hline(110,79,2);

	draw_hline(62,48,4);
	draw_hline(62,103,4);
	draw_hline(69,72,21);
	draw_hline(69,79,21);

	draw_vline(60,50,52);
	draw_vline(67,50,21);
	draw_vline(67,81,21);
	draw_vline(91,74,4);

	draw_plot (61,49);
	draw_plot (66,49);
	draw_plot (61,102);
	draw_plot (66,102);
	draw_plot (68,71);
	draw_plot (68,80);
	draw_plot (90,73);
	draw_plot (90,78);

	draw_hline(5,72,37);

	draw_plot (4,71);

	draw_vline(1,72,2);

	draw_hline(2,74,2);
	draw_hline(4,75,36);

	draw_vline(40,76,24);
	draw_vline(43,74,28);

	draw_plot (42,73);

	draw_hline(0,100,40);
	draw_hline(0,103,42);

	draw_plot (42,102);

	draw_hline(62,120,4);
	draw_hline(62,151,4);

	draw_vline(60,122,28);
	draw_vline(67,122,28);

	draw_plot (61,121);
	draw_plot (66,121);
	draw_plot (61,150);
	draw_plot (66,150);

	draw_hline(0,120,42);
	draw_hline(0,123,40);

	draw_vline(40,124,24);
	draw_vline(43,122,28);

	draw_plot (42,121);
	draw_plot (42,150);

	draw_hline(84,96,20);
	draw_hline(87,99,17);

	draw_vline(103,97,2);
	draw_vline(84,97,30);
	draw_vline(87,100,24);

	draw_hline(87,124,25);
	draw_hline(84,127,28);

	draw_hline(4,148,36);
	draw_hline(5,151,37);

	draw_plot (42,150);

	draw_hline(2,149,2);

	draw_vline(1,150,2);
	draw_vline(0,152,88);

	draw_plot (4,152);

	draw_vline(3,153,38);

	draw_plot (4,191);

	draw_hline(5,192,13);

	draw_plot (18,193);

	draw_vline(19,194,4);

	draw_plot (18,198);

	draw_hline(5,199,13);

	draw_plot (4,200);

	draw_vline(3,201,37);

	draw_plot (4,238);

	draw_hline(5,239,107);

	draw_hline(22,168,20);
	draw_hline(22,175,13);
	draw_hline(38,199,4);

	draw_vline(20,170,4);
	draw_vline(36,177,21);
	draw_vline(43,170,28);

	draw_plot (21,169);
	draw_plot (21,174);
	draw_plot (42,169);
	draw_plot (35,176);
	draw_plot (37,198);
	draw_plot (42,198);

	draw_hline(86,144,26);
	draw_hline(86,151,21);
	draw_hline(110,175,2);

	draw_vline(84,146,4);
	draw_vline(108,153,21);

	draw_plot (85,145);
	draw_plot (85,150);
	draw_plot (107,152);
	draw_plot (109,174);

	draw_hline(86,144+48,26);
	draw_hline(86,151+48,21);
	draw_hline(110,175+48,2);

	draw_vline(84,146+48,4);
	draw_vline(108,153+48,21);

	draw_plot (85,145+48);
	draw_plot (85,150+48);
	draw_plot (107,152+48);
	draw_plot (109,174+48);

	draw_hline(62,168,28);
	draw_hline(62,175,28);

	draw_vline(60,170,4);
	draw_vline(91,170,4);

	draw_plot (61,169);
	draw_plot (61,174);
	draw_plot (90,169);
	draw_plot (90,174);

	draw_hline(62,192,4);
	draw_hline(22,216,37);
	draw_hline(69,216,21);
	draw_hline(22,223,68);

	draw_vline(60,194,21);
	draw_vline(67,194,21);
	draw_vline(20,218,4);
	draw_vline(91,218,4);

	draw_plot (61,193);
	draw_plot (66,193);
	draw_plot (59,215);
	draw_plot (68,215);
	draw_plot (21,217);
	draw_plot (90,217);
	draw_plot (21,222);
	draw_plot (90,222);

	int xp = MAPXOFFSET + 104;
	int y = 96;
	for (int i = 0; i < 16; i++){
	
		plotasm (xp, y, DOORCOLOR);
		plotasm (xp, y+1, DOORCOLOR);
		plotasm (xp, y+2, DOORCOLOR);
		plotasm (xp, y+3, DOORCOLOR);
		xp++;
	}

}

////////////////////////////////////////////////////////////////////////////////
void fill_row(gamestate_t *g,int rowNum,int startX, int endX){

	char *dots = g->dots;

	int offset = rowNum*DOTS_SX;

	int i = startX;

	while (i <= endX){

		dots[offset+i] = DOT;
		dots[offset+((DOTS_SX-1)-i)] = DOT;
		g->remainDots += 2;
		i++;
	}
}

////////////////////////////////////////////////////////////////////////////////
void set_bits(char *dots,int rowNum,int colNum, uint8_t bits){

	int offset = rowNum*DOTS_SX;

	dots[offset+colNum] |= bits;
}

////////////////////////////////////////////////////////////////////////////////
uint8_t get_dot(char *dots,int rowNum,int colNum){

	int offset = rowNum*DOTS_SX;

	return dots[offset+colNum];
}

////////////////////////////////////////////////////////////////////////////////
void clr_dot(char *dots,int rowNum,int colNum){

	int offset = rowNum*DOTS_SX;

	dots[offset+colNum] &= ~(DOT|DOTPWR);
}

////////////////////////////////////////////////////////////////////////////////
void fill_dots(gamestate_t *g){

	int i;
	for (i = 0; i < DOTS_TOTAL; i++)
		g->dots[i] = 0;

	g->remainDots = 0;

	//Row 0
	fill_row(g,0,0,11);
	//Row 1
	fill_row(g,1,0,0);
	fill_row(g,1,5,5);
	fill_row(g,1,11,11);
	//Row 2
	set_bits(g->dots,2,0,DOTPWR);
	g->AddrPW0 = (DOTPWYOFFSET+2*8)*BYTES_PER_LINE + MAPXOFFSET/2 + DOTPWXOFFSET/2;
	g->pw0 = 1;
	set_bits(g->dots,2,25,DOTPWR);
	g->AddrPW1 = (DOTPWYOFFSET+2*8)*BYTES_PER_LINE + MAPXOFFSET/2 + (25*8/2) + DOTPWXOFFSET/2;
	g->pw1 = 1;
	g->remainDots += 2;
	fill_row(g,2,5,5);
	fill_row(g,2,11,11);
	//Row 3
	fill_row(g,3,0,0);
	fill_row(g,3,5,5);
	fill_row(g,3,11,11);
	//Row 4
	fill_row(g,4,0,12);
	//Row 5
	fill_row(g,5,0,0);
	fill_row(g,5,5,5);
	fill_row(g,5,8,8);
	//Row 6
	fill_row(g,6,0,0);
	fill_row(g,6,5,5);
	fill_row(g,6,8,8);
	//Row 7
	fill_row(g,7,0,5);
	fill_row(g,7,8,11);
	//Row 8
	fill_row(g,8,5,5);
	//Row 9
	fill_row(g,9,5,5);
	//Row 10
	fill_row(g,10,5,5);
	//Row 11
	fill_row(g,11,5,5);
	//Row 12
	fill_row(g,12,5,5);
	//Row 13
	fill_row(g,13,5,5);
	//Row 14
	fill_row(g,14,5,5);
	//Row 15
	fill_row(g,15,5,5);
	//Row 16
	fill_row(g,16,5,5);
	//Row 17
	fill_row(g,17,5,5);
	//Row 18
	fill_row(g,18,5,5);
	//Row 19
	fill_row(g,19,0,11);
	//Row 20
	fill_row(g,20,0,0);
	fill_row(g,20,5,5);
	fill_row(g,20,11,11);
	//Row 21
	fill_row(g,21,0,0);
	fill_row(g,21,5,5);
	fill_row(g,21,11,11);
	//Row 22
	set_bits(g->dots,22,0,DOTPWR);
	g->AddrPW2 = (DOTPWYOFFSET+22*8)*BYTES_PER_LINE + MAPXOFFSET/2 + DOTPWXOFFSET/2;
	g->pw2 = 1;
	set_bits(g->dots,22,25,DOTPWR);
	g->AddrPW3 = (DOTPWYOFFSET+22*8)*BYTES_PER_LINE + MAPXOFFSET/2 + (25*8/2) + DOTPWXOFFSET/2;
	g->pw3 = 1;
	g->remainDots += 2;
	fill_row(g,22,1,2);
	fill_row(g,22,5,11);
	//Row 23
	fill_row(g,23,2,2);
	fill_row(g,23,5,5);
	fill_row(g,23,8,8);
	//Row 24
	fill_row(g,24,2,2);
	fill_row(g,24,5,5);
	fill_row(g,24,8,8);
	//Row 25
	fill_row(g,25,0,5);
	fill_row(g,25,8,11);
	//Row 26
	fill_row(g,26,0,0);
	fill_row(g,26,11,11);
	//Row 27
	fill_row(g,27,0,0);
	fill_row(g,27,11,11);
	//Row 28
	fill_row(g,28,0,12);

	char *dots = g->dots;

	//Row 0
	set_bits(dots,0,0,DIRC0|DIRC3);
	set_bits(dots,0,5,DIRC0|DIRC2|DIRC3);

	set_bits(dots,0,11,DIRC2|DIRC3);
	set_bits(dots,0,(DOTS_SX-1)-0,DIRC2|DIRC3);
	set_bits(dots,0,(DOTS_SX-1)-5,DIRC0|DIRC2|DIRC3);
	set_bits(dots,0,(DOTS_SX-1)-11,DIRC0|DIRC3);

	//Row 4
	set_bits(dots,4,0,DIRC0|DIRC1|DIRC3);
	set_bits(dots,4,5,DIRC0|DIRC1|DIRC2|DIRC3);
	set_bits(dots,4,8,DIRC0|DIRC2|DIRC3);
	set_bits(dots,4,11,DIRC0|DIRC1|DIRC2);
	set_bits(dots,4,(DOTS_SX-1)-0,DIRC2|DIRC1|DIRC3);
	set_bits(dots,4,(DOTS_SX-1)-5,DIRC0|DIRC1|DIRC2|DIRC3);
	set_bits(dots,4,(DOTS_SX-1)-8,DIRC0|DIRC2|DIRC3);
	set_bits(dots,4,(DOTS_SX-1)-11,DIRC0|DIRC1|DIRC2);

	//Row 7
	set_bits(dots,7,0,DIRC0|DIRC1);
	set_bits(dots,7,5,DIRC1|DIRC2|DIRC3);
	set_bits(dots,7,8,DIRC0|DIRC1);
	set_bits(dots,7,11,DIRC2|DIRC3);
	set_bits(dots,7,(DOTS_SX-1)-0,DIRC2|DIRC1);
	set_bits(dots,7,(DOTS_SX-1)-5,DIRC1|DIRC0|DIRC3);
	set_bits(dots,7,(DOTS_SX-1)-8,DIRC1|DIRC2);
	set_bits(dots,7,(DOTS_SX-1)-11,DIRC0|DIRC3);

	//Row 10
	set_bits(dots,10,8,DIRC0|DIRC3);
	set_bits(dots,10,11,DIRC0|DIRC1|DIRC2);
	set_bits(dots,10,(DOTS_SX-1)-8,DIRC2|DIRC3);
	set_bits(dots,10,(DOTS_SX-1)-11,DIRC0|DIRC1|DIRC2);

	//Row 13
	set_bits(dots,13,5,DIRC0|DIRC1|DIRC2|DIRC3);
	set_bits(dots,13,8,DIRC1|DIRC2|DIRC3);
	set_bits(dots,13,(DOTS_SX-1)-5,DIRC0|DIRC1|DIRC2|DIRC3);
	set_bits(dots,13,(DOTS_SX-1)-8,DIRC1|DIRC0|DIRC3);

	//Row 16
	set_bits(dots,16,8,DIRC0|DIRC1|DIRC3);
	set_bits(dots,16,(DOTS_SX-1)-8,DIRC2|DIRC1|DIRC3);
	
	//Row 19
	set_bits(dots,19,0,DIRC0|DIRC3);
	set_bits(dots,19,5,DIRC0|DIRC1|DIRC2|DIRC3);
	set_bits(dots,19,8,DIRC0|DIRC1|DIRC2);
	set_bits(dots,19,11,DIRC2|DIRC3);
	set_bits(dots,19,(DOTS_SX-1)-0,DIRC2|DIRC3);
	set_bits(dots,19,(DOTS_SX-1)-5,DIRC0|DIRC1|DIRC2|DIRC3);
	set_bits(dots,19,(DOTS_SX-1)-8,DIRC0|DIRC1|DIRC2);
	set_bits(dots,19,(DOTS_SX-1)-11,DIRC0|DIRC3);

	//Row 22
	set_bits(dots,22,0,DIRC0|DIRC1);
	set_bits(dots,22,2,DIRC2|DIRC3);
	set_bits(dots,22,5,DIRC0|DIRC1|DIRC3);
	set_bits(dots,22,8,DIRC0|DIRC2|DIRC3);
	set_bits(dots,22,11,DIRC0|DIRC1|DIRC2);
	set_bits(dots,22,(DOTS_SX-1)-0,DIRC2|DIRC1);
	set_bits(dots,22,(DOTS_SX-1)-2,DIRC0|DIRC3);
	set_bits(dots,22,(DOTS_SX-1)-5,DIRC2|DIRC1|DIRC3);
	set_bits(dots,22,(DOTS_SX-1)-8,DIRC0|DIRC2|DIRC3);
	set_bits(dots,22,(DOTS_SX-1)-11,DIRC0|DIRC1|DIRC2);

	//Row 25
	set_bits(dots,25,0,DIRC0|DIRC3);
	set_bits(dots,25,2,DIRC0|DIRC1|DIRC2);
	set_bits(dots,25,5,DIRC1|DIRC2);
	set_bits(dots,25,8,DIRC0|DIRC1);
	set_bits(dots,25,11,DIRC2|DIRC3);
	set_bits(dots,25,(DOTS_SX-1)-0,DIRC2|DIRC3);
	set_bits(dots,25,(DOTS_SX-1)-2,DIRC0|DIRC1|DIRC2);
	set_bits(dots,25,(DOTS_SX-1)-5,DIRC1|DIRC0);
	set_bits(dots,25,(DOTS_SX-1)-8,DIRC2|DIRC1);
	set_bits(dots,25,(DOTS_SX-1)-11,DIRC0|DIRC3);

	//Row 28
	set_bits(dots,28,0,DIRC0|DIRC1);
	set_bits(dots,28,11,DIRC0|DIRC1|DIRC2);

	set_bits(dots,28,(DOTS_SX-1)-0,DIRC2|DIRC1);
	set_bits(dots,28,(DOTS_SX-1)-11,DIRC0|DIRC1|DIRC2);
}

////////////////////////////////////////////////////////////////////////////////
void draw_dots(gamestate_t *g){

	uint8_t *dots = g->dots;

	int row, col;

	int ptr = 0;

	for (row = 0; row < DOTS_SY; row++){

		for (col = 0; col < DOTS_SX; col++){

			if (dots[ptr] & DOT){
				plotasm(MAPXOFFSET+DOTXOFFSET+col*8,DOTYOFFSET+row*8,8);
				plotasm(1+MAPXOFFSET+DOTXOFFSET+col*8,DOTYOFFSET+row*8,8);
				plotasm(MAPXOFFSET+DOTXOFFSET+col*8,1+DOTYOFFSET+row*8,8);
				plotasm(1+MAPXOFFSET+DOTXOFFSET+col*8,1+DOTYOFFSET+row*8,8);
			}
			else
			if (dots[ptr] & DOTPWR){
				draw_pwrup_on(g,row,col);
			}

#if 0
			if (dots[ptr]&DIRC0)
				plotasm(2+MAPXOFFSET+DOTXOFFSET+col*8,DOTYOFFSET+row*8,2);

			if (dots[ptr]&DIRC1)
				plotasm(MAPXOFFSET+DOTXOFFSET+col*8,DOTYOFFSET+row*8-1,2);

			if (dots[ptr]&DIRC2)
				plotasm(+MAPXOFFSET+DOTXOFFSET+col*8-1,DOTYOFFSET+row*8,2);

			if (dots[ptr]&DIRC3)
				plotasm(MAPXOFFSET+DOTXOFFSET+col*8,DOTYOFFSET+row*8+2,2);
#endif

			ptr++;
		}
	}
}

