;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  BLINKER FOR KRAFT 80
;  Rev 1.0
;  12-Nov-2025 - ARMCoder
;  
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

		.include "kraft80.inc"

		.area	_CODE

loop1:		inc	(hl)
		djnz	loop1
		out	(PORTLEDS),a
		inc	a
		jr	loop1
		
		.area	_DATA

