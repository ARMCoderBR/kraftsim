/*
IO-KRAFT80.C
Support routines for the KRAFT 80
2025 - ARM Coder
LCD control functions by Wagner Rambo - WR Kits - wrkits.com.br
*/

#include <stdio.h>
#include "io-kraft80.h"
#include "kraft80.h"

#pragma codeseg CODE

////////////////////////////////////////////////////////////////////////////////
char *lgets_noecho(char *buf, int bufsize){

    int i = 0;
    char a;
    for (;;){
    
        a = getchar();

        if (a == 0x0d){
            buf[i] = 0;
            return buf;
        }

        if (a == 0x08){
            if (i) --i;
            continue;
        }

        //if (i >= ' '){
            buf[i] = a;
            if (i < (bufsize-1))
                ++i;
        //}
    }
}

////////////////////////////////////////////////////////////////////////////////
char *lgets(char *buf, int bufsize){

    int i = 0;
    char a;
    for (;;){
    
        a = getchar();
        putchar(a);
        if (a == 0x0d){
            buf[i] = 0;
            return buf;
        }
        if (a == 0x08){
            if (i) --i;
            continue;
        }
        buf[i] = a;
        if (i < (bufsize-1))
            ++i;
    }
}

////////////////////////////////////////////////////////////////////////////////
void putstr(char *s){

    while(*s){
        putchar(*(s++));
    }
}
	
////////////////////////////////////////////////////////////////////////////////
void putstr_lcd(char *s){

    while(*s){
        putchar_lcd(*(s++));
    }
}

	
////////////////////////////////////////////////////////////////////////////////
int getchar() __naked{

    __asm

    rst #0x10
    jr z,_getchar
    ld e,a
    ld d,#0
    ret
    
    __endasm;
}
	
	
////////////////////////////////////////////////////////////////////////////////
int putchar (int a) __naked{

    __asm

    ld a,l
    cp #0x0a
    jr nz,_putchar1
    ld a,#0x0d
    rst #0x08
    ld a,#0x0a
_putchar1:
    rst #0x08
    ld l,#0
    ret
    
    __endasm;
}

////////////////////////////////////////////////////////////////////////////////
void setleds(char leds) __naked{

    __asm

    out(PORTLEDS),a
    ret
    
    __endasm;
}

////////////////////////////////////////////////////////////////////////////////
unsigned char readbuttons() __naked{

    __asm
    
    in a,(PORTBUTTONS)
    ld l,a
    ret
    
    __endasm;
}

////////////////////////////////////////////////////////////////////////////////
void putchar_lcd (char a) __sdcccall(1){
    __asm

	;RS R/W DB7 DB6 DB5 DB4
	;1   0   D7  D6  D5  D4
	;push	bc

	push	af
	ld	a,(_dispcol)
	cp	#16
	jr	nz,lcd_w1
	call	_lcd_home2
	jr	lcd_w2

lcd_w1:	cp	#32
	jr	nz,lcd_w2
	call	_lcd_home

lcd_w2:	ld	a,(_dispcol)
	inc	a
	ld	(_dispcol),a
	pop	af
	
	ld	c,a
	and	#0xf0
	or	#0x01
	call	lcd_out

	;RS R/W DB7 DB6 DB5 DB4
	;1   0   D3  D2  D1  D0
	ld	a,c		
	sla	a
	sla	a
	sla	a
	sla	a
	or	#0x01
	call	lcd_out

	call	delay_5ms

	;pop	bc

	;ret
    
    __endasm;
}

////////////////////////////////////////////////////////////////////////////////
void lcd_home() {

    __asm
    
	;RS R/W DB7 DB6 DB5 DB4
	;0   0   0   0   0   0
	ld	a,#0b00000000
	call	lcd_out
	;RS R/W DB3 DB2 DB1 DB0
	;0   0   0   0   1   0
	ld	a,#0b00100000
	call	lcd_out

	call	delay_5ms
	xor a
	ld	(_dispcol),a
	;ret

    __endasm;
}

////////////////////////////////////////////////////////////////////////////////
void lcd_home2() {

    __asm

	;RS R/W DB7 DB6 DB5 DB4
	;0   0   1   1   0   0
	ld	a,#0b11000000
	call	lcd_out
	;RS R/W DB3 DB2 DB1 DB0
	;0   0   0   0   0   0
	ld	a,#0b00000000
	call	lcd_out

	call	delay_5ms
	ld	a,#16
	ld	(_dispcol),a
	;ret

    __endasm;
}

////////////////////////////////////////////////////////////////////////////////
void lcd_clear() {

    __asm

	;RS R/W DB7 DB6 DB5 DB4
	;0   0   0   0   0   0
	ld	a,#0b00000000
	call	lcd_out
	;RS R/W DB3 DB2 DB1 DB0
	;0   0   0   0   0   1
	ld	a,#0b00010000
	call	lcd_out

	call	delay_5ms
	xor a
	ld	(_dispcol),a
	;ret

    __endasm;
}

////////////////////////////////////////////////////////////////////////////////
void lcd_begin() {

    __asm
    
	call	delay_15ms

	;RS R/W DB7 DB6 DB5 DB4
	;0   0   0   0   1   1
	ld	a,#0b00110000
	call	lcd_out
	call	delay_5ms
	ld	a,#0b00110000
	call	lcd_out
	call	delay_5ms

	;RS R/W DB7 DB6 DB5 DB4
	;0   0   0   0   1   0
	ld	a,#0b00100000		; Set 4 bit mode
	call	lcd_out

	call	delay_5ms

	;RS R/W DB7 DB6 DB5 DB4
	;0   0   0   0   1   0
	ld	a,#0b00100000		; Will set N F
	call	lcd_out
	;RS R/W DB3 DB2 DB1 DB0
	;0   0   N   F   x   x  N=1 F=1
	ld	a,#0b11000000
	call	lcd_out

	call	delay_5ms

	;RS R/W DB7 DB6 DB5 DB4
	;0   0   0   0   0   0
	ld	a,#0b00000000		; Will turn display on
	call	lcd_out
	;RS R/W DB3 DB2 DB1 DB0
	;0   0   1   1   0   0
	ld	a,#0b11000000
	call	lcd_out

	call	delay_5ms

	;RS R/W DB7 DB6 DB5 DB4
	;0   0   0   0   0   0
	ld	a,#0b00000000		; Will clear display
	call	lcd_out
	;RS R/W DB3 DB2 DB1 DB0
	;0   0   0   0   0   1
	ld	a,#0b00010000
	call	lcd_out

	call	delay_5ms

	;RS R/W DB7 DB6 DB5 DB4
	;0   0   0   0   0   0
	ld	a,#0b00000000		; Will set Increment mode
	call	lcd_out		; No shift
	;RS R/W DB3 DB2 DB1 DB0
	;0   0   0   1   1   0
	ld	a,#0b01100000
	call	lcd_out

	call	delay_5ms

	call	_lcd_home

    __endasm;
}

////////////////////////////////////////////////////////////////////////////////

void lcd_out() __naked {

    __asm

lcd_out:
	out	(PORTDISP),a
	nop
	nop
	set	1,a
	out	(PORTDISP),a
	nop
	nop
	res	1,a
	out	(PORTDISP),a
	ret

	;///////////////////////////////////////////////////////////////

delay_5ms:
	ld	bc,#768		; 2.5us

delay_5ms_a:
	dec	bc		; 1.5 us
	ld	a,b		; 1 us
	or	c		; 1 us
	jr	nz,delay_5ms_a	; 3 us
	ret			; 2.5 us

	;///////////////////////////////////////////////////////////////

delay_15ms:
	ld	bc,#2307	; 2.5us

delay_15ms_a:
	dec	bc		; 1.5 us
	ld	a,b		; 1 us
	or	c		; 1 us
	jr	nz,delay_15ms_a	; 3 us
	ret			; 2.5 us

    __endasm;
}

uint8_t dispcol;
int pos;

////////////////////////////////////////////////////////////////////////////////
void video_setpos(int row, int col){

  // Video is 160 bytes wide (320 pixels) by 200 rows
  // Each byte stores 2 pixels, the 4 MSBs store the "left" pixel color and the 4 LSBs store the "right" pixel.
  
  pos = 160*row + col;
  
  __asm

  di
  ld hl,(_pos)
  ld a,l
  out (PORTADDRL),a
  ld a,h
  out (PORTADDRH),a
  ei
  
  __endasm;
}

////////////////////////////////////////////////////////////////////////////////
void video_out(unsigned char b){

  // Video is 160 bytes wide (320 pixels) by 200 rows
  // Each byte stores 2 pixels, the 4 MSBs store the "left" pixel color and the 4 LSBs store the "right" pixel.

  __asm

  out (PORTDATA),a

  __endasm;
}

////////////////////////////////////////////////////////////////////////////////
int video_in(void){

  // Video is 160 bytes wide (320 pixels) by 200 rows
  // Each byte stores 2 pixels, the 4 MSBs store the "left" pixel color and the 4 LSBs store the "right" pixel.

  __asm

    in a,(PORTDATA)
    ld e,a
    ld d,#0
    ret

  __endasm;
}

////////////////////////////////////////////////////////////////////////////////
void video_begin(int mode){

  int row = 0;
  int col;
  
  pos = mode;
  __asm

  ld hl,(_pos)
  ld a,l
  out (PORTMODE),a

  xor a
  out (PORTADDRL),a
  out (PORTADDRH),a

  ld c,#240
nextrow:
  ld b,#160
fillrow:
  out(0x50),a
  djnz fillrow
  dec c
  jr nz,nextrow

  __endasm;
}

////////////////////////////////////////////////////////////////////////////////
int serial_getchar() __naked{

    __asm

    ld c,#3
    rst #0x20
    jr z,_serial_getchar
    ld e,a
    ld d,#0
    ret

    __endasm;
}

////////////////////////////////////////////////////////////////////////////////
int serial_kbhit() __naked{

    __asm

    ld c,#2
    rst #0x20
    ld d,#0
    ld e,#0
    ret z
    inc e
    ret

    __endasm;
}

////////////////////////////////////////////////////////////////////////////////
int serial_putchar (int a) __naked{

    __asm

    ld a,l
    ld c,#1
    rst #0x20
    ld l,#0
    ret

    __endasm;
}

FILE filedata[3];

////////////////////////////////////////////////////////////////////////////////
FILE * fopen(char *name, char *mode) __naked __sdcccall(1){

    __asm

    push hl

    ld b,#3
    ld hl,#_filedata
fopen0:
    ld a,(hl)
    or a
    jr z,fopen1
    inc	hl	; As much as sizeof(FILE) - 1x for now
    djnz fopen0
    pop hl
fopen_err:
    ld de,#0
    ret

fopen1:
    push hl
    pop ix

    pop hl

    ld	bc,#0x000E	;B=0 C=14 -> open file
    push ix
    rst #0x20		;HL=name DE=mode
    pop de
    jr c,fopen_err

    ld (de),a
    ret

    __endasm;
}

////////////////////////////////////////////////////////////////////////////////
void fclose (FILE *f) __naked __sdcccall(1){

}

