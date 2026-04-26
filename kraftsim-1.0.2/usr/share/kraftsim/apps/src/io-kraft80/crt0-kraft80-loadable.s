;--------------------------------------------------------------------------
;  crt0.s - Generic crt0.s for a Z80
;
;  Copyright (C) 2000, Michael Hope
;
;  This library is free software; you can redistribute it and/or modify it
;  under the terms of the GNU General Public License as published by the
;  Free Software Foundation; either version 2, or (at your option) any
;  later version.
;
;  This library is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License 
;  along with this library; see the file COPYING. If not, write to the
;  Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
;   MA 02110-1301, USA.
;
;  As a special exception, if you link this library with other files,
;  some of which are compiled with SDCC, to produce an executable,
;  this library does not by itself cause the resulting executable to
;  be covered by the GNU General Public License. This exception does
;  not however invalidate any other reasons why the executable file
;  might be covered by the GNU General Public License.
;--------------------------------------------------------------------------

		.module crt0

CMD_BUFSZ	.equ	96
cmd_buf		.equ	0xF813
cmd_maxargs	.equ	20

		.area	_CODE

	        call	gsinit

		ld	a,#1
		ld	(aargc),a

		ld	hl,#cmd_buf
		ld	(aargv),hl
		ld	de,#aargv+2

loopargs:	ld	a,(hl)
		or	a
		jr	z,endargs
		cp 	#' '
		jr	z,end1arg

		inc	hl
		jr	loopargs

end1arg:	xor	a
		ld	(hl),a
		inc	hl
		ld	a,(hl)
		or	a
		jr	z,endargs
		cp 	#' '
		jr	z,end1arg

		ld	a,(aargc)
		cp	#cmd_maxargs
		jr	z,endargs
		inc	a
		ld	(aargc),a
		ld	a,l
		ld	(de),a
		inc	de
		ld	a,h
		ld	(de),a
		inc	de
		jr	loopargs

endargs:	ld	hl,#aargv
		push	hl
		ld	a,(aargc)
		ld	l,a
		xor	a
		ld	h,a
		push	hl
	
		call	_main

		pop	hl
		pop	hl
		ret

		;///////////////////////////////////////////////////////////////

gsinit:		ld	bc, #l__INITIALIZER
		ld	a, b
		or	a, c
		jr	Z, gsinit_next
		ld	de, #s__INITIALIZED
		ld	hl, #s__INITIALIZER
		ldir
gsinit_next:	ret

		.area	DATA
aargc:		.ds	1
aargv:		.ds	2*cmd_maxargs

		;; Ordering of segments for the linker.
		.area	_HOME
		.area	_CODE
		.area	_INITIALIZER
		.area   _GSINIT
		.area   _GSFINAL
        	.area   _MAIN
		.area	_DATA
		.area	_INITIALIZED
		.area	_BSEG
		.area   _BSS
		.area   _HEAP
		.area   _GSFINAL

