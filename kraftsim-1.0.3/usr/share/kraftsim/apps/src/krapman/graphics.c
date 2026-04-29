#include "graphics.h"
#include "graphres.h"
#include "game.h"

////////////////////////////////////////////////////////////////////////////////
void plotasm (int x, int y, char color) __naked __sdcccall(0) {

	__asm

		ld iy,#2
		add iy,sp
		ld l,2(iy)	;y
		ld h,3(iy)
		sla l		;160 = 128+32
		rl h
		sla l
		rl h
		sla l
		rl h
		sla l
		rl h
		sla l
		rl h

		ld c,l		;BC stores the origind hl * 32
		ld b,h

		sla l
		rl h
		sla l
		rl h

		add hl,bc	;HL is multiplied by 160

		ld c,(iy)	;x
		ld b,1(iy)
		srl b		;halves BC
		rr c

		add hl,bc
		ld a,l
		out (PORTADDRL),a
		ld a,h
		out (PORTADDRH),a

		in a,(PORTDATA)
		bit 0,(iy)
		jr z,plotasm_coleven

		;column is odd here

		and #0xf0
		ld b,a
		ld a,4(iy)
		and #0x0f
		or b
		out (PORTDATA),a
		ret

plotasm_coleven:
		and #0x0f
		ld b,a
		ld a,4(iy)
		sla a	
		sla a	
		sla a	
		sla a	
		or b
		out (PORTDATA),a
		ret

	__endasm;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void draw_sprite(int scrpos, const char *sprite) __naked __sdcccall(1){

	__asm

		; de = sprite  hl = scrpos
		ld	c,#16
printlb1:	ld	a,l
		out	(PORTADDRL),a
		ld	a,h
		out	(PORTADDRH),a

		ld	b,#8
printlb2:	ld	a,(de)
		cp	#0x55	// Magenta, not used -> transparency
		jr	nz,printlb2a

		inc	hl
		ld	a,l
		out	(PORTADDRL),a
		ld	a,h
		out	(PORTADDRH),a
		jr	printlb2b

printlb2a:	out	(PORTDATA),a
		inc	hl
printlb2b:	inc	de
		djnz	printlb2

		ld	a,l
		add	a,#BYTES_PER_LINE-8
		ld	l,a
		jr	nc,printlb3
		inc	h

printlb3:	dec	c
		jr	nz,printlb1

		ret

	__endasm;
}

////////////////////////////////////////////////////////////////////////////////
void read_sprite(int scrpos, char *sprite)  __naked __sdcccall(1){

	__asm

		; de = sprite save area  hl = scrpos
		ld	c,#16

readlb1:	ld	b,#8

readlb2:	ld	a,l
		out	(PORTADDRL),a
		ld	a,h
		out	(PORTADDRH),a
		inc	hl
		in	a,(PORTDATA)
		ld	(de),a
		inc	de
		djnz	readlb2

		ld	a,l
		add	a,#BYTES_PER_LINE-8
		ld	l,a
		jr	nc,readlb3
		inc	h

readlb3:	dec	c
		jr	nz,readlb1

		ret

	__endasm;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void restore_sprite_col0(int scrpos, char *sprite)  __naked __sdcccall(1){

	__asm

		; de = sprite save area  hl = scrpos

		ld	c,#16

restcol0_1:	ld	a,l
		out	(PORTADDRL),a
		ld	a,h
		out	(PORTADDRH),a
		ld	a,(de)
		out	(PORTDATA),a

		ld	a,e
		add	a,#8
		ld	e,a
		jr	nc,restcol0_2
		inc	d

restcol0_2:	ld	a,l
		add	a,#BYTES_PER_LINE
		ld	l,a
		jr	nc,restcol0_3
		inc	h

restcol0_3:	dec	c
		jr	nz,restcol0_1

		ret

	__endasm;
}

////////////////////////////////////////////////////////////////////////////////
void roll_spleft(char *sprite)  __naked __sdcccall(1){

	__asm

		; hl = sprite save area

		ld	d,h
		ld	e,l
		inc	hl
		ld	bc,#127
		ldir

		ret

	__endasm;
}

////////////////////////////////////////////////////////////////////////////////
void read_sprite_col7(int scrpos, char *sprite)  __naked __sdcccall(1){

	__asm

		; de = sprite save area  hl = scrpos

		ld	bc,#7
		add	hl,bc
		ex	de,hl
		add	hl,bc
		ex	de,hl

		ld	c,#16

readcol7_1:	ld	a,l
		out	(PORTADDRL),a
		ld	a,h
		out	(PORTADDRH),a
		in	a,(PORTDATA)
		ld	(de),a

		ld	a,e
		add	a,#8
		ld	e,a
		jr	nc,readcol7_2
		inc	d

readcol7_2:	ld	a,l
		add	a,#BYTES_PER_LINE
		ld	l,a
		jr	nc,readcol7_3
		inc	h

readcol7_3:	dec	c
		jr	nz,readcol7_1

		ret

	__endasm;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void restore_sprite_col7(int scrpos, char *sprite)  __naked __sdcccall(1){

	__asm

		; de = sprite save area  hl = scrpos

		ld	bc,#7
		add	hl,bc
		ex	de,hl
		add	hl,bc
		ex	de,hl

		ld	c,#16

restcol7_1:	ld	a,l
		out	(PORTADDRL),a
		ld	a,h
		out	(PORTADDRH),a
		ld	a,(de)
		out	(PORTDATA),a

		ld	a,e
		add	a,#8
		ld	e,a
		jr	nc,restcol7_2
		inc	d

restcol7_2:	ld	a,l
		add	a,#BYTES_PER_LINE
		ld	l,a
		jr	nc,restcol7_3
		inc	h

restcol7_3:	dec	c
		jr	nz,restcol7_1

		ret

	__endasm;
}

////////////////////////////////////////////////////////////////////////////////
void roll_spright(char *sprite)  __naked __sdcccall(1){

	__asm

		; hl = sprite save area
		ld	de,#126
		add	hl,de
		ld	d,h
		ld	e,l
		inc	de
		ld	bc,#127
		lddr

		ret

	__endasm;
}

////////////////////////////////////////////////////////////////////////////////
void read_sprite_col0(int scrpos, char *sprite)  __naked __sdcccall(1){

	__asm

		; de = sprite save area  hl = scrpos

		ld	c,#16

readcol0_1:	ld	a,l
		out	(PORTADDRL),a
		ld	a,h
		out	(PORTADDRH),a
		in	a,(PORTDATA)
		ld	(de),a

		ld	a,e
		add	a,#8
		ld	e,a
		jr	nc,readcol0_2
		inc	d

readcol0_2:	ld	a,l
		add	a,#BYTES_PER_LINE
		ld	l,a
		jr	nc,readcol0_3
		inc	h

readcol0_3:	dec	c
		jr	nz,readcol0_1

		ret

	__endasm;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void restore_sprite_row0(int scrpos, char *sprite)  __naked __sdcccall(1){

	__asm

		; de = sprite save area  hl = scrpos

		ld	b,#8

		ld	a,l
		out	(PORTADDRL),a
		ld	a,h
		out	(PORTADDRH),a
restrow0_1:	ld	a,(de)
		out	(PORTDATA),a
		inc	de
		djnz	restrow0_1

		ret

	__endasm;
}

////////////////////////////////////////////////////////////////////////////////
void roll_spup(char *sprite)  __naked __sdcccall(1){

	__asm

		; hl = sprite save area
		ld	d,h
		ld	e,l
		ld	bc,#8
		add	hl,bc
		ld	bc,#120
		ldir
		ret

	__endasm;
}

////////////////////////////////////////////////////////////////////////////////
void read_sprite_row15(int scrpos, char *sprite)  __naked __sdcccall(1){

	__asm

		; de = sprite save area  hl = scrpos

		ld	bc,#(15*BYTES_PER_LINE)
		add	hl,bc
		ex	de,hl
		ld	bc,#120
		add	hl,bc
		ex	de,hl

		ld	b,#8

readrow15_1:	ld	a,l
		out	(PORTADDRL),a
		ld	a,h
		out	(PORTADDRH),a
		in	a,(PORTDATA)
		ld	(de),a
		inc	hl
		inc	de
		djnz	readrow15_1

		ret

	__endasm;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void restore_sprite_row15(int scrpos, char *sprite)  __naked __sdcccall(1){

	__asm

		; de = sprite save area  hl = scrpos

		ld	bc,#(15*BYTES_PER_LINE)
		add	hl,bc
		ex	de,hl
		ld	bc,#120
		add	hl,bc
		ex	de,hl

		ld	b,#8

		ld	a,l
		out	(PORTADDRL),a
		ld	a,h
		out	(PORTADDRH),a
restrow15_1:	ld	a,(de)
		out	(PORTDATA),a
		inc	de
		djnz	restrow15_1

		ret

	__endasm;
}

////////////////////////////////////////////////////////////////////////////////
void roll_spdown(char *sprite)  __naked __sdcccall(1){

	__asm

		; hl = sprite save area
		ld	bc,#119
		add	hl,bc
		ld	d,h
		ld	e,l
		ld	bc,#8
		add	hl,bc
		ex	de,hl
		ld	bc,#120
		lddr
		ret

	__endasm;
}

////////////////////////////////////////////////////////////////////////////////
void read_sprite_row0(int scrpos, char *sprite)  __naked __sdcccall(1){

	__asm

		; de = sprite save area  hl = scrpos

		ld	b,#8

readrow0_1:	ld	a,l
		out	(PORTADDRL),a
		ld	a,h
		out	(PORTADDRH),a
		in	a,(PORTDATA)
		ld	(de),a
		inc	hl
		inc	de
		djnz	readrow0_1

		ret

	__endasm;
}

////////////////////////////////////////////////////////////////////////////////
void check_sprite_superpos_col7(gamestate_t *g, actor_t *act){

	uint8_t idnow = act->selfId;
	uint8_t colStart = act->Xph/2;
	uint8_t colEnd = colStart + 7;
	uint8_t rowStart = act->Y;
	uint8_t rowEnd = rowStart + 15;
	uint8_t id = idnow+1;

	if (id > LASTACTOR)
		id = 0;

	for (uint8_t i = 0; i < LASTACTOR; i++){

		uint8_t xph = g->actor[id].Xph/2;

		if ((g->actor[id].Y+15) < rowStart) goto next;
		if (g->actor[id].Y > rowEnd) goto next;
		if (xph > colEnd) goto next;
		if ((xph+7) < colEnd) goto next;

		uint8_t colSrc;
		uint8_t colDst;
		int diff_rows = rowStart - g->actor[id].Y;

		if (diff_rows > 0){
			colSrc = 7 - (xph - colStart)+8*diff_rows;
			colDst = 7;
		}
		else
		if (diff_rows < 0){
			diff_rows = -diff_rows;
			colSrc = 7 - (xph - colStart);
			colDst = 7+8*diff_rows;
		}
		else{	// Equal
			colSrc = 7 - (xph - colStart);
			colDst = 7;
		}

		uint8_t num_rows = 16 - diff_rows;
		for (int j = 0; j < num_rows; j++){

			act->bg[colDst] = g->actor[id].bg[colSrc];
			colDst += 8; colSrc += 8;
		}
next:
		id++;
		if (id > LASTACTOR)
			id = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////
void check_sprite_superpos_col0(gamestate_t *g, actor_t *act){

	uint8_t idnow = act->selfId;
	uint8_t colStart = act->Xph/2;
	uint8_t colEnd = colStart + 7;
	uint8_t rowStart = act->Y;
	uint8_t rowEnd = rowStart + 15;
	uint8_t id = idnow+1;

	if (id > LASTACTOR)
		id = 0;

	for (uint8_t i = 0; i < LASTACTOR; i++){

		uint8_t xph = g->actor[id].Xph/2;

		if ((g->actor[id].Y+15) < rowStart) goto next;
		if (g->actor[id].Y > rowEnd) goto next;
		if (xph > colStart) goto next;
		if ((xph+7) < colStart) goto next;

		uint8_t colSrc;
		uint8_t colDst;
		int diff_rows = rowStart - g->actor[id].Y;

		if (diff_rows > 0){
			colSrc = (colStart - xph)+8*diff_rows;
			colDst = 0;
		}
		else
		if (diff_rows < 0){
			diff_rows = -diff_rows;
			colSrc = colStart - xph;
			colDst = 8*diff_rows;
		}
		else{	// Equal
			colSrc = colStart - xph;
			colDst = 0;
		}

		uint8_t num_rows = 16 - diff_rows;
		for (int j = 0; j < num_rows; j++){

			act->bg[colDst] = g->actor[id].bg[colSrc];
			colDst += 8; colSrc += 8;
		}
next:
		id++;
		if (id > LASTACTOR)
			id = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////
void check_sprite_superpos_row0(gamestate_t *g, actor_t *act){

	uint8_t idnow = act->selfId;
	uint8_t colStart = act->Xph/2;
	uint8_t colEnd = colStart + 7;
	uint8_t rowStart = act->Y;
	uint8_t rowEnd = rowStart + 15;
	uint8_t id = idnow+1;

	if (id > LASTACTOR)
		id = 0;

	for (uint8_t i = 0; i < LASTACTOR; i++){

		uint8_t xph = g->actor[id].Xph/2;

		if ((g->actor[id].Y+15) < rowStart) goto next;
		if (g->actor[id].Y > rowStart) goto next;
		if (xph > colEnd) goto next;
		if ((xph+7) < colStart) goto next;

		uint8_t colSrc;
		uint8_t colDst;
		int diff_cols = colStart - xph;

		if (diff_cols > 0){
			colSrc = (rowStart - g->actor[id].Y)*8+diff_cols;
			colDst = 0;
		}
		else
		if (diff_cols < 0){
			diff_cols = -diff_cols;
			colSrc = (rowStart - g->actor[id].Y)*8;
			colDst = diff_cols;
		}
		else{	// Equal
			colSrc = (rowStart - g->actor[id].Y)*8;
			colDst = 0;
		}

		uint8_t num_cols = 8 - diff_cols;
		for (int j = 0; j < num_cols; j++){

			act->bg[colDst++] = g->actor[id].bg[colSrc++];
		}
next:
		id++;
		if (id > LASTACTOR)
			id = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////
void check_sprite_superpos_row15(gamestate_t *g, actor_t *act){

	uint8_t idnow = act->selfId;
	uint8_t colStart = act->Xph/2;
	uint8_t colEnd = colStart + 7;
	uint8_t rowStart = act->Y;
	uint8_t rowEnd = rowStart + 15;
	uint8_t id = idnow+1;

	if (id > LASTACTOR)
		id = 0;

	for (uint8_t i = 0; i < LASTACTOR; i++){

		uint8_t xph = g->actor[id].Xph/2;

		if ((g->actor[id].Y+15) < rowEnd) goto next;
		if (g->actor[id].Y > rowEnd) goto next;
		if (xph > colEnd) goto next;
		if ((xph+7) < colStart) goto next;

		uint8_t colSrc;
		uint8_t colDst;
		int diff_cols = colStart - xph;

		if (diff_cols > 0){
			colSrc = (15-(g->actor[id].Y - rowStart))*8+diff_cols;
			colDst = 15*8;
		}
		else
		if (diff_cols < 0){
			diff_cols = -diff_cols;
			colSrc = (15-(g->actor[id].Y - rowStart))*8;
			colDst = 15*8+diff_cols;
		}
		else{	// Equal
			colSrc = (15-(g->actor[id].Y - rowStart))*8;
			colDst = 15*8;
		}

		uint8_t num_cols = 8 - diff_cols;
		for (int j = 0; j < num_cols; j++){

			act->bg[colDst++] = g->actor[id].bg[colSrc++];
		}
next:
		id++;
		if (id > LASTACTOR)
			id = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void draw_actor(actor_t *act){

	uint8_t idx = act->spriteIdx;

	const uint8_t *sprite;

	if (act->selfId == 4){	// Pac Man

		switch(act->Dir){
		case 0:
			//switch ((act->Xph>>1) & 3){
			switch (act->Xph & 3){
				case 0:
					sprite = pac0e;
					break;
				case 2:
					sprite = pac2e;
					break;
				default:	//1 & 3
					sprite = pac13e;
					break;
			}
			break;

		case 1:
			//switch ((act->Y>>1) & 3){
			switch (act->Y & 3){
				case 0:
					sprite = pac0n;
					break;
				case 2:
					sprite = pac2n;
					break;
				default:	//1 & 3
					sprite = pac13n;
					break;
			}
			break;

		case 2:
			//switch ((act->Xph>>1) & 3){
			switch (act->Xph & 3){
				case 0:
					sprite = pac0w;
					break;
				case 2:
					sprite = pac2w;
					break;
				default:	//1 & 3
					sprite = pac13w;
					break;
			}
			break;

		case 3:
			//switch ((act->Y>>1) & 3){
			switch (act->Y & 3){
				case 0:
					sprite = pac0s;
					break;
				case 2:
					sprite = pac2s;
					break;
				default:	//1 & 3
					sprite = pac13s;
					break;
			}
			break;

		case 4:	// Not a direction, used for death sequence
			sprite = pacdie01;
			break;
		case 5:	// Not a direction, used for death sequence
			sprite = pacdie02;
			break;
		case 6:	// Not a direction, used for death sequence
			sprite = pacdie03;
			break;
		case 7:	// Not a direction, used for death sequence
			sprite = pacdie04;
			break;
		case 8:	// Not a direction, used for death sequence
			sprite = pacdie05;
			break;
		case 9:	// Not a direction, used for death sequence
			sprite = pacdie06;
			break;
		case 10:// Not a direction, used for death sequence
			sprite = pacdie07;
			break;
		case 11:// Not a direction, used for death sequence
			sprite = pacdie08;
			break;
		case 12:// Not a direction, used for death sequence
			sprite = pacdie09;
			break;
		case 13:// Not a direction, used for death sequence
			sprite = pacdie10;
			break;
		case 14:// Not a direction, used for death sequence
			sprite = pacdie11;
			break;
		//case 15:// Not a direction, used for death sequence
		default:
			sprite = pacdie12;
			break;
		}

		draw_sprite(act->Addr,sprite);

		return;
	}

	// Ghosts
	switch(act->Dir){
	case 0:
		if (act->Xph & 1)
			draw_sprite(act->Addr,ghostSpriteBase[idx]);
		else
			draw_sprite(act->Addr,ghostSpriteBase[idx+1]);
		break;

	case 1:
		draw_sprite(act->Addr,ghostSpriteBase[idx+4]);
		break;

	case 2:
		if (!(act->Xph & 1))
			draw_sprite(act->Addr,ghostSpriteBase[idx+2]);
		else
			draw_sprite(act->Addr,ghostSpriteBase[idx+3]);
		break;

	default:
		draw_sprite(act->Addr,ghostSpriteBase[idx+5]);
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////
void save_actor_bg(actor_t *act){

	read_sprite(act->Addr,act->bg);
}

////////////////////////////////////////////////////////////////////////////////
void restore_actor_bg(actor_t *act){

	draw_sprite(act->Addr,act->bg);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void draw_pwrup(int scrpos, const char *sprite) __naked __sdcccall(1){

	__asm

		; de = sprite  hl = scrpos
		ld	c,#8
printpw1:	ld	a,l
		out	(PORTADDRL),a
		ld	a,h
		out	(PORTADDRH),a

		ld	b,#4
printpw2:	ld	a,(de)
		out	(PORTDATA),a
		inc	de
		djnz	printpw2

		ld	a,l
		add	a,#BYTES_PER_LINE
		ld	l,a
		jr	nc,printpw3
		inc	h

printpw3:	dec	c
		jr	nz,printpw1

		ret

	__endasm;
}

////////////////////////////////////////////////////////////////////////////////
void draw_pwrup_off2(int scrpos) __naked __sdcccall(1){

	__asm

		; hl = scrpos
		ld	c,#8
printpwf1:	ld	a,l
		out	(PORTADDRL),a
		ld	a,h
		out	(PORTADDRH),a

		ld	b,#4
		xor	a

printpwf2:	out	(PORTDATA),a
		djnz	printpwf2

		ld	a,l
		add	a,#BYTES_PER_LINE
		ld	l,a
		jr	nc,printpwf3
		inc	h

printpwf3:	dec	c
		jr	nz,printpwf1

		ret

	__endasm;
}

////////////////////////////////////////////////////////////////////////////////
const uint8_t pwronsprite[] = {
//  . . # # # # . .
//  . # # # # # # .
//  # # # # # # # #
//  # # # # # # # #
//  # # # # # # # #
//  # # # # # # # #
//  . # # # # # # .
//  . . # # # # . .
	0x00, 0x88, 0x88, 0x00,
	0x08, 0x88, 0x88, 0x80,
	0x88, 0x88, 0x88, 0x88,
	0x88, 0x88, 0x88, 0x88,
	0x88, 0x88, 0x88, 0x88,
	0x88, 0x88, 0x88, 0x88,
	0x08, 0x88, 0x88, 0x80,
	0x00, 0x88, 0x88, 0x00
};

void draw_pwrup_on(gamestate_t *g,uint8_t row,uint8_t col){

	int addr = 0;

	if (row == 2){
		if (col == 0) addr = g->AddrPW0;
		else
		if (col == 25) addr = g->AddrPW1;
	}
	else
	if (row == 22){
		if (col == 0) addr = g->AddrPW2;
		else
		if (col == 25) addr = g->AddrPW3;
	}

	if (addr)
		draw_pwrup(addr, pwronsprite);
}

////////////////////////////////////////////////////////////////////////////////
void redraw_pwrup_on(gamestate_t *g){

	if (g->pw0)
		draw_pwrup(g->AddrPW0, pwronsprite);

	if (g->pw1)
		draw_pwrup(g->AddrPW1, pwronsprite);

	if (g->pw2)
		draw_pwrup(g->AddrPW2, pwronsprite);

	if (g->pw3)
		draw_pwrup(g->AddrPW3, pwronsprite);
}

////////////////////////////////////////////////////////////////////////////////
void redraw_pwrup_off(gamestate_t *g){

	if (g->pw0)
		draw_pwrup_off2(g->AddrPW0);

	if (g->pw1)
		draw_pwrup_off2(g->AddrPW1);

	if (g->pw2)
		draw_pwrup_off2(g->AddrPW2);

	if (g->pw3)
		draw_pwrup_off2(g->AddrPW3);
}

