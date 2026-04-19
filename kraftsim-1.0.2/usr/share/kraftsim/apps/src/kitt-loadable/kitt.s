;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  KITT FOR KRAFT 80
;  Rev 1.0
;  12-Nov-2025 - ARMCoder
;  
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

		.include "kraft80.inc"

		.area	_CODE

BASE_DATA	.equ	0x4600

		ld	bc,#0x0101

loop1:		ld	hl,#0x4000
loop2:		dec	hl
		ld	a,h
		or	l
		jr	nz,loop2
		ld	a,c
		out	(PORTLEDS),a
		bit	0,b
		jr	z,loop2a
		sla	a
		jr	loop2b
loop2a:
		srl	a
loop2b:
		ld	c,a
		and	#0x81
		jr	z,loop1
		inc	b
		jr	loop1
		
		.area	_DATA


