;--------------------------------------------------------------------------
;  kraftmon.s - Simple monior for the Kraft 80
;
;  Copyright (C) 2025,26 ARMCoder
;--------------------------------------------------------------------------

		.include "kraft80.inc"

SOH 		.equ	1
EOT		.equ	4
ACK		.equ	6
CR		.equ	13
NAK		.equ	21
ESC		.equ	27

;///////////////////////////////////////////////////////////////////////
;/////////////////////////   KRAFTMON MONITOR   ////////////////////////
;///////////////////////////////////////////////////////////////////////

		.module kraftmon

		.globl	kraftmon
		.globl	_sysm_crlf

		.area	_DATA

SYSM_BUFPTR:	.ds	1
SYSM_LASTDM:	.ds	2
SYSM_LASTED:	.ds	2
SYSM_FLAGS:	.ds	1

		.area	_CODE

;signon_kraftmon:.ascii	'\r\nKraftmon by ARMCoder\r\n\n\0'

kraftmon:	;ld	hl,#signon_kraftmon
		;call	_prints

		call	ch376_init_system

		ld	hl,#PROG_AREA
		ld	(SYSM_LASTDM),hl
		ld	(SYSM_LASTED),hl

		xor	a
		ld	(SYSM_FLAGS),a
		in	a,(PORTBUTTONS)
		bit	6,a
		jr	nz,kraftmon_loop

		ld	a,#1
		ld	(SYSM_FLAGS),a
	
		ld	c,#9
		rst	0x20
		ld	c,#13
		ld	hl,#msg_ld_xmodem
		rst	0x20
		jp	load_xmodem
	
msg_ld_xmodem:	.ascii	"Loading XMODEM.\0"

kraftmon_loop:	xor	a
		ld	(SYSM_BUFPTR),a

		call sysm_prompt

		jr	kraftmon_loop

		;///////////////////////////////////////////////////////////////

tabcmds:	.ascii	"loadx\0"
		.ascii	"g\0"
		.ascii	"d\0"
		.ascii	"e\0"
		.ascii	"dir\0"
		.ascii	"load\0"
		.ascii	"\0"

tabcmdaddrs:	.word	load_xmodem
		.word	sysm_go
		.word	sysm_dump
		.word	sysm_edit
		.word	_ch376_listdir
		.word	loadfile

		;///////////////////////////////////////////////////////////////

sysm_prompt:	ld	a,#':'
		rst	0x08
		call	sysm_readline
		call	_sysm_crlf

		ld	a,(cmd_buf)
		or	a
		ret	z

		ld	hl,#tabcmds
		ld	bc,#tabcmdaddrs

nexttok:	ld	a,(hl)
		or	a
		jr	z,tryload

		ld	de,#cmd_buf
		push	hl
		push	bc
		call	strcompare
		pop	bc
		pop	hl
		jr	z,exectok

		inc	bc
		inc	bc

nexttok2:	ld	a,(hl)
		or	a
		jr	z,nexttok1
		inc	hl
		jr	nexttok2

nexttok1:	inc	hl
		jr	nexttok

exectok:	ld	a,(bc)
		ld	l,a
		inc	bc
		ld	a,(bc)
		ld	h,a
		jp	(hl)

		;--------------------- Will try load file from disk

tryload:	ld	hl,#filename+12
		ld	(hl),#0
		ld	hl,#cmd_buf
		ld	de,#filename
		ld	bc,#13
		ldir
		ld	hl,#filename+1
		ld	b,#8
tryload1:	ld	a,(hl)
		or	a
		jr	z,tryload2
		cp	#' '
		jr	z,tryload2
		cp	#'.'
		jr	z,tryload3
		inc	hl
		djnz	tryload1
		ret

tryload2:	ld	(hl),#'.'
		inc	hl
		ld	(hl),#'c'
		inc	hl
		ld	(hl),#'o'
		inc	hl
		ld	(hl),#'m'
		inc	hl
		ld	(hl),#0
		inc	hl
		jr	tryload4

tryload3:	ld	b,#3
tryload3a:	inc	hl
		ld	a,(hl)
		or	a
		jr	z,tryload4
		cp	#' '
		jr	z,tryload3b
		djnz	tryload3a

tryload3b:	ld	(hl),#0

tryload4:	ld	hl,#PROG_AREA
		push	hl
		ld	hl,#filename
		push	hl
		call	_ch376_dumpfile
		bit	7,l
		jr	nz,loadfileerr
		pop	hl
		pop	hl

		call	#PROG_AREA
		jp	_sysm_crlf

		;///////////////////////////////////////////////////////////////

loadfile:	ld	hl,#PROG_AREA
		push	hl
		ld	hl,#cmd_buf+5
		push	hl

		call	_ch376_dumpfile
		bit	7,l
		jr	z,loadfile1

;		ld	a,l
;		cp	#-1
;		jr	z,loadfnf
;		cp	#-2
;		jr	z,loadfrder
;
loadfileerr:
		ld	hl,#msgloaderr
loadfile2:	call	_prints
;		jr	loadfile1
;
;loadfnf:	ld	hl,#msgloadfnf
;		jr	loadfile2
;
;loadfrder:	ld	hl,#msgloadfrder
;		jr	loadfile2

loadfile1:	pop	hl
		pop	hl
		ret

msgloaderr:	.ascii	"Load Error\r\n\0"
;msgloadfnf:	.ascii	"File not Found\r\n\0"
;msgloadfrder:	.ascii	"Read Error\r\n\0"

		;///////////////////////////////////////////////////////////////

str_go:		.ascii	'Go!'
		.byte	13,10,0

sysm_go:	ld	hl,#str_go
		call	_prints
		ld	hl,#sysm_goret
		push	hl
		ld	hl,#cmd_buf+1
		call	skip_spc
		call	parsehex16
		ld	hl,#PROG_AREA
		jr	z,sysm_go2
		ld	h,d
		ld	l,e

sysm_go2:	jp	(hl)

sysm_goret:	jp	_sysm_crlf

		;///////////////////////////////////////////////////////////////

sysm_edit:	ld	hl,#cmd_buf+1
		call	skip_spc
		call	parsehex16
		jr	z,sysm_editloop
		ld	h,d
		ld	l,e
		ld	(SYSM_LASTED),hl

sysm_editloop:	ld	hl,(SYSM_LASTED)
		ld	a,h
		call	sysm_printh8	
		ld	a,l
		call	sysm_printh8	
		ld	a,#':'
		rst	0x08

		ld	c,#0

		push	bc
		call	sysm_readline
		call	_sysm_crlf
		pop	bc

		ld	hl,#cmd_buf

sysm_editloop2:	call	skip_spc
		call	parsehex8
		jr	z,sysm_edit2

		push	hl
		ld	hl,(SYSM_LASTED)
		ld	(hl),e
		inc	hl
		ld	(SYSM_LASTED),hl
		pop	hl
		inc	c
		jr	sysm_editloop2

sysm_edit2:	ld	a,c
		or	a
		jr	nz,sysm_editloop
		ret
	
		;///////////////////////////////////////////////////////////////

dumphdr:	.ascii	'      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\r\n\0'

sysm_dump:	ld	hl,#cmd_buf+1
		call	skip_spc
		call	parsehex16
		jr	z,sysm_dump0
		ld	h,d
		ld	l,e
		ld	(SYSM_LASTDM),hl

sysm_dump0:	ld	hl,#dumphdr
		call	_prints
		ld	b,#8

sysm_dump1:	ld	hl,(SYSM_LASTDM)
		ld	a,h
		call	sysm_printh8	
		ld	a,l
		call	sysm_printh8	
		ld	a,#':'
		rst	0x08

		ld	c,#16

sysm_dump2:	ld	a,(hl)
		call	sysm_printh8	
		ld	a,#' '
		rst	0x08
		inc	hl
		dec	c
		jr	nz,sysm_dump2

		call	_sysm_crlf

		ld	hl,(SYSM_LASTDM)
		ld	de,#16
		add	hl,de
		ld	(SYSM_LASTDM),hl
	
		djnz	sysm_dump1
	
		ret

		;///////////////////////////////////////////////////////////////

skip_spc:	ld	a,(hl)
		cp	#' '
		ret	nz
		inc	hl
		jr	skip_spc

		;///////////////////////////////////////////////////////////////
		; Buffer in HL
		; returns result in E and flag NZ if valid, or flag Z if not valid
parsehex8:	ld	a,(hl)
		call	parsehexdigit
		ret	z
		ld	e,a

		inc	hl
		ld	a,(hl)
		call	parsehexdigit
		jr	z,parsehex8_1
		inc	hl
		sla	e
		sla	e
		sla	e
		sla	e
		or	e
		ld	e,a

parsehex8_1:	xor	a
		cp	#0xff
		ret

		;///////////////////////////////////////////////////////////////
		; Buffer in HL
		; returns result in DE and flag NZ if valid, or flag Z if not valid
parsehex16:	ld	de,#0
		ld	a,(hl)
		call	parsehexdigit
		ret	z
		ld	e,a

		ld	b,#3

parsehex16_2:	inc	hl
		ld	a,(hl)
		call	parsehexdigit
		jr	z,parsehex16_1
		sla	e
		rl	d
		sla	e
		rl	d
		sla	e
		rl	d
		sla	e
		rl	d
		or	e
		ld	e,a
		djnz	parsehex16_2

parsehex16_1:	xor	a
		cp	#0xff
		ret

		;///////////////////////////////////////////////////////////////
		; Digit in A
		; returns nibble in A and flag NZ if valid, or flag Z if not valid
parsehexdigit:	cp	#'0'
		jr	c,parsehexdigit1

		cp	#'9'+1
		jr	c,parsehexdigit2

		res	5,a
		cp	#'F'+1
		jr	nc,parsehexdigit1

		cp	#'A'
		jr	nc,parsehexdigit3

parsehexdigit1:	xor	a
		ret

parsehexdigit2:	sub	#'0'
		cp	#0xff
		ret

parsehexdigit3:	sub	#('A' - 10)
		cp	#0xff
		ret

		;///////////////////////////////////////////////////////////////
		; STR1 in HL, STR2 in DE
		; Returns Z if equal, NZ and C if STR1<STR2, NZ and NC if STR1>STR2
strcompare:	ld	a,(de)
		cp	#' '
		jr	nz,strcomp0
		xor	a

strcomp0:	ld	c,a
		ld	a,(hl)
		cp	c
		jr	z,strcomp1
		ret	

strcomp1:	or	a
		ret	z

		inc	hl
		inc	de
		jr	strcompare

		;///////////////////////////////////////////////////////////////
		; Value in ACC
sysm_printh8:	push	bc
		ld	b,a
		srl	a
		srl	a
		srl	a
		srl	a
		cp	#10
		jr	nc,sysm_ph8a
		add	a,#'0'
		jr	sysm_ph8b

sysm_ph8a:	add	a,#'A'-10

sysm_ph8b:	rst	0x08

		ld	a,b
		and	#0x0f
		cp	#10
		jr	nc,sysm_ph8c
		add	a,#'0'
		jr	sysm_ph8d

sysm_ph8c:	add	a,#'A'-10

sysm_ph8d:	rst	0x08
		pop	bc
		ret

		;///////////////////////////////////////////////////////////////

_sysm_crlf:	ld	a,#0x0d
		rst	0x08
		ld	a,#0x0a
		rst	0x08
		ret

		;///////////////////////////////////////////////////////////////

sysm_readline:	call	ch376_run
		rst	0x18
		jr	z,sysm_readline

		rst	0x10
		cp	#0x0d
		jr	z,sysm_rdl_cr
		cp	#0x08
		jr	z,sysm_rdl_bs

		ld	b,a
		ld	a,(SYSM_BUFPTR)
		cp	#CMD_BUFSZ
		jr	z,sysm_readline

		ld	c,a
		ld	a,b
		ld	b,#0
		ld	hl,#cmd_buf
		add	hl,bc
		ld	(hl),a

		rst	0x08
		inc	c
		ld	a,c
		ld	(SYSM_BUFPTR),a
		jr	sysm_readline

sysm_rdl_cr:	ld	a,(SYSM_BUFPTR)
		ld	c,a
		ld	b,#0
		ld	hl,#cmd_buf
		add	hl,bc
		xor	a
		ld	(hl),a
		ld	(SYSM_BUFPTR),a
		ret

sysm_rdl_bs:	ld	a,(SYSM_BUFPTR)
		or	a
		jr	z,sysm_readline
		dec	a
		ld	(SYSM_BUFPTR),a

		ld	a,#8
		rst	0x08
		ld	a,#' '
		rst	0x08
		ld	a,#8
		rst	0x08
		jr	sysm_readline

		;///////////////////////////////////////////////////////////////

str_load_xmodem:.ascii '\r\nSend file via XMODEM, [ENTER] abort\r\n\0'

load_xmodem:	ld	hl,#str_load_xmodem
		call	_prints
		ld	d,#1	;lastseq
		ld	hl,#PROG_AREA
		ld	(SYSM_LASTED),hl
		ld	bc,#30000

loop_xmodem:	push	bc
		ld	c,#2	; Test Char
		rst	0x20
		pop	bc
		jr	z,load_xmodem_nochar

		call	serial_getchar

		cp	#EOT
		jr	nz,load_xmodem1

		ld	a,#ACK
		ld	c,#1	; TX Char
		rst	0x20

		ld	a,(SYSM_FLAGS)
		bit	0,a
		ret	z

		ld	c,#9
		rst	0x20
		ld	c,#13
		ld	hl,#msg_run
		rst	0x20
		jp	PROG_AREA

msg_run:	.ascii	"Run...\0"

load_xmodem1:	cp	#ESC
		ret	z
		cp	#CR
		ret	z

		cp	#SOH
		jr	nz,loop_xmodem

		call	serial_getchar
		cp	d	;lastseq
		jr	nz,loop_xmodem

		call	serial_getchar
		cpl
		cp	d	;~lastseq
		jr	nz,loop_xmodem

		ld	b,#128
		ld	c,#0	;chksum
		ld	hl,(SYSM_LASTED)

loop_xmodem2:	call	serial_getchar
		ld	e,a
		add	a,c
		ld	c,a
		ld	(hl),e
		inc	hl
		djnz	loop_xmodem2

		ld	(SYSM_LASTED),hl
		call	serial_getchar
		cp	c
		jr	nz,load_xm_ckerr	

		inc	d
		ld	a,#ACK
		ld	c,#1	; TX Char
		rst	0x20
		jr	endloop_xmodem

load_xm_ckerr:
		ld	a,#NAK
		ld	c,#1	; TX Char
		rst	0x20
		jr	endloop_xmodem	

load_xmodem_nochar:
		dec	bc
		ld	a,b
		or	c
		jr	nz,loop_xmodem

		ld	a,#NAK
		ld	c,#1	; TX Char
		rst	0x20

endloop_xmodem:
		ld	bc,#30000
		jr	loop_xmodem

serial_getchar:
		push	bc

serial_getchar1:
		ld	c,#3
		rst	0x20
		jr	z,serial_getchar1

		pop	bc
		ret

;///////////////////////////////////////////////////////////////////////
;///////////////////////////   KRAFTMON END   //////////////////////////
;///////////////////////////////////////////////////////////////////////

