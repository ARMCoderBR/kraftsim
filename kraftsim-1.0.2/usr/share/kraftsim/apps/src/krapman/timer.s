;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  KRAPMAN FOR KRAFT 80
;  A mini game inspired on Namco's PAC MAN
;  Rev 1.0
;  14-Oct-2025 - ARMCoder
;  Timer module
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

		.include "kraft80.inc"

		.module timer

		.globl	_init_timer, _get_timer, _set_timer

		.area	_CODE

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_init_timer:	di
		;;ld	(timer_srvector),hl
		ld	hl,#0
		ld	(timer_counter),hl
		ld	hl,(isr2vector_addr)
		ld	(isr2vector_copy),hl
		ld	hl,#timer_isr
		ld	(isr2vector_addr),hl
		ei
		ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_get_timer:	ld	de,(timer_counter)
		ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_set_timer:	ld	(timer_counter),hl
		ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

timer_isr:	;;ld	hl,#timer_isr2
		;;push	hl
		;;ld	hl,(timer_srvector)
		;;jp	(hl)
;;timer_isr2:

		ld	hl,(timer_counter)
		ld	a,h
		or	l
		jr	z,timer_isr_a
		dec	hl
		ld	(timer_counter),hl
timer_isr_a:
		ld	hl,(isr2vector_copy)
		jp	(hl)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

		.area _DATA

timer_counter:	.ds	2

;;timer_srvector:	.ds	2	; User service
isr2vector_copy:.ds	2	; Timer

