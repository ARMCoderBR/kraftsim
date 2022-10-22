/*
 * z80.c
 *
 *  Created on: 13 de out de 2022
 *      Author: milton
 */

#include <stddef.h>
#include <stdio.h>

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
void z80_refresh_up(z80_t *z){

    uint8_t r8 = z->_r & 0x80;
    z->_r++;
    z->_r &= 0x7F;
    z->_r |= r8;
}

////////////////////////////////////////////////////////////////////////////////
uint8_t z80_fetch(z80_t *z){

    uint8_t b = z80_read(z,z->pc++);
    printf("%02X ",b);
    return b;
}

////////////////////////////////////////////////////////////////////////////////
uint8_t *z80_get_phl_dest(z80_t *z){

    uint16_t addr = z->hl;

    if (z->code_prefix & (CODE_PREFIX_DD | CODE_PREFIX_FD)){

        uint8_t ofs = z80_fetch(z);

        if (ofs < 128){

            if (z->code_prefix & CODE_PREFIX_DD)
                addr = z->ix + ofs;
            else
                addr = z->iy + ofs;
        }
        else{
            ofs ^= 0xff;
            ofs ++;
            if (z->code_prefix & CODE_PREFIX_DD)
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

    if (z->code_prefix & (CODE_PREFIX_DD | CODE_PREFIX_FD)){

        uint8_t ofs = z80_fetch(z);

        if (ofs < 128){

            if (z->code_prefix == CODE_PREFIX_DD)
                addr = z->ix + ofs;
            else
                addr = z->iy + ofs;
        }
        else{
            ofs ^= 0xff;
            ofs ++;
            if (z->code_prefix == CODE_PREFIX_DD)
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
    z80_write(z,z->sp,val&0xFF);
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
void z80_add_acc (z80_t *z, int8_t arg, uint8_t add_cy){

    if (add_cy)
        if (z->_f & FLG_C)
            arg++;

    z->_f &= ~(FLG_S|FLG_Z|FLG_H|FLG_PV|FLG_N|FLG_C);
    uint16_t sum = z->_a + arg;

    if (sum & 0xFF00){
        z->_f |= FLG_C;
        sum &= 0xFF;
    }

    if (sum & 0x80)
        z->_f |= FLG_S;

    if (!sum)
        z->_f |= FLG_Z;

    if ((z->_a & 0x0F) + (arg & 0x0F) > 0x0F)
        z->_f |= FLG_H;

    if (!((z->_a ^ arg) & 0x80)){

        if (((z->_a ^ sum) & 0x80))
            z->_f |= FLG_PV;
    }

    z->_a = sum;
}

////////////////////////////////////////////////////////////////////////////////
void z80_sub_acc (z80_t *z, uint8_t arg, uint8_t sub_cy){

    if (sub_cy)
        if (z->_f & FLG_C)
            arg++;

    z->_f &= ~(FLG_S|FLG_Z|FLG_H|FLG_PV|FLG_C);
    z->_f |= FLG_N;             // Indica op. subtração

    if (z->_a < arg)
        z->_f |= FLG_C;

    if ((z->_a & 0x0F) < (arg & 0x0F))
        z->_f |= FLG_H;

    int8_t diff = z->_a - arg;

    if ((z->_a ^ arg) & 0x80){

        if (((z->_a ^ diff) & 0x80))
            z->_f |= FLG_PV;
    }

    if (diff & 0x80)
        z->_f |= FLG_S;

    if (!diff)
        z->_f |= FLG_Z;

    z->_a = diff;
}

////////////////////////////////////////////////////////////////////////////////
void z80_exec_cb(z80_t *z){


}

////////////////////////////////////////////////////////////////////////////////
void z80_exec_ed(z80_t *z){

    uint8_t opcode = z->opcode;
    if (opcode == 0x57){                             // LD A,I

        z->_a = z->_i;
        z->_f &= FLG_C;
        if (z->_a & 0x80)
            z->_f |= FLG_S;
        else
        if (!z->_a)
            z->_f |= FLG_Z;
        if (z->iff2)
            z->_f |= FLG_PV;
    }
    else
    if (opcode == 0x5F){                             // LD A,R

        z->_a = z->_r;
        z->_f &= FLG_C;
        if (z->_a & 0x80)
            z->_f |= FLG_S;
        else
        if (!z->_a)
            z->_f |= FLG_Z;
        if (z->iff2)
            z->_f |= FLG_PV;
    }
    else
    if (opcode == 0x47){                             // LD I,A

        z->_i = z->_a;
    }
    else
    if (opcode == 0x4F){                             // LD R,A

        z->_r = z->_a;
    }
    else
    if ((opcode & 0b11001111) == 0b01001011){        // LD dd,(nn)

        uint16_t addrl = z80_fetch(z);
        uint16_t addrh = z80_fetch(z);
        addrh <<= 8;
        addrh |= addrl;

        uint16_t arg = z80_read(z, addrh++);
        uint16_t arg2 = z80_read(z, addrh);
        arg2 <<= 8;
        arg |= arg2;
        switch(opcode & 0b00110000){

            case 0b00000000:
                z->bc = arg;
                break;
            case 0b00010000:
                z->de = arg;
                break;
            case 0b00100000:
                z->hl = arg;
                break;
            case 0b00110000:
                z->sp = arg;
                break;
        }
    }
    else
    if ((opcode & 0b11001111) == 0b01000011){        // LD (nn),dd

        uint16_t addrl = z80_fetch(z);
        uint16_t addrh = z80_fetch(z);
        addrh <<= 8;
        addrh |= addrl;

        uint16_t arg;
        switch(opcode & 0b00110000){

            case 0b00000000:
                arg = z->bc;
                break;
            case 0b00010000:
                arg = z->de;
                break;
            case 0b00100000:
                arg = z->hl;
                break;
            case 0b00110000:
                arg = z->sp;
                break;
        }

        z80_write(z, addrh++, arg & 0xFF);
        z80_write(z, addrh, arg >> 8);
    }
    else
    if ((opcode == 0xA0)||(opcode == 0xB0)||
        (opcode == 0xA8)||(opcode == 0xB8)){            // LDI / LDIR / LDD / LDDR

        z80_write(z, z->de, z80_read(z, z->hl));
        if (opcode & 0x08){
            --z->hl; --z->de;
        }
        else{
            ++z->hl; ++z->de;
        }
        z->bc--;
        z->_f &= ~(FLG_H|FLG_N|FLG_PV);
        if (z->bc){
            z->_f |= FLG_PV;
            if (opcode & 0x10)
                z->pc -= 2;
        }
    }
    else
    if ((opcode == 0xA1)||(opcode == 0xB1)||
        (opcode == 0xA9)||(opcode == 0xB9)){            // CPI / CPIR / CPD / CPDR

        uint8_t arg = z80_read(z, z->hl);
        if (opcode & 0x08){
            --z->hl;
        }
        else{
            ++z->hl;
        }
        z->bc--;

        z->_f &= ~(FLG_H|FLG_PV|FLG_S|FLG_Z);
        z->_f |= FLG_N;

        if (z->bc)
            z->_f |= FLG_PV;

        if ((z->_a & 0x0F) < (arg & 0x0F))
            z->_f |= FLG_H;

        if (z->_a < arg)
            z->_f |= FLG_S;

        if (z->_a == arg)
            z->_f |= FLG_Z;
        else{
            if (opcode & 0x10){
                if (z->bc)
                    z->pc -= 2;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
void z80_step(z80_t *z){

rescan:
    z->opcode = z80_fetch(z);

    if (z->opcode == 0xDD){                             //DD Prefix (IX)

        z->code_prefix |= CODE_PREFIX_DD;
        z->code_prefix &= ~CODE_PREFIX_FD;
        goto rescan;
    }

    if (z->opcode == 0xFD){                             //FD Prefix (IY)

        z->code_prefix |= CODE_PREFIX_FD;
        z->code_prefix &= ~CODE_PREFIX_DD;
        goto rescan;
    }

    if (z->opcode == 0xCB){                             //CB Prefix (bit ops)

        z->code_prefix |= CODE_PREFIX_CB;
        z->code_prefix &= ~CODE_PREFIX_ED;
        goto rescan;
    }

    if (z->opcode == 0xED){                             //ED Prefix (specials)

        z->code_prefix |= CODE_PREFIX_ED;
        z->code_prefix &= ~CODE_PREFIX_CB;
        goto rescan;
    }

    z80_refresh_up(z);

    ////////////////////////////////////////////////////////////////////////////
    if (z->code_prefix & CODE_PREFIX_CB){

        z80_exec_cb(z);
        z->code_prefix = 0;
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if (z->code_prefix & CODE_PREFIX_ED){

        z80_exec_ed(z);
        z->code_prefix = 0;
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if ( ((z->opcode & 0b11000111) == 0b01000110) &&
         ((z->opcode & 0b00111000) != 0b00110000) ){    // LD r,(HL) / LD r,(IX+d) / LD r,(IY+d)

        const uint8_t *porig = z80_get_orig(z);
        uint8_t arg = 0xFF;
        if (porig)
            arg = *porig;
        *z80_get_dest(z) = arg;

endxy:  z->code_prefix = 0;
        return;
    }

    if ( ((z->opcode & 0b11111000) == 0b01110000) &&
         ((z->opcode & 0b00000111) != 0b00000110) ){    // LD (HL),r / LD (IX+d),r / LD (IY+d),r

        uint8_t *pdest = z80_get_dest(z);
        if (pdest){

            *pdest = *z80_get_orig(z);
        }
        goto endxy;
    }

    if (z->opcode == 0x36){                             // LD (HL),n / LD (IX+d),n / LD (IY+d),n

        uint8_t *pdest = z80_get_dest(z);
        uint8_t arg = z80_fetch(z);
        if (pdest){

            *pdest = arg;
        }
        goto endxy;
    }

    if (z->opcode == 0x21){                             // LD HL,nn / LD IX,nn / LD IY,nn

        uint16_t argl = z80_fetch(z);
        uint16_t argh = z80_fetch(z);
        argh <<= 8;
        argh |= argl;

        if (z->code_prefix & CODE_PREFIX_DD)
            z->ix = argh;
        else
        if (z->code_prefix & CODE_PREFIX_FD)
            z->iy = argh;
        else
            z->hl = argh;

        goto endxy;
    }

    if (z->opcode == 0x2A){                             // LD HL,(nn) / LD IX,(nn) / LD IY,(nn)

        uint16_t addrl = z80_fetch(z);
        uint16_t addrh = z80_fetch(z);
        addrh <<= 8;
        addrh |= addrl;

        uint16_t arg = z80_read(z, addrh++);
        uint16_t arg2 = z80_read(z, addrh);
        arg2 <<= 8;
        arg |= arg2;

        if (z->code_prefix & CODE_PREFIX_DD)
            z->ix = arg;
        else
        if (z->code_prefix & CODE_PREFIX_FD)
            z->iy = arg;
        else
            z->hl = arg;

        goto endxy;
    }

    if (z->opcode == 0x22){                             // LD (nn),HL / LD (nn),IX / LD (nn),IY

        uint16_t addrl = z80_fetch(z);
        uint16_t addrh = z80_fetch(z);
        addrh <<= 8;
        addrh |= addrl;

        uint16_t arg;
        if (z->code_prefix & CODE_PREFIX_DD)
            arg = z->ix;
        else
        if (z->code_prefix & CODE_PREFIX_FD)
            arg = z->iy;
        else
            arg = z->hl;

        z80_write(z, addrh++, arg & 0xFF);
        z80_write(z, addrh, arg >> 8);

        goto endxy;
    }

    if (z->opcode == 0xF9){                             // LD SP,HL / LD SP,IX / LD SP,IY

        uint16_t arg;
        if (z->code_prefix & CODE_PREFIX_DD)
            arg = z->ix;
        else
        if (z->code_prefix & CODE_PREFIX_FD)
            arg = z->iy;
        else
            arg = z->hl;

        z->sp = arg;

        goto endxy;
    }

    if (z->opcode == 0xE5){                             // PUSH HL / PUSH IX / PUSH IY

        uint16_t arg;
        if (z->code_prefix & CODE_PREFIX_DD)
            arg = z->ix;
        else
        if (z->code_prefix & CODE_PREFIX_FD)
            arg = z->iy;
        else
            arg = z->hl;

        z80_push(z, arg);

        goto endxy;
    }
    else
    if (z->opcode == 0xE1){                             // POP HL / POP IX / POP IY

        uint16_t arg = z80_pop(z);

        if (z->code_prefix & CODE_PREFIX_DD)
            z->ix = arg;
        else
        if (z->code_prefix & CODE_PREFIX_FD)
            z->iy = arg;
        else
            z->hl = arg;

        goto endxy;
    }
    else
    if (z->opcode == 0xE3){                             // EX (SP),HL / EX (SP),IX / EX (SP),IY

        uint16_t arg = z80_pop(z);
        uint16_t arg2;

        if (z->code_prefix & CODE_PREFIX_DD){
            arg2 = z->ix;
            z->ix = arg;
        }
        else
        if (z->code_prefix & CODE_PREFIX_FD){
            arg2 = z->iy;
            z->iy = arg;
        }
        else{
            arg2 = z->hl;
            z->hl = arg;
        }

        z80_push(z, arg2);
        goto endxy;
    }

    if (z->opcode == 0x86){                             // ADD A,(HL) / ADD A,(IX+d) / ADD A,(IY+d)

        uint8_t arg;
        const uint8_t *parg = z80_get_phl_orig(z);
        if (parg != NULL)
            arg = *parg;
        else
            arg = 0xff;
        z80_add_acc(z, arg, 0);
        goto endxy;
    }

    if (z->opcode == 0x8E){                             // ADC A,(HL) / ADC A,(IX+d) / ADC A,(IY+d)

        uint8_t arg;
        const uint8_t *parg = z80_get_phl_orig(z);
        if (parg != NULL)
            arg = *parg;
        else
            arg = 0xff;
        z80_add_acc(z, arg, 1);
        goto endxy;
    }

    if (z->opcode == 0x96){                             // SUB (HL) / SUB (IX+d) / SUB (IY+d)

        uint8_t arg;
        const uint8_t *parg = z80_get_phl_orig(z);
        if (parg != NULL)
            arg = *parg;
        else
            arg = 0xff;
        z80_sub_acc(z, arg, 0);
        goto endxy;
    }

    if (z->opcode == 0x9E){                             // SBC A,(HL) / SBC A,(IX+d) / SBC A,(IY+d)

        uint8_t arg;
        const uint8_t *parg = z80_get_phl_orig(z);
        if (parg != NULL)
            arg = *parg;
        else
            arg = 0xff;
        z80_sub_acc(z, arg, 1);
        goto endxy;
    }

    if (z->opcode == 0x23){                             // INC HL / INC IX / INC IY

        if (z->code_prefix & CODE_PREFIX_DD)
            z->ix++;
        else
        if (z->code_prefix & CODE_PREFIX_FD)
            z->iy++;
        else
            z->hl++;
        goto endxy;
    }

    if (z->opcode == 0x2B){                             // DEC HL / DEC IX / DEC IY

        if (z->code_prefix & CODE_PREFIX_DD)
            z->ix--;
        else
        if (z->code_prefix & CODE_PREFIX_FD)
            z->iy--;
        else
            z->hl--;
        goto endxy;
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    if (z->code_prefix){    // Elimina códigos DD e FD inválidos

        printf("Invalid IX/IY prefix\n");
        goto endxy;
    }

    ////////////////////////////////////////////////////////////////////////////

    if (!z->opcode){   //NOP

        return;
    }

    if (z->opcode == 0x76){   //HALT

        --z->pc;
        z->halted = 1;
        return;
    }

    if ((z->opcode & 0b11000000) == 0b01000000){         // LD r,r' - só registrador, sem acesso a memória

        *z80_get_dest(z) = *z80_get_orig(z);
        return;
    }

    if ( ((z->opcode & 0b11000111) == 0b00000110) &&
         ((z->opcode & 0b00111000) != 0b00110000) ){    // LD r,n

        *z80_get_dest(z) = z80_fetch(z);
        return;
    }

    if (z->opcode == 0x0A){                             // LD A,(BC)

        z->_a = z80_read(z, z->bc);
        return;
    }

    if (z->opcode == 0x1A){                             // LD A,(DE)

        z->_a = z80_read(z, z->de);
        return;
    }

    if (z->opcode == 0x3A){                             // LD A,(nn)

        uint16_t addrl = z80_fetch(z);
        uint16_t addrh = z80_fetch(z);
        z->_a = z80_read(z, addrh<<8 | addrl);
        return;
    }

    if (z->opcode == 0x02){                             // LD (BC),A

        z80_write(z, z->bc, z->_a);
        return;
    }

    if (z->opcode == 0x12){                             // LD (DE),A

        z80_write(z, z->de, z->_a);
        return;
    }

    if (z->opcode == 0x32){                             // LD (nn),A

        uint16_t addrl = z80_fetch(z);
        uint16_t addrh = z80_fetch(z);
        z80_write(z, addrh<<8 | addrl, z->_a);
        return;
    }

    if (z->opcode == 0x08){                             // EX AF,AF'

        uint16_t temp = z->af;
        z->af = z->afa;
        z->afa = temp;
        return;
    }

    if (z->opcode == 0xD9){                             // EXX

        uint16_t temp = z->bc;
        z->bc = z->bca;
        z->bca = temp;

        temp = z->de;
        z->de = z->dea;
        z->dea = temp;

        temp = z->hl;
        z->hl = z->hla;
        z->hla = temp;
        return;
    }

    if ((z->opcode & 0b11001111) == 0b00000001){        // LD dd,nn

        uint16_t argl = z80_fetch(z);
        uint16_t argh = z80_fetch(z);
        argh <<= 8;
        argh |= argl;
        switch(z->opcode & 0b00110000){
            case 0b00000000:
                z->bc = argh;
                break;
            case 0b00010000:
                z->de = argh;
                break;
//            case 0b00100000:
//                if (z->code_prefix == 0xDD)
//                    z->ix = argh;
//                else
//                if (z->code_prefix == 0xFD)
//                    z->iy = argh;
//                else
//                    z->hl = argh;
//                break;
            case 0b00110000:
                z->sp = argh;
                break;
        }
        return;
    }

    if ((z->opcode & 0b11110000) == 0b10000000){        // ADD A,r / ADC A,r

        uint8_t arg;
        switch(z->opcode & 0b00000111){

            case 0b00000000:
                arg = z->_b;
                break;
            case 0b00000001:
                arg = z->_c;
                break;
            case 0b00000010:
                arg = z->_d;
                break;
            case 0b00000011:
                arg = z->_e;
                break;
            case 0b00000100:
                arg = z->_h;
                break;
            case 0b00000101:
                arg = z->_l;
                break;
//            case 0b00000110:
//                break;
            case 0b00000111:
                arg = z->_a;
                break;
        }

        z80_add_acc(z, arg, z->opcode & 0x08 ? 1:0);    // Seleciona ADD ou ADC
        return;
    }

    if (z->opcode == 0xC6){                             // ADD A,n

        uint8_t arg = z80_fetch(z);
        z80_add_acc(z, arg, 0);
        return;
    }

    if (z->opcode == 0xCE){                             // ADC A,n

        uint8_t arg = z80_fetch(z);
        z80_add_acc(z, arg, 1);
        return;
    }



    if ((z->opcode & 0b11110000) == 0b10010000){        // SUB r / SBC A,r

        uint8_t arg;
        switch(z->opcode & 0b00000111){

            case 0b00000000:
                arg = z->_b;
                break;
            case 0b00000001:
                arg = z->_c;
                break;
            case 0b00000010:
                arg = z->_d;
                break;
            case 0b00000011:
                arg = z->_e;
                break;
            case 0b00000100:
                arg = z->_h;
                break;
            case 0b00000101:
                arg = z->_l;
                break;
//            case 0b00000110:
//                break;
            case 0b00000111:
                arg = z->_a;
                break;
        }

        z80_sub_acc(z, arg, z->opcode & 0x08 ? 1:0);    // Seleciona SUB ou SBC
        return;
    }

    if (z->opcode == 0xD6){                             // SUB n

        uint8_t arg = z80_fetch(z);
        z80_sub_acc(z, arg, 0);
        return;
    }

    if (z->opcode == 0xDE){                             // SBC A,n

        uint8_t arg = z80_fetch(z);
        z80_sub_acc(z, arg, 1);
        return;
    }

    if (z->opcode == 0xEB){                             // EX DE,HL

        uint16_t temp = z->hl;
        z->hl = z->de;
        z->de = temp;
        return;
    }

    if ((z->opcode & 0b11001111) == 0b11000101){        // PUSH qq

        uint16_t arg;
        switch(z->opcode & 0b00110000){

            case 0b00000000:
                arg = z->bc;
                break;
            case 0b00010000:
                arg = z->de;
                break;
//            case 0b00100000:
//                if (z->code_prefix & CODE_PREFIX_DD)
//                    arg = z->ix;
//                else
//                if (z->code_prefix & CODE_PREFIX_FD)
//                    arg = z->iy;
//                else
//                    arg = z->hl;
//                break;
            case 0b00110000:
                arg = z->af;
                break;
        }

        z80_push(z, arg);
        return;
    }

    if ((z->opcode & 0b11001111) == 0b11000001){        // POP qq

        uint16_t arg = z80_pop(z);
        switch(z->opcode & 0b00110000){

            case 0b00000000:
                z->bc = arg;
                break;
            case 0b00010000:
                z->de = arg;
                break;
//            case 0b00100000:
//                if (z->code_prefix & CODE_PREFIX_DD)
//                    z->ix = arg;
//                else
//                if (z->code_prefix & CODE_PREFIX_FD)
//                    z->iy = arg;
//                else
//                    z->hl = arg;
//                break;
            case 0b00110000:
                z->af = arg;
                break;
        }
        return;
    }

    if ((z->opcode & 0b11000111) == 0b00000011){        // INC ss / DEC ss

        switch(z->opcode){
            case 0b00000011:
                z->bc++;
                break;
            case 0b00010011:
                z->de++;
                break;
            case 0b00110011:
                z->sp++;
                break;
            case 0b00001011:
                z->bc--;
                break;
            case 0b00011011:
                z->de--;
                break;
            case 0b00111011:
                z->sp--;
                break;
        }
        return;
    }

    if (z->opcode == 0x37){                             // SCF

        z->_f &= ~(FLG_H|FLG_N);
        z->_f |= FLG_C;
        return;
    }

    if (z->opcode == 0x3F){                             // CCF

        z->_f &= ~(FLG_H|FLG_N);

        if (z->_f & FLG_C)
            z->_f |= FLG_H;

        z->_f ^= FLG_C;
        return;
    }

    printf("Unk. Opcode\n");
}

void z80_dump(z80_t *z){

    printf("\nPC:%04X  SP:%04X  BC:%04x  DE:%04X  HL:%04X  IX:%04X  IY:%04X  AF:%04X   FLAGS:",
            z->pc,z->sp,z->bc,z->de,z->hl,z->ix,z->iy,z->af);

#define FLG_S 0x80
#define FLG_Z 0x40
#define FLG_H 0x10
#define FLG_PV 0x04
#define FLG_N 0x02
#define FLG_C 0x01

    if (z->_f & FLG_S)
        printf("NEG ");
    else
        printf("POS ");

    if (z->_f & FLG_Z)
        printf("Z  ");
    else
        printf("NZ ");

    if (z->_f & FLG_H)
        printf("H  ");
    else
        printf("NH ");

    if (z->_f & FLG_PV)
        printf("PE OV ");
    else
        printf("PO NV ");

    if (z->_f & FLG_N)
        printf("SUB ");
    else
        printf("ADD ");

    if (z->_f & FLG_C)
        printf("CY ");
    else
        printf("NC ");

    printf("\n");
}

////////////////////////////////////////////////////////////////////////////////
void z80_dump_mem(z80_t *z, uint16_t start, uint16_t size){

    int i;

    uint16_t start2 = start;
    if (start2 % 16){

        start2 &= ~15;
    }

    size += start & 15;

    printf("\n      +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F");
    for (i = 0; i < size; i++){

        if (!(i&15))
            printf("\n%04x  ",start2+i);

        if ((i + start2) < start)
            printf("   ");
        else
            printf("%02X ",z80_read(z,i+start2));

//        if (!((i+1)&15))
//            printf("\n");
    }
    printf("\n");
}

