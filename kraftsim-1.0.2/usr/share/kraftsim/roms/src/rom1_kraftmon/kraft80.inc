;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; LEDS & BUTTONS
PORTLEDS	.equ	0x00
PORTBUTTONS	.equ	0x00
PORTDISP	.equ	0x10

PROG_AREA	.equ	0x2000

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; FPGA addr mapping (base 0x50)
; 0000: Video RAM Data (R/W)
; 0001: Video ADDR Low (W)
; 0010: Video ADDR High (W)
; 0011: Video control (W)
; 0100: Timer Status & Control(R/W)
; 0101: PS/2 RX Data (R)
; 0110: Sound REG Index (W)
; 0111: Sound REG Data (W)
; 1000: Serial Status & Control (R/W)
; 1001: Serial Data RX/TX (R/W)
; 1010: SPI Status & Control(R/W)
; 1011: SPI Data RX/TX (R/W)
; 1100
; 1101
; 1110
; 1111: FPGA Interrupt Status Reg (R)

; VIDEO
PORTDATA	.equ	0x50
PORTADDRL	.equ	0x51
PORTADDRH	.equ	0x52
PORTMODE	.equ	0x53

; TIMER STATUS/CONTROL
PORTTIMER	.equ	0x54

; PS2 KEYBOARD
PORTKEY		.equ	0x55

; AUDIO
PORTAYADDR	.equ	0x56
PORTAYDATA	.equ	0x57

; SERIAL
PORTSERSTATUS	.equ	0x58
PORTSERCTL	.equ	0x58
PORTSERDATA	.equ	0x59

; SPI (Memory card)
PORTSPISTATUS	.equ	0x5A
PORTSPICTL	.equ	0x5A
PORTSPIDATA	.equ	0x5B

; FPGA STATUS
PORTFPGASTATUS	.equ	0x5F

; INTERRUPT VECTORS (RAM)
isr0vector_addr	.equ	0xF800
isr1vector_addr	.equ	0xF802
isr2vector_addr	.equ	0xF804
isr3vector_addr	.equ	0xF806
isr4vector_addr	.equ	0xF808
isr5vector_addr	.equ	0xF80A
isr6vector_addr	.equ	0xF80C
isr7vector_addr	.equ	0xF80E
diskvector_addr	.equ	0xF810
timecount_addr	.equ	0xF812

