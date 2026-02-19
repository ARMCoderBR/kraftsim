;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  INVADERS FOR KRAFT 80
;  A mini game inspired on Taito's Space Invaders
;  Rev 1.0
;  12-Feb-2026 - ARMCoder
;  Sound module
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

		.include "defines.h"

		.globl	init_sound
		.globl	sound_run
		.globl	sound_missile
		.globl	sound_move
		.globl	sound_kill
		
		.area	CODE

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;		

init_sound:	xor	a
		ld	(sound_presc),a
		ld	(sound_state),a
		ld	(soundtmr1),a
		jp	end_sound

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;		

end_sound:	ld	a,#7			;Disable all
		out	(PORTAYADDR),a
		ld	a,#0b00111111
		out	(PORTAYDATA),a
		ld	(r7copy),a
		ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;		

sound_run:	ld	a,(sound_presc)		;Divides by 10 (9+1) -> 30 Hz
		or	a
		jr	z,sound_run1
		dec	a
		ld	(sound_presc),a
		ret

sound_run1:	ld	a,#9
		ld	(sound_presc),a

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

		ld	a,(sound_state)
		bit	0,a			;Missile fired
		jr	z,sound_run2

		xor	a			;Chan A Tone Period
		out	(PORTAYADDR),a
		ld	a,#50
		out	(PORTAYDATA),a
		ld	a,#1
		out	(PORTAYADDR),a
		xor	a
		out	(PORTAYDATA),a

		ld	a,#6			;Noise Period
		out	(PORTAYADDR),a
		ld	a,#2
		out	(PORTAYDATA),a

		ld	a,#7			;Enable Channel A (Tone+Noise)
		out	(PORTAYADDR),a
		;ld	a,#0b00110110
		ld	a,(r7copy)
		and	#0b11110110
		out	(PORTAYDATA),a
		ld	(r7copy),a

		ld	a,#11			;Envelope Period
		out	(PORTAYADDR),a
		ld	a,#1
		out	(PORTAYDATA),a
		ld	a,#12
		out	(PORTAYADDR),a
		ld	a,#1
		out	(PORTAYDATA),a

		ld	a,#8			;Chan A Amp
		out	(PORTAYADDR),a
		ld	a,#0b00010000
		out	(PORTAYDATA),a

		ld	a,#13			;Envelope Shape
		out	(PORTAYADDR),a
		xor	a
		out	(PORTAYDATA),a

		ld	a,(sound_state)
		res	0,a
		ld	(sound_state),a

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

sound_run2:	bit	1,a			;Invaders moving
		jr	z,sound_run3

		xor	#0x04
		ld	(sound_state),a
		bit	2,a
		jr	z,sound_run2a

		ld	a,#2			;Chan B Tone Period 1
		out	(PORTAYADDR),a
		ld	a,#0x40
		out	(PORTAYDATA),a
		ld	a,#0x03
		out	(PORTAYADDR),a
		ld	a,#0x06
		out	(PORTAYDATA),a
		jr	sound_run2b

sound_run2a:	ld	a,#2			;Chan B Tone Period 2
		out	(PORTAYADDR),a
		ld	a,#0xd0
		out	(PORTAYDATA),a
		ld	a,#0x03
		out	(PORTAYADDR),a
		ld	a,#0x07
		out	(PORTAYDATA),a

sound_run2b:	ld	a,#9			;Chan B Amp
		out	(PORTAYADDR),a
		ld	a,#0b00001111
		out	(PORTAYDATA),a

		ld	a,#7			;Enable Channel B
		out	(PORTAYADDR),a
		ld	a,(r7copy)
		and	#0b11111101
		out	(PORTAYDATA),a
		ld	(r7copy),a

		ld	a,#3
		ld	(soundtmr1),a

		ld	a,(sound_state)
		res	1,a
		ld	(sound_state),a

sound_run3:
		ld	a,(soundtmr1)
		or	a
		jr	z,sound_run4

		dec	a
		ld	(soundtmr1),a
		or	a
		jr	nz,sound_run4

		ld	a,#7			;Disable Channel B
		out	(PORTAYADDR),a
		ld	a,(r7copy)
		or	#0b00000010
		out	(PORTAYDATA),a
		ld	(r7copy),a

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

sound_run4:	ld	a,(sound_state)		;Invader killed
		bit	3,a
		jr	z,sound_run5

		ld	hl,#40
		ld	(soundtonec),hl
		
		ld	a,#4			;Chan C Tone Period
		out	(PORTAYADDR),a
		ld	a,l		
		out	(PORTAYDATA),a
		ld	a,#5
		out	(PORTAYADDR),a
		ld	a,h		
		out	(PORTAYDATA),a

		ld	a,#10			;Chan C Amp
		out	(PORTAYADDR),a
		ld	a,#0b00001111	
		out	(PORTAYDATA),a

		ld	a,#7			;Enable Channel B
		out	(PORTAYADDR),a
		ld	a,(r7copy)
		and	#0b11111011
		out	(PORTAYDATA),a
		ld	(r7copy),a

		ld	a,#8
		ld	(soundtmr2),a

		ld	a,(sound_state)
		res	3,a
		ld	(sound_state),a
		jr	sound_run7

sound_run5:
		ld	a,(soundtmr2)
		or	a
		jr	z,sound_run7

		dec	a
		ld	(soundtmr2),a
		or	a
		jr	nz,sound_run6

		ld	a,#7			;Disable Channel B
		out	(PORTAYADDR),a
		ld	a,(r7copy)
		or	#0b00000100
		out	(PORTAYDATA),a
		ld	(r7copy),a
		jr	sound_run7

sound_run6:	ld	hl,(soundtonec)
		ld	a,#40
		add	a,l
		ld	l,a
		ld	a,#0
		adc	a,h
		ld	h,a
		ld	(soundtonec),hl
		ld	a,#4			;Chan C Tone Period
		out	(PORTAYADDR),a
		ld	a,l		
		out	(PORTAYDATA),a
		ld	a,#5
		out	(PORTAYADDR),a
		ld	a,h		
		out	(PORTAYDATA),a

sound_run7:
		ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;		

sound_missile:
		ld	a,(sound_state)
		set	0,a
		ld	(sound_state),a
		ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;		

sound_move:	ld	a,(sound_state)
		set	1,a
		ld	(sound_state),a
		ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;		

sound_kill:	push	af
		ld	a,(sound_state)
		set	3,a
		ld	(sound_state),a
		pop	af
		ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;		

		.area _DATA

r7copy:		.ds	1
sound_presc:	.ds	1
sound_state:	.ds	1
soundtmr1:	.ds	1
soundtmr2:	.ds	1
soundtonec:	.ds	2

