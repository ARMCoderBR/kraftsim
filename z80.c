/*
 * z80.c
 *
 *  Created on: 13 de out de 2022
 *      Author: milton
 */

#include <stddef.h>
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
    z->code_prefix = 0;
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

    return z80_read(z,z->pc++);
}

////////////////////////////////////////////////////////////////////////////////
uint8_t *z80_get_phl_dest(z80_t *z){

    uint16_t addr = z->hl;

    if ((z->code_prefix == 0xdd)||(z->code_prefix == 0xfd)){

        uint8_t ofs = z80_fetch(z);

        if (ofs < 128){

            if (z->code_prefix == 0xdd)
                addr = z->ix + ofs;
            else
                addr = z->iy + ofs;
        }
        else{
            ofs ^= 0xff;
            ofs ++;
            if (z->code_prefix == 0xdd)
                addr = z->ix - ofs;
            else
                addr = z->iy - ofs;
        }
    }

    if ((addr >= z->rambase)&&(addr <= z->ramend))
        return z->ram+(addr - z->rambase);

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
const uint8_t *z80_get_phl_orig(z80_t *z){

    uint16_t addr = z->hl;

    if ((z->code_prefix == 0xdd)||(z->code_prefix == 0xfd)){

        uint8_t ofs = z80_fetch(z);

        if (ofs < 128){

            if (z->code_prefix == 0xdd)
                addr = z->ix + ofs;
            else
                addr = z->iy + ofs;
        }
        else{
            ofs ^= 0xff;
            ofs ++;
            if (z->code_prefix == 0xdd)
                addr = z->ix - ofs;
            else
                addr = z->iy - ofs;
        }
    }

    if (addr < z->romsz)
        return z->rom+addr;

    if ((addr >= z->rambase)&&(addr <= z->ramend))
        return z->ram+(addr - z->rambase);

    return NULL;
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
uint8_t *z80_get_dest(z80_t *z){

    uint8_t opc = z->opcode;

    switch (opc & 0b00111000){

        case 0b00000000:
            return &z->_b;
        case 0b00001000:
            return &z->_c;
        case 0b00010000:
            return &z->_d;
        case 0b00011000:
            return &z->_e;
        case 0b00100000:
            return &z->_h;
        case 0b00101000:
            return &z->_l;
        case 0b00111000:
            return &z->_a;
    }

    // (hl)
    return z80_get_phl_dest(z);
}

////////////////////////////////////////////////////////////////////////////////
const uint8_t *z80_get_orig(z80_t *z){

    uint8_t opc = z->opcode;

    switch (opc & 0b00000111){

        case 0b00000000:
            return &z->_b;
        case 0b00000001:
            return &z->_c;
        case 0b00000010:
            return &z->_d;
        case 0b00000011:
            return &z->_e;
        case 0b00000100:
            return &z->_h;
        case 0b00000101:
            return &z->_l;
        case 0b00000111:
            return &z->_a;
    }

    // (hl)
    return z80_get_phl_orig(z);
}

////////////////////////////////////////////////////////////////////////////////
void z80_step(z80_t *z){

    z->opcode = z80_fetch(z);

    //0x76 = 0b01 110 110
    if (z->opcode == 0x76){   //HALT

        // TODO: HALT
    }
    else
    if (z->opcode == 0xdd){   //DD Prefix (IX)

        z->code_prefix = 0xdd;
    }
    else
    if (z->opcode == 0xfd){   //FD Prefix (IY)

        z->code_prefix = 0xfd;
    }
    else
    if ((z->opcode & 0b11000000) == 0b01000000){  // LD R,R' / LD (HL),R / LD R,(HL)

        uint8_t *pdest = z80_get_dest(z);
        if (pdest){

            const uint8_t *porig = z80_get_orig(z);
            uint8_t orig = 0xFF;
            if (porig)
                orig = *porig;
            *pdest = orig;
        }
    }
    else
    if ((z->opcode & 0b11000111) == 0b00000110){  // LD R,n / LD (HL),n / LD (IX+d),n / LD (IY+d),n

        uint8_t *pdest = z80_get_dest(z);
        uint8_t arg = z80_fetch(z);
        if (pdest){

            *pdest = arg;
        }
    }

    z->code_prefix = 0;
}

