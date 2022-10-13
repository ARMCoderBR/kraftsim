/*
 * z80.c
 *
 *  Created on: 13 de out de 2022
 *      Author: milton
 */

#include "z80.h"

////////////////////////////////////////////////////////////////////////////////
void z80_initialize(z80_t *z, const uint8_t *rom, uint16_t romsz, uint8_t *ram, uint16_t rambase, uint16_t ramsz){

    z->rom = rom;
    z->romsz = romsz;
    z->ram = ram;
    z->rambase = rambase;
    z->ramsz = ramsz;
    z->ramend = rambase + ramsz - 1;
}

////////////////////////////////////////////////////////////////////////////////
/*
RESET. Reset (input, active Low). RESET initializes the CPU as follows: it resets the
interrupt enable flip-flop, clears the Program Counter and registers I and R, and sets the
interrupt status to Mode 0
 */
void z80_reset (z80_t *z){

    z->iff1 = z->iff2 = 0;
    z->_i = z->_r = 0;
    z->im = 0;
    z->pc = 0;
}

////////////////////////////////////////////////////////////////////////////////
uint8_t z80_read(z80_t *z, uint16_t addr){

    if (addr < z->romsz)
        return z->rom[addr];

    if ((addr >= z->rambase)&&(addr <= z->ramend))
        return (z->ram[addr - z->rambase]);

    return 0xff;
}

////////////////////////////////////////////////////////////////////////////////
void z80_write(z80_t *z, uint16_t addr, uint8_t byte){

    if ((addr >= z->rambase)&&(addr <= z->ramend))
        z->ram[addr - z->rambase] = byte;
}

////////////////////////////////////////////////////////////////////////////////
uint8_t z80_fetch(z80_t *z){

    return z80_read(z->pc);
}

////////////////////////////////////////////////////////////////////////////////
void z80_push(z80_t *z, uint16_t val){

    --z->sp;
    z80_write(z,z->sp,val>>8);
    --z->sp;
    z80_write(z,z->sp,val&0x0f);
}

////////////////////////////////////////////////////////////////////////////////
uint16_t z80_pop(z80_t *z){

    uint16_t val = 0;
    uint16_t val2 = 0;
    val = z80_read(z,z->sp);
    ++z->sp;
    val2 = z80_read(z,z->sp);
    ++z->sp;

    val2 <<= 8;
    return val|val2;
}

////////////////////////////////////////////////////////////////////////////////
void z80_step(z80_t *z){

    uint8_t op1 = z80_fetch(z);


}

