#ifndef __KRAFT80_H__
#define __KRAFT80_H__

////////////////////////////////////////////////////////////////////////////////
// LEDS & BUTTONS
#define PORTLEDS	0x00
#define PORTBUTTONS	0x00
#define PORTDISP	0x10

#define PROG_AREA	0x2000

////////////////////////////////////////////////////////////////////////////////
// FPGA addr mapping (base 0x50)
// 0000: Video RAM Data (R/W)
// 0001: Video ADDR Low (W)
// 0010: Video ADDR High (W)
// 0011: Video control (W)
// 0100: Timer Status & Control(R/W)
// 0101: PS/2 RX Data (R)
// 0110: Sound REG Index (W)
// 0111: Sound REG Data (W)
// 1000: Serial Status & Control (R/W)
// 1001: Serial Data RX/TX (R/W)
// 1010: SPI Status & Control(R/W)
// 1011: SPI Data RX/TX (R/W)
// 1100
// 1101
// 1110
// 1111: FPGA Interrupt Status Reg (R)

// VIDEO
#define PORTDATA	0x50
#define PORTADDRL	0x51
#define PORTADDRH	0x52
#define PORTMODE	0x53

// TIMER STATUS/CONTROL
#define PORTTIMER	0x54

// PS2 KEYBOARD
#define PORTKEY		0x55

// AUDIO
#define PORTAYADDR	0x56
#define PORTAYDATA	0x57

// SERIAL
#define PORTSERSTATUS	0x58
#define PORTSERCTL	0x58
#define PORTSERDATA	0x59

// SPI (Memory card)
#define PORTSPISTATUS	0x5A
#define PORTSPICTL	0x5A
#define PORTSPIDATA	0x5B

// FPGA STATUS
#define PORTFPGASTATUS	0x5F

// INTERRUPT VECTORS (RAM)
#define isr0vector_addr	0xF800
#define isr1vector_addr	0xF802
#define isr2vector_addr	0xF804
#define isr3vector_addr	0xF806
#define isr4vector_addr	0xF808
#define isr5vector_addr	0xF80A
#define isr6vector_addr	0xF80C
#define isr7vector_addr	0xF80E

// FREE RUNNING 1 BYTE TIMER COUNTER
#define timecount_addr	0xF812

#endif

