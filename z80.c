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
    z->halted = 0;
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
uint8_t *z80_get_phl_dest_last(z80_t *z){

    uint16_t addr = z->hl;

    if (z->code_prefix & (CODE_PREFIX_DD | CODE_PREFIX_FD)){

        uint8_t ofs = z->last_ofs;

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
uint8_t *z80_get_phl_dest(z80_t *z){

    uint16_t addr = z->hl;

    if (z->code_prefix & (CODE_PREFIX_DD | CODE_PREFIX_FD)){

        uint8_t ofs = z80_fetch(z);

        z->last_ofs = ofs;

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
    else
        z->last_ofs = 0;

    if ((addr >= z->rambase)&&(addr <= z->ramend))
        return z->ram+(addr - z->rambase);

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
const uint8_t *z80_get_phl_orig(z80_t *z){

    uint16_t addr = z->hl;

    if (z->code_prefix & (CODE_PREFIX_DD | CODE_PREFIX_FD)){

        uint8_t ofs = z80_fetch(z);

        z->last_ofs = ofs;

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
    else
        z->last_ofs = 0;

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
uint8_t *z80_get_dest(z80_t *z, int use_code_prefix){

    uint8_t opc = z->opcode;
    uint8_t code_prefix_for_reg8_undoc = use_code_prefix ? z->code_prefix : 0;

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
            if (code_prefix_for_reg8_undoc & CODE_PREFIX_DD)
                return &z->_ixh;
            else if (code_prefix_for_reg8_undoc & CODE_PREFIX_FD)
                return &z->_iyh;
            else
                return &z->_h;
        case 0b00101000:
            if (code_prefix_for_reg8_undoc & CODE_PREFIX_DD)
                return &z->_ixl;
            else if (code_prefix_for_reg8_undoc & CODE_PREFIX_FD)
                return &z->_iyl;
            else
                return &z->_l;
        case 0b00111000:
            return &z->_a;
    }

    // (hl)
    return z80_get_phl_dest(z);
}

////////////////////////////////////////////////////////////////////////////////
const uint8_t *z80_get_orig(z80_t *z, int use_code_prefix){

    uint8_t opc = z->opcode;
    uint8_t code_prefix_for_reg8_undoc = use_code_prefix ? z->code_prefix : 0;

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
            if (code_prefix_for_reg8_undoc & CODE_PREFIX_DD)
                return &z->_ixh;
            else if (code_prefix_for_reg8_undoc & CODE_PREFIX_FD)
                return &z->_iyh;
            else
                return &z->_h;
        case 0b00000101:
            if (code_prefix_for_reg8_undoc & CODE_PREFIX_DD)
                return &z->_ixl;
            else if (code_prefix_for_reg8_undoc & CODE_PREFIX_FD)
                return &z->_iyl;
            else
                return &z->_l;
        case 0b00000111:
            return &z->_a;
    }

    // (hl)
    return z80_get_phl_orig(z);
}

////////////////////////////////////////////////////////////////////////////////
uint8_t *z80_get_reg8_ptr(z80_t *z, int use_code_prefix){

    uint8_t opc = z->opcode;
    uint8_t code_prefix_for_reg8_undoc = use_code_prefix ? z->code_prefix : 0;

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
            if (code_prefix_for_reg8_undoc & CODE_PREFIX_DD)
                return &z->_ixh;
            else if (code_prefix_for_reg8_undoc & CODE_PREFIX_FD)
                return &z->_iyh;
            else
                return &z->_h;
        case 0b00000101:
            if (code_prefix_for_reg8_undoc & CODE_PREFIX_DD)
                return &z->_ixl;
            else if (code_prefix_for_reg8_undoc & CODE_PREFIX_FD)
                return &z->_iyl;
            else
                return &z->_l;
        case 0b00000111:
            return &z->_a;
    }

    // (hl)
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
void z80_update_flags_logic_reg(z80_t *z, int flg_h, uint8_t reg){

    uint8_t f = z->_f;

    if (reg & 0x80)
        f |= FLG_S;
    else
        f &= ~FLG_S;

    if (!reg)
        f |= FLG_Z;
    else
        f &= ~FLG_Z;

    f &= ~0x28;
    f |= (reg & 0x28);

    int p = 1;
    int i;
    for (i = 0; i < 4; i++){

        int b = reg & 3;
        if (b==1) p^=1;
        else
        if (b==2) p^=1;
        reg >>= 2;
    }

    if (p)
        f |= FLG_PV;
    else
        f &= ~FLG_PV;

    if (flg_h)
        f |= FLG_H;
    else
        f &= ~FLG_H;

    f &= ~(FLG_N|FLG_C);

    z->_f = f;
}

////////////////////////////////////////////////////////////////////////////////
void z80_update_flags_logic_acc(z80_t *z, int flg_h){

    z80_update_flags_logic_reg(z, flg_h, z->_a);
}

////////////////////////////////////////////////////////////////////////////////
void z80_add_acc (z80_t *z, int8_t arg, uint8_t add_cy){

    int16_t sum16 = (int16_t)arg;
    int16_t acc16 = z->_a;
    int ovf = 0;

    sum16 += acc16;
    if (add_cy)
        sum16++;

    if ((sum16 > 127) || (sum16 < -128)) ovf = 1;

    z->_f &= ~(FLG_S|FLG_Z|FLG_H|FLG_PV|FLG_N|FLG_C);

    if (sum16 & 0xFF00)
        z->_f |= FLG_C;

    if (sum16 & 0x80)
        z->_f |= FLG_S;

    if (!(sum16 & 0xFF))
        z->_f |= FLG_Z;

    if ((z->_a & 0x0F) + (arg & 0x0F) + (add_cy?1:0) > 0x0F)
        z->_f |= FLG_H;

    if (ovf)
        z->_f |= FLG_PV;

    z->_f &= ~0x28;
    z->_f |= (sum16 & 0x28);
    z->_a = sum16 & 0xFF;
}

////////////////////////////////////////////////////////////////////////////////
void z80_sub_acc (z80_t *z, uint8_t arg, uint8_t sub_cy){

    int16_t dif16 = -(int16_t)arg;
    int16_t acc16 = z->_a;
    int ovf = 0;

    dif16 += acc16;
    if (sub_cy)
        dif16--;

    if ((dif16 > 127) || (dif16 < -128)) ovf = 1;

    z->_f &= ~(FLG_S|FLG_Z|FLG_H|FLG_PV|FLG_C);
    z->_f |= FLG_N;             // Indica op. subtração

    if (dif16 & 0xFF00)
        z->_f |= FLG_C;

    if (dif16 & 0x80)
        z->_f |= FLG_S;

    if (!(dif16 & 0xFF))
        z->_f |= FLG_Z;

    if ((z->_a & 0x0F) < (sub_cy?1:0)+(arg & 0x0F))
        z->_f |= FLG_H;

    if (ovf)
        z->_f |= FLG_PV;

    z->_f &= ~0x28;
    z->_f |= (dif16 & 0x28);
    z->_a = dif16 & 0xFF;
}

////////////////////////////////////////////////////////////////////////////////
void z80_exec_cb(z80_t *z){

    const uint8_t *operand_hxy_r = NULL;
    uint8_t *operand_hxy_w = NULL;

    if (z->code_prefix & (CODE_PREFIX_DD | CODE_PREFIX_FD)){

        z->pc--;
        operand_hxy_r = z80_get_phl_orig(z);
        operand_hxy_w = z80_get_phl_dest_last(z);
        z->opcode = z80_fetch(z);
    }
    else{

        operand_hxy_r = z80_get_phl_orig(z);
        operand_hxy_w = z80_get_phl_dest_last(z);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// RLC m
    if ((z->opcode & 0b11111000) == 0b00000000){

        if (z->opcode  == 0b00000110){                  // RLC (HL) / RLC (IX+d) / RLC (IY+d)

            uint8_t arg = 0xff;
            if (operand_hxy_r != NULL)
                arg = *operand_hxy_r;

            uint8_t arg2 = arg;

            arg <<= 1;
            if (arg2 & 0x80)
                arg |= 0x01;

            if (operand_hxy_w != NULL)
                *operand_hxy_w = arg;

            z80_update_flags_logic_reg(z, 0, arg);

            if (arg2 & 0x80)
                z->_f |= FLG_C;
            return;
        }

        uint8_t arg,arg2;                               // RLC r
        uint8_t *reg = z80_get_reg8_ptr(z,1);   //???

        arg2 = *reg; arg = arg2 << 1;
        if (arg2 & 0x80)
            arg |= 0x01;
        *reg = arg;

        z80_update_flags_logic_reg(z, 0, arg & 0xFF);

        if (arg2 & 0x80)
            z->_f |= FLG_C;
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// RL m
    if ((z->opcode & 0b11111000) == 0b00010000){        // RL r

        if (z->opcode  == 0b00010110){

            uint8_t arg = 0xff;
            if (operand_hxy_r != NULL)
                arg = *operand_hxy_r;

            uint8_t arg2 = arg;

            arg <<= 1;
            if (z->_f & FLG_C)
                arg |= 0x01;

            if (operand_hxy_w != NULL)
                *operand_hxy_w = arg;

            z80_update_flags_logic_reg(z, 0, arg);

            if (arg2 & 0x80)
                z->_f |= FLG_C;
            return;
        }

        uint16_t arg, arg2;                             // RL (HL) / RL (IX+d) / RL (IY+d)
        uint8_t *reg = z80_get_reg8_ptr(z,0);

        arg2 = *reg; arg = arg2 << 1;
        if (z->_f & FLG_C)
            arg |= 0x01;
        *reg = arg;

        z80_update_flags_logic_reg(z, 0, arg & 0xFF);

        if (arg2 & 0x80)
            z->_f |= FLG_C;
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// RRC m
    if ((z->opcode & 0b11111000) == 0b00001000){

        if (z->opcode  == 0b00001110){                  // RRC (HL) / RRC (IX+d) / RRC (IY+d)

            uint8_t arg = 0xff;
            if (operand_hxy_r != NULL)
                arg = *operand_hxy_r;

            uint8_t arg2 = arg;

            arg >>= 1;
            if (arg2 & 0x01)
                arg |= 0x80;

            if (operand_hxy_w != NULL)
                *operand_hxy_w = arg;

            z80_update_flags_logic_reg(z, 0, arg);

            if (arg2 & 0x01)
                z->_f |= FLG_C;
            return;
        }

        uint8_t arg,arg2;                               // RRC r
        uint8_t *reg = z80_get_reg8_ptr(z,0);

        arg2 = *reg; arg = arg2 >> 1;
        if (arg2 & 0x01)
            arg |= 0x80;
        *reg = arg;

        z80_update_flags_logic_reg(z, 0, arg);

        if (arg2 & 0x01)
            z->_f |= FLG_C;
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// RR m
    if ((z->opcode & 0b11111000) == 0b00011000){
        if (z->opcode  == 0b00011110){                  // RR (HL) / RR (IX+d) / RR (IY+d)

            uint8_t arg = 0xff;
            if (operand_hxy_r != NULL)
                arg = *operand_hxy_r;

            uint8_t arg2 = arg;

            arg >>= 1;
            if (z->_f & FLG_C)
                arg |= 0x80;

            if (operand_hxy_w != NULL)
                *operand_hxy_w = arg;

            z80_update_flags_logic_reg(z, 0, arg);

            if (arg2 & 0x01)
                z->_f |= FLG_C;
            return;
        }

        uint8_t arg,arg2;                               // RR r
        uint8_t *reg = z80_get_reg8_ptr(z,0);

        arg2 = *reg; arg = arg2 >> 1;
        if (z->_f & FLG_C)
            arg |= 0x80;
        *reg = arg;

        z80_update_flags_logic_reg(z, 0, arg);

        if (arg2 & 0x01)
            z->_f |= FLG_C;
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// SLA m
    if ((z->opcode & 0b11111000) == 0b00100000){

        if (z->opcode  == 0b00100110){                  // SLA (HL) / SLA (IX+d) / SLA (IY+d)

            uint8_t arg = 0xff;
            if (operand_hxy_r != NULL)
                arg = *operand_hxy_r;

            uint8_t arg2 = arg;

            arg <<= 1;

            if (operand_hxy_w != NULL)
                *operand_hxy_w = arg;

            z80_update_flags_logic_reg(z, 0, arg);

            if (arg2 & 0x80)
                z->_f |= FLG_C;
            return;
        }

        uint8_t arg,arg2;                               // SLA r
        uint8_t *reg = z80_get_reg8_ptr(z,0);

        arg2 = *reg; arg = arg2 >> 1;
        *reg = arg;

        z80_update_flags_logic_reg(z, 0, arg);

        if (arg2 & 0x80)
            z->_f |= FLG_C;
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// SRA m
    if ((z->opcode & 0b11111000) == 0b00101000){

        if (z->opcode  == 0b00101110){                  // SRA (HL) / SRA (IX+d) / SRA (IY+d)

            uint8_t arg = 0xff;
            if (operand_hxy_r != NULL)
                arg = *operand_hxy_r;

            uint8_t arg2 = arg;

            arg >>= 1; arg |= (arg2 & 0x80);

            if (operand_hxy_w != NULL)
                *operand_hxy_w = arg;

            z80_update_flags_logic_reg(z, 0, arg);

            if (arg2 & 0x01)
                z->_f |= FLG_C;
            return;
        }

        uint8_t arg,arg2;                               // SRA r
        uint8_t *reg = z80_get_reg8_ptr(z,0);

        arg2 = *reg; arg = arg2 >> 1; arg |= (arg2 & 0x80);
        *reg = arg;

        z80_update_flags_logic_reg(z, 0, arg);

        if (arg2 & 0x01)
            z->_f |= FLG_C;
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// SRL m
    if ((z->opcode & 0b11111000) == 0b00111000){

        if (z->opcode  == 0b00111110){                  // SRL (HL) / SRL (IX+d) / SRL (IY+d)

            uint8_t arg = 0xff;
            if (operand_hxy_r != NULL)
                arg = *operand_hxy_r;

            uint8_t arg2 = arg;

            arg >>= 1;

            if (operand_hxy_w != NULL)
                *operand_hxy_w = arg;

            z80_update_flags_logic_reg(z, 0, arg);

            if (arg2 & 0x01)
                z->_f |= FLG_C;
            return;
        }

        uint8_t arg,arg2;                               // SRL r
        uint8_t *reg = z80_get_reg8_ptr(z,0);

        arg2 = *reg; arg = arg2 >> 1;
        *reg = arg;

        z80_update_flags_logic_reg(z, 0, arg);

        if (arg2 & 0x01)
            z->_f |= FLG_C;
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// BIT b,m
    if ((z->opcode & 0b11000000) == 0b01000000){

        uint8_t mask = 0x01 << ((z->opcode >> 3) & 0x07);

        if ((z->opcode & 0b00000111)  == 0b00000110){   // BIT b,(HL) / BIT b,(IX+d) / BIT b,(IY+d)

            uint8_t arg = 0xff;
            if (operand_hxy_r != NULL)
                arg = *operand_hxy_r;

            if (arg & mask)
                z->_f &= ~FLG_Z;
            else
                z->_f |= FLG_Z;

            z->_f |= FLG_H;
            z->_f &= ~FLG_N;
            return;
        }

        uint8_t *reg = z80_get_reg8_ptr(z,0);             // BIT b,r
        if (*reg & mask)
            z->_f &= ~FLG_Z;
        else
            z->_f |= FLG_Z;

        z->_f |= FLG_H;
        z->_f &= ~FLG_N;
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// SET b,m / RES b,m
    if ((z->opcode & 0b10000000) == 0b10000000){

        uint8_t mask = 0x01 << ((z->opcode >> 3) & 0x07);

        if ((z->opcode & 0b00000111) == 0b00000110){   // SET/RES b,(HL) / SET/RES b,(IX+d) / SET/RES b,(IY+d)

            uint8_t arg = 0xff;
            if (operand_hxy_r != NULL)
                arg = *operand_hxy_r;

            if (z->opcode & 0b01000000)
                arg |= mask;  // SET
            else
                arg &= ~mask; // RES

            if (operand_hxy_w != NULL)
                *operand_hxy_w = arg;
            return;
        }

        uint8_t *reg = z80_get_reg8_ptr(z,0);             // SET/RES b,r

        if (z->opcode & 0b01000000)
            *reg |= mask;     // SET
        else
            *reg &= ~mask;    // RES
        return;
    }
}

////////////////////////////////////////////////////////////////////////////////
void z80_exec_ed(z80_t *z){

    uint8_t opcode = z->opcode;
    if (opcode == 0x57){                                // LD A,I

        z->_a = z->_i;
        z->_f &= FLG_C;
        if (z->_a & 0x80)
            z->_f |= FLG_S;
        else
        if (!z->_a)
            z->_f |= FLG_Z;
        if (z->iff2)
            z->_f |= FLG_PV;

        return;
    }

    if (opcode == 0x5F){                                // LD A,R

        z->_a = z->_r;
        z->_f &= FLG_C;
        if (z->_a & 0x80)
            z->_f |= FLG_S;
        else
        if (!z->_a)
            z->_f |= FLG_Z;
        if (z->iff2)
            z->_f |= FLG_PV;

        return;
    }

    if (opcode == 0x47){                                // LD I,A

        z->_i = z->_a;
        return;
    }

    if (opcode == 0x4F){                                // LD R,A

        z->_r = z->_a;
        return;
    }

    if ((opcode & 0b11001111) == 0b01001011){           // LD dd,(nn)

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

        return;
    }

    if ((opcode & 0b11001111) == 0b01000011){           // LD (nn),dd

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

        return;
    }

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

        return;
    }

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

        return;
    }

    if (z->opcode == 0x44){                             // NEG

        z->_f &= ~(FLG_C|FLG_PV|FLG_H|FLG_Z|FLG_S);
        z->_f |= FLG_N;

        if (z->_a)
            z->_f |= FLG_C;

        if (z->_a == 0x80)
            z->_f |= FLG_PV;

        if (z->_a & 0x0F)   // Verifica se bits 0..3 são diferentes de zero
            z->_f |= FLG_H; // (causam borrow do bit 4 após o NEG).

        z->_a ^= 0xFF;
        z->_a++;

        if (z->_a & 0x80)
            z->_f |= FLG_S;

        if (!z->_a)
            z->_f |= FLG_Z;

        return;
    }

    if ((z->opcode & 0b11001111) == 0b01001010){        // ADC HL,ss

        int32_t arg1, arg2;

        arg1 = (int16_t)z->hl;

        switch(z->opcode & 0b00110000){

            case 0b00000000:
                arg2 = (int16_t)z->bc;
                break;
            case 0b00010000:
                arg2 = (int16_t)z->de;
                break;
            case 0b00100000:
                arg2 = (int16_t)z->hl;
                break;
            case 0b00110000:
                arg2 = (int16_t)z->sp;
                break;
        }

        z->_f &= ~(FLG_C|FLG_N|FLG_PV|FLG_H|FLG_Z|FLG_S); // Verificar cálculo de FLG_PV

        if (((arg1&0xFFF) + (arg2&0xFFF) + (z->_f & FLG_C ? 1:0)) & 0xF000)
            z->_f |= FLG_H;

        arg1 += arg2;
        if (z->_f & FLG_C)
            arg1++;

        z->hl = arg1 & 0xFFFF;

        if (arg1 & 0xFFFF0000)
            z->_f |= FLG_C;

        if (!z->hl)
            z->_f |= FLG_Z;

        if (arg1 & 0x8000)
            z->_f |= FLG_S;

        if ((arg1 > 32767) || (arg1 < -32768))
            z->_f |= FLG_PV;

        return;
    }

    if ((z->opcode & 0b11001111) == 0b01000010){        // SBC HL,ss

        int32_t arg1, arg2;

        arg1 = (int16_t)z->hl;

        switch(z->opcode & 0b00110000){

            case 0b00000000:
                arg2 = (int16_t)z->bc;
                break;
            case 0b00010000:
                arg2 = (int16_t)z->de;
                break;
            case 0b00100000:
                arg2 = (int16_t)z->hl;
                break;
            case 0b00110000:
                arg2 = (int16_t)z->sp;
                break;
        }

        z->_f &= ~(FLG_C|FLG_N|FLG_PV|FLG_H|FLG_Z|FLG_S); // Verificar cálculo de FLG_PV

        if (((arg1&0xFFF) - ((arg2&0xFFF) + (z->_f & FLG_C ? 1:0))) & 0xF000)
            z->_f |= FLG_H;

        arg1 -= arg2;
        if (z->_f & FLG_C)
            arg1--;

        z->hl = arg1 & 0xFFFF;

        if (arg1 & 0xFFFF0000)
            z->_f |= FLG_C;

        if (!z->hl)
            z->_f |= FLG_Z;

        if (arg1 & 0x8000)
            z->_f |= FLG_S;

        if ((arg1 > 32767) || (arg1 < -32768))
            z->_f |= FLG_PV;

        return;
    }

    if (z->opcode == 0x6F){                             // RLD

        z->code_prefix = 0;
        const uint8_t *orig = z80_get_phl_orig(z);
        uint8_t *dest = z80_get_phl_dest_last(z);

        uint8_t arg = 0xFF;
        if (orig != NULL)
            arg = *orig;

        uint8_t arg2 = arg << 4;
        arg2 |= (z->_a & 0x0F);
        z->_a &= 0xF0;
        z->_a |= (arg >> 4);
        if (dest != NULL)
            *dest = arg2;

        uint8_t oldf = z->_f & FLG_C;
        z80_update_flags_logic_acc(z,0);
        z->_f |= oldf;

        return;
    }

    if (z->opcode == 0x67){                             // RRD

        z->code_prefix = 0;
        const uint8_t *orig = z80_get_phl_orig(z);
        uint8_t *dest = z80_get_phl_dest_last(z);

        uint8_t arg = 0xFF;
        if (orig != NULL)
            arg = *orig;

        uint8_t arg2 = arg >> 4;
        arg2 |= (z->_a << 4);
        z->_a &= 0xF0;
        z->_a |= (arg & 0x0F);
        if (dest != NULL)
            *dest = arg2;

        uint8_t oldf = z->_f & FLG_C;
        z80_update_flags_logic_acc(z,0);
        z->_f |= oldf;
        return;
    }

    if (z->opcode == 0x4D){                              // RETI

        z->pc = z80_pop(z); // TODO: Estudar melhor como funciona
        return;
    }

    if (z->opcode == 0x45){                              // RETN

        z->iff1 = z->iff2;
        z->pc = z80_pop(z); // TODO: Estudar melhor como funciona
        return;
    }
}

////////////////////////////////////////////////////////////////////////////////
int z80_calc_conditional(z80_t *z){

    uint8_t cond = z->opcode & 0b00111000;

    uint8_t flags = z->_f;

    switch(cond){

    case 0b00000000:    //NZ
        if (flags & FLG_Z) return 0;
        return 1;

    case 0b00001000:    //Z
        if (flags & FLG_Z) return 1;
        return 0;

    case 0b00010000:    //NC
        if (flags & FLG_C) return 0;
        return 1;

    case 0b00011000:    //C
        if (flags & FLG_C) return 1;
        return 0;

    case 0b00100000:    //PO
        if (flags & FLG_PV) return 0;
        return 1;

    case 0b00101000:    //PE
        if (flags & FLG_PV) return 1;
        return 0;

    case 0b00110000:    //P
        if (flags & FLG_S) return 0;
        return 1;

    case 0b00111000:    //M
        if (flags & FLG_S) return 1;
        return 0;
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
int z80_exec_jmp(z80_t *z){

    uint16_t nextPC;

    switch(z->opcode){

    case 0xC3:          // JP
        nextPC = z80_fetch(z);
        nextPC |= (uint16_t)z80_fetch(z) << 8;
        z->pc = nextPC;
        break;

    case 0b11000010:    // JP NZ
    case 0b11001010:    // JP Z
    case 0b11010010:    // JP NC
    case 0b11011010:    // JP C
    case 0b11100010:    // JP PO
    case 0b11101010:    // JP PE
    case 0b11110010:    // JP P
    case 0b11111010:    // JP M
        nextPC = z80_fetch(z);
        nextPC |= (uint16_t)z80_fetch(z) << 8;

        if (z80_calc_conditional(z))
            z->pc = nextPC;
        break;

    case 0x18:          // JR
        nextPC = z80_fetch(z);
finish_jr:
        if (nextPC & 0x80)
            nextPC |= 0xFF00;   // Extensão de sinal
        z->pc += nextPC;
        break;

    case 0x38:          // JR C,
        nextPC = z80_fetch(z);
        if (z->_f & FLG_C) goto finish_jr;
        break;

    case 0x30:          // JR NC,
        nextPC = z80_fetch(z);
        if (z->_f & FLG_C) break;
        goto finish_jr;

    case 0x28:          // JR Z,
        nextPC = z80_fetch(z);
        if (z->_f & FLG_Z) goto finish_jr;
        break;

    case 0x20:          // JR NZ,
        nextPC = z80_fetch(z);
        if (z->_f & FLG_Z) break;
        goto finish_jr;

    case 0xE9:          // JP (HL) / JP (IX) / JP (IY)
        if (z->code_prefix & CODE_PREFIX_DD)
            z->pc = z->ix;
        else
        if (z->code_prefix & CODE_PREFIX_FD)
            z->pc = z->iy;
        else
            z->pc = z->hl;
        break;

    case 0x10:          // DJNZ
        nextPC = z80_fetch(z);
        z->_b--;
        if (!z->_b) break;
        goto finish_jr;


    default:
        return 0;
    }

    return 1;
}

////////////////////////////////////////////////////////////////////////////////
int z80_exec_call(z80_t *z){

    uint16_t nextPC;

    switch(z->opcode){

    case 0xCD:          // CALL
        nextPC = z80_fetch(z);
        nextPC |= (uint16_t)z80_fetch(z) << 8;
        z80_push(z, z->pc);
        z->pc = nextPC;
        break;

    case 0b11000100:    // CALL NZ
    case 0b11001100:    // CALL Z
    case 0b11010100:    // CALL NC
    case 0b11011100:    // CALL C
    case 0b11100100:    // CALL PO
    case 0b11101100:    // CALL PE
    case 0b11110100:    // CALL P
    case 0b11111100:    // CALL M
        nextPC = z80_fetch(z);
        nextPC |= (uint16_t)z80_fetch(z) << 8;

        if (z80_calc_conditional(z)){

            z80_push(z, z->pc);
            z->pc = nextPC;
        }
        break;

    case 0xC9:          // RET
        z->pc = z80_pop(z);
        break;

    case 0b11000000:    // RET NZ
    case 0b11001000:    // RET Z
    case 0b11010000:    // RET NC
    case 0b11011000:    // RET C
    case 0b11100000:    // RET PO
    case 0b11101000:    // RET PE
    case 0b11110000:    // RET P
    case 0b11111000:    // RET M
        if (z80_calc_conditional(z))
            z->pc = z80_pop(z);
        break;

    case 0b11000111:    // RST 00h
    case 0b11001111:    // RST 08h
    case 0b11010111:    // RST 10h
    case 0b11011111:    // RST 18h
    case 0b11100111:    // RST 20h
    case 0b11101111:    // RST 28h
    case 0b11110111:    // RST 30h
    case 0b11111111:    // RST 38h
        z80_push(z, z->pc);
        z->pc = z->opcode & 0x38;
        break;

    default:
        return 0;
    }

    return 1;
}

////////////////////////////////////////////////////////////////////////////////
int z80_exec_undoc_dd_fd(z80_t *z){

    switch(z->opcode){


    }



    return 1;
}

////////////////////////////////////////////////////////////////////////////////
void z80_step_in(z80_t *z){

rescan:
    z->opcode = z80_fetch(z);

    ////////////////////////////////////////////////////////////////////////////
    if (z->opcode == 0xDD){                             //DD Prefix (IX)

        z->code_prefix |= CODE_PREFIX_DD;
        z->code_prefix &= ~CODE_PREFIX_FD;
        goto rescan;
    }

    ////////////////////////////////////////////////////////////////////////////
    if (z->opcode == 0xFD){                             //FD Prefix (IY)

        z->code_prefix |= CODE_PREFIX_FD;
        z->code_prefix &= ~CODE_PREFIX_DD;
        goto rescan;
    }

    ////////////////////////////////////////////////////////////////////////////
    if (z->opcode == 0xCB){                             //CB Prefix (bit ops)

        z->code_prefix |= CODE_PREFIX_CB;
        z->code_prefix &= ~CODE_PREFIX_ED;
        goto rescan;
    }

    ////////////////////////////////////////////////////////////////////////////
    if (z->opcode == 0xED){                             //ED Prefix (specials)

        z->code_prefix |= CODE_PREFIX_ED;
        z->code_prefix &= ~CODE_PREFIX_CB;
        goto rescan;
    }

    z80_refresh_up(z);

    ////////////////////////////////////////////////////////////////////////////
    if (z->code_prefix & CODE_PREFIX_CB){

        z80_exec_cb(z);
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if (z->code_prefix & CODE_PREFIX_ED){

        z80_exec_ed(z);
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if (z80_exec_jmp(z))
        return;

    ////////////////////////////////////////////////////////////////////////////
    if (z80_exec_call(z))
        return;

    ////////////////////////////////////////////////////////////////////////////
    if ((z->opcode & 0b11000111) == 0b00000110){        // LD r,n

        if (z->opcode == 0x36){                         // LD (HL),n / LD (IX+d),n / LD (IY+d),n

            uint8_t *pdest = z80_get_dest(z, 0);
            uint8_t arg = z80_fetch(z);
            if (pdest){

                *pdest = arg;
            }
            return;
        }

        *z80_get_dest(z,1) = z80_fetch(z);
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
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
            case 0b00100000:                            // LD HL,nn / LD IX,nn / LD IY,nn
                if (z->code_prefix & CODE_PREFIX_DD)
                    z->ix = argh;
                else
                if (z->code_prefix & CODE_PREFIX_FD)
                    z->iy = argh;
                else
                    z->hl = argh;
                break;
            case 0b00110000:
                z->sp = argh;
                break;
        }
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
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
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
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
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
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
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if ((z->opcode & 0b11001111) == 0b11000101){        // PUSH qq

        uint16_t arg;
        switch(z->opcode & 0b00110000){

            case 0b00000000:
                arg = z->bc;
                break;
            case 0b00010000:
                arg = z->de;
                break;
            case 0b00100000:
                if (z->code_prefix & CODE_PREFIX_DD)
                    arg = z->ix;
                else
                if (z->code_prefix & CODE_PREFIX_FD)
                    arg = z->iy;
                else
                    arg = z->hl;
                break;
            case 0b00110000:
                arg = z->af;
                break;
        }

        z80_push(z, arg);
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if ((z->opcode & 0b11001111) == 0b11000001){        // POP qq

        uint16_t arg = z80_pop(z);
        switch(z->opcode & 0b00110000){

            case 0b00000000:
                z->bc = arg;
                break;
            case 0b00010000:
                z->de = arg;
                break;
            case 0b00100000:
                if (z->code_prefix & CODE_PREFIX_DD)
                    z->ix = arg;
                else
                if (z->code_prefix & CODE_PREFIX_FD)
                    z->iy = arg;
                else
                    z->hl = arg;
                break;
            case 0b00110000:
                z->af = arg;
                break;
        }
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
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
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if ((z->opcode & 0b11110000) == 0b10000000){        // ADD A,r / ADC A,r

        if (z->opcode == 0x86){                         // ADD A,(HL) / ADD A,(IX+d) / ADD A,(IY+d)

            uint8_t arg;
            const uint8_t *parg = z80_get_phl_orig(z);
            if (parg != NULL)
                arg = *parg;
            else
                arg = 0xff;
            z80_add_acc(z, arg, 0);
            return;
        }

        if (z->opcode == 0x8E){                         // ADC A,(HL) / ADC A,(IX+d) / ADC A,(IY+d)

            uint8_t arg;
            const uint8_t *parg = z80_get_phl_orig(z);
            if (parg != NULL)
                arg = *parg;
            else
                arg = 0xff;
            z80_add_acc(z, arg, 1);
            return;
        }

        uint8_t *arg = z80_get_reg8_ptr(z,1);
        z80_add_acc(z, *arg, z->opcode & 0x08 ? 1:0);   // Seleciona ADD ou ADC
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if ((z->opcode & 0b11110000) == 0b10010000){        // SUB r / SBC A,r

        if (z->opcode == 0x96){                         // SUB (HL) / SUB (IX+d) / SUB (IY+d)

            uint8_t arg;
            const uint8_t *parg = z80_get_phl_orig(z);
            if (parg != NULL)
                arg = *parg;
            else
                arg = 0xff;
            z80_sub_acc(z, arg, 0);
            return;
        }

        if (z->opcode == 0x9E){                         // SBC A,(HL) / SBC A,(IX+d) / SBC A,(IY+d)

            uint8_t arg;
            const uint8_t *parg = z80_get_phl_orig(z);
            if (parg != NULL)
                arg = *parg;
            else
                arg = 0xff;
            z80_sub_acc(z, arg, 1);
            return;
        }

        uint8_t *arg = z80_get_reg8_ptr(z,1);
        z80_sub_acc(z, *arg, z->opcode & 0x08 ? 1:0);    // Seleciona SUB ou SBC
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if ((z->opcode & 0b11111000) == 0b10100000){        // AND r

        if (z->opcode == 0xA6){                         // AND (HL) / AND (IX+d) / AND (IY+d)

            uint8_t arg;
            const uint8_t *parg = z80_get_phl_orig(z);
            if (parg != NULL)
                arg = *parg;
            else
                arg = 0xff;
            z->_a &= arg;
            z80_update_flags_logic_acc(z,1);
            return;
        }


        uint8_t *arg = z80_get_reg8_ptr(z,1);
        z->_a &= *arg;
        z80_update_flags_logic_acc(z,1);
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if ((z->opcode & 0b11111000) == 0b10110000){        // OR r

        if (z->opcode == 0xB6){                         // OR (HL) / OR (IX+d) / OR (IY+d)

            uint8_t arg;
            const uint8_t *parg = z80_get_phl_orig(z);
            if (parg != NULL)
                arg = *parg;
            else
                arg = 0xff;
            z->_a |= arg;
            z80_update_flags_logic_acc(z,0);
            return;
        }

        uint8_t *arg = z80_get_reg8_ptr(z,1);
        z->_a |= *arg;
        z80_update_flags_logic_acc(z,0);
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if ((z->opcode & 0b11111000) == 0b10101000){        // XOR r

        if (z->opcode == 0xAE){                         // XOR (HL) / XOR (IX+d) / XOR (IY+d)

            uint8_t arg;
            const uint8_t *parg = z80_get_phl_orig(z);
            if (parg != NULL)
                arg = *parg;
            else
                arg = 0xff;
            z->_a ^= arg;
            z80_update_flags_logic_acc(z,0);
            return;
        }

        uint8_t *arg = z80_get_reg8_ptr(z,1);
        z->_a ^= *arg;
        z80_update_flags_logic_acc(z,0);
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if ((z->opcode & 0b11111000) == 0b10111000){        // CP r

        if (z->opcode == 0xBE){                         // CP (HL) / CP (IX+d) / CP (IY+d)

            uint8_t arg;
            const uint8_t *parg = z80_get_phl_orig(z);
            if (parg != NULL)
                arg = *parg;
            else
                arg = 0xff;
            uint8_t asave = z->_a;
            z80_sub_acc(z, arg, 0);
            z->_a = asave;
            return;
        }

        uint8_t *arg = z80_get_reg8_ptr(z,1);
        uint8_t asave = z->_a;
        z80_sub_acc(z, *arg, 0);
        z->_a = asave;
        z->_f &= ~0x28;
        z->_f |= (*arg & 0x28);
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if ((z->opcode & 0b11000111) == 0b00000100){        // INC r

        uint8_t arg;

        if (z->opcode == 0x34){                         // INC (HL) / INC (IX+d) / INC (IY+d)

            const uint8_t *parg = z80_get_phl_orig(z);
            uint8_t *parg2 = z80_get_phl_dest_last(z);

            if (parg != NULL)
                arg = *parg;
            else
                arg = 0xff;
            arg++;
            if (parg2)
                *parg2 = arg;

            goto _inc_r_flags;
        }

        uint8_t *parg = z80_get_dest(z,1);
        arg = *parg;
        arg++;
        *parg = arg;

_inc_r_flags:
        z->_f = (arg & 0x28) | (z->_f & FLG_C);
        if (arg & 0x80)
            z->_f |= FLG_S;
        if (!arg)
            z->_f |= FLG_Z;
        if (!(arg&0x0F))
            z->_f |= FLG_H;
        if (arg == 0x80)
            z->_f |= FLG_PV;
        //já está zerada z->_f &= ~FLG_N;
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if ((z->opcode & 0b11000111) == 0b00000101){        // DEC r

        uint8_t arg;

        if (z->opcode == 0x35){                         // DEC (HL) / DEC (IX+d) / DEC (IY+d)

            const uint8_t *parg = z80_get_phl_orig(z);
            uint8_t *parg2 = z80_get_phl_dest_last(z);

            if (parg != NULL)
                arg = *parg;
            else
                arg = 0xff;

            arg--;

            if (parg2)
                *parg2 = arg;

            goto _dec_r_flags;
        }

        uint8_t *parg = z80_get_dest(z,1);
        arg = *parg;
        arg--;
        *parg = arg;

_dec_r_flags:
        z->_f = (arg & 0x28) | (z->_f & FLG_C);
        if (arg & 0x80)
            z->_f |= FLG_S;
        if (!arg)
            z->_f |= FLG_Z;
        if ((arg&0x0F) == 0x0F)
            z->_f |= FLG_H;
        if (arg == 0x7F)
            z->_f |= FLG_PV;
        z->_f |= FLG_N;
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if ((z->opcode & 0b11000111) == 0b00000011){        // INC ss / DEC ss

        switch(z->opcode){
            case 0b00000011:
                z->bc++;
                break;
            case 0b00010011:
                z->de++;
                break;
            case 0b00100011:                            // INC HL / INC IX / INC IY
                if (z->code_prefix & CODE_PREFIX_DD)
                    z->ix++;
                else
                if (z->code_prefix & CODE_PREFIX_FD)
                    z->iy++;
                else
                    z->hl++;
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
            case 0b00101011:                            // DEC HL / DEC IX / DEC IY
                if (z->code_prefix & CODE_PREFIX_DD)
                    z->ix--;
                else
                if (z->code_prefix & CODE_PREFIX_FD)
                    z->iy--;
                else
                    z->hl--;
                break;
            case 0b00111011:
                z->sp--;
                break;
        }
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if ((z->opcode & 0b11001111) == 0b00001001){        // ADD HL,ss / ADD IX,ss / ADD IY,ss

        uint32_t arg1, arg2;
        if (z->code_prefix & CODE_PREFIX_DD)
            arg1 = z->ix;
        else
        if (z->code_prefix & CODE_PREFIX_FD)
            arg1 = z->iy;
        else
            arg1 = z->hl;

        switch(z->opcode & 0b00110000){

            case 0b00000000:
                arg2 = z->bc;
                break;
            case 0b00010000:
                arg2 = z->de;
                break;
            case 0b00100000:
                if (z->code_prefix & CODE_PREFIX_DD)
                    arg2 = z->ix;
                else
                if (z->code_prefix & CODE_PREFIX_FD)
                    arg2 = z->iy;
                else
                    arg2 = z->hl;
                break;
            case 0b00110000:
                arg2 = z->sp;
                break;
        }

        z->_f &= ~(FLG_N|FLG_H|FLG_C);

        if (((arg1&0xFFF) + (arg2&0xFFF)) & 0xF000)
            z->_f |= FLG_H;

        arg1 += arg2;

        if (arg1 & 0xFFFF0000)
            z->_f |= FLG_C;

        if (z->code_prefix & CODE_PREFIX_DD)
            z->ix = arg1;
        else
        if (z->code_prefix & CODE_PREFIX_FD)
            z->iy = arg1;
        else
            z->hl = arg1;
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if ((z->opcode & 0b11000000) == 0b01000000){        // LD r,r' e HALT

        if (z->opcode == 0x76) {                        // Caso especial 0x76: HALT

            --z->pc;
            z->halted = 1;
            return;
        }

        if ((z->opcode & 0b11000111) == 0b01000110){    // LD r,(HL) / LD r,(IX+d) / LD r,(IY+d)

            const uint8_t *porig = z80_get_orig(z,0);
            uint8_t arg = 0xFF;
            if (porig)
                arg = *porig;
            *z80_get_dest(z, 0) = arg;
            return;
        }

        if ((z->opcode & 0b11111000) == 0b01110000){    // LD (HL),r / LD (IX+d),r / LD (IY+d),r

            uint8_t *pdest = z80_get_dest(z,0);
            if (pdest)
                *pdest = *z80_get_orig(z,0);
            return;
        }

        *z80_get_dest(z,1) = *z80_get_orig(z,1);        // LD r,r'
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if (z->code_prefix){    // Nesta altura o code_prefix pode conter apenas DD e FD (CB e ED já foram eliminados)

        if (z80_exec_undoc_dd_fd(z))
            return;
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    if (z->code_prefix){    // Elimina códigos DD e FD inválidos

        printf("Invalid IX/IY prefix\n");
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if (!z->opcode){   //NOP

        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if (z->opcode == 0x0A){                             // LD A,(BC)

        z->_a = z80_read(z, z->bc);
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if (z->opcode == 0x1A){                             // LD A,(DE)

        z->_a = z80_read(z, z->de);
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if (z->opcode == 0x3A){                             // LD A,(nn)

        uint16_t addrl = z80_fetch(z);
        uint16_t addrh = z80_fetch(z);
        z->_a = z80_read(z, addrh<<8 | addrl);
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if (z->opcode == 0x02){                             // LD (BC),A

        z80_write(z, z->bc, z->_a);
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if (z->opcode == 0x12){                             // LD (DE),A

        z80_write(z, z->de, z->_a);
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if (z->opcode == 0x32){                             // LD (nn),A

        uint16_t addrl = z80_fetch(z);
        uint16_t addrh = z80_fetch(z);
        z80_write(z, addrh<<8 | addrl, z->_a);
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if (z->opcode == 0x08){                             // EX AF,AF'

        uint16_t temp = z->af;
        z->af = z->afa;
        z->afa = temp;
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
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

    ////////////////////////////////////////////////////////////////////////////
    if (z->opcode == 0xC6){                             // ADD A,n

        uint8_t arg = z80_fetch(z);
        z80_add_acc(z, arg, 0);
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if (z->opcode == 0xCE){                             // ADC A,n

        uint8_t arg = z80_fetch(z);
        z80_add_acc(z, arg, 1);
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if (z->opcode == 0xD6){                             // SUB n

        uint8_t arg = z80_fetch(z);
        z80_sub_acc(z, arg, 0);
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if (z->opcode == 0xDE){                             // SBC A,n

        uint8_t arg = z80_fetch(z);
        z80_sub_acc(z, arg, 1);
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if (z->opcode == 0xE6){                             // AND n

        uint8_t arg = z80_fetch(z);
        z->_a &= arg;
        z80_update_flags_logic_acc(z,1);
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if (z->opcode == 0xF6){                             // OR n

        uint8_t arg = z80_fetch(z);
        z->_a |= arg;
        z80_update_flags_logic_acc(z,0);
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if (z->opcode == 0xEE){                             // XOR n

        uint8_t arg = z80_fetch(z);
        z->_a ^= arg;
        z80_update_flags_logic_acc(z,0);
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if (z->opcode == 0xFE){                             // CP n

        uint8_t arg = z80_fetch(z);
        uint8_t a = z->_a;
        z80_sub_acc(z, arg, 0);
        z->_a = a;
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if (z->opcode == 0xEB){                             // EX DE,HL

        uint16_t temp = z->hl;
        z->hl = z->de;
        z->de = temp;
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if (z->opcode == 0x37){                             // SCF

        z->_f &= ~(FLG_H|FLG_N);
        z->_f |= FLG_C;
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if (z->opcode == 0x3F){                             // CCF

        z->_f &= ~(FLG_H|FLG_N);

        if (z->_f & FLG_C)
            z->_f |= FLG_H;

        z->_f ^= FLG_C;
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if (z->opcode == 0x27){                             // DAA

        uint8_t ah = z->_a >> 4;
        uint8_t al = z->_a & 0x0f;

        typedef struct{
            uint8_t nf_before;
            uint8_t cf_before;
            uint8_t ah_min,ah_max;
            uint8_t hf_before;
            uint8_t al_min,al_max;
            uint8_t byte_to_add;
            uint8_t cf_after;
        } daa_table_t;

        const daa_table_t daa_table[] = {

            { 0, 0, 0x00, 0x09, 0, 0x00, 0x09, 0x00, 0},
            { 0, 0, 0x00, 0x08, 0, 0x0a, 0x0f, 0x06, 0},
            { 0, 0, 0x00, 0x09, 1, 0x00, 0x03, 0x06, 0},
            { 0, 0, 0x0a, 0x0f, 0, 0x00, 0x09, 0x60, 1},
            { 0, 0, 0x09, 0x0f, 0, 0x0a, 0x0f, 0x66, 1},
            { 0, 0, 0x0a, 0x0f, 1, 0x00, 0x03, 0x66, 1},
            { 0, 1, 0x00, 0x02, 0, 0x00, 0x09, 0x60, 1},
            { 0, 1, 0x00, 0x02, 0, 0x0a, 0x0f, 0x66, 1},
            { 0, 1, 0x00, 0x03, 1, 0x00, 0x03, 0x66, 1},
            { 1, 0, 0x00, 0x09, 0, 0x00, 0x09, 0x00, 0},
            { 1, 0, 0x00, 0x08, 1, 0x06, 0x0f, 0xfa, 0},
            { 1, 1, 0x07, 0x0f, 0, 0x00, 0x09, 0xa0, 1},
            { 1, 1, 0x06, 0x0f, 1, 0x06, 0x0f, 0x9a, 1}
        };

        for (int i = 0; i < 12; i++){

            if ( (((z->_f & FLG_N)?1:0) == daa_table[i].nf_before)
            &&
                 (((z->_f & FLG_C)?1:0) == daa_table[i].cf_before)
            &&
                 (((z->_f & FLG_H)?1:0) == daa_table[i].hf_before)
            ){
                if ( (ah >= daa_table[i].ah_min) && (ah <= daa_table[i].ah_max)
                &&
                     (al >= daa_table[i].al_min) && (al <= daa_table[i].al_max)
                ){
                    z->_a += daa_table[i].byte_to_add;

                    int flg_h = 0;
                    if ((z->_a >> 4) != ah)
                        flg_h = 1;

                    z80_update_flags_logic_acc(z, flg_h);

                    if (daa_table[i].cf_after)
                        z->_f |= FLG_C;
                    else
                        z->_f &= ~FLG_C;

                    return;
                }
            }
        }

        z80_update_flags_logic_acc(z, 0);   // Se não processou o DAA, atualiza os flags assim mesmo.
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if (z->opcode == 0x2F){                             // CPL

        z->_f |= (FLG_H|FLG_N);

        z->_a ^= 0xFF;
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if (z->opcode == 0x07){                             // RLCA

        z->_f &= ~(FLG_H|FLG_N|FLG_C);

        uint8_t acopy = z->_a;
        z->_a <<= 1;

        if (acopy & 0x80){
            z->_f |= FLG_C;
            z->_a |= 0x01;
        }
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if (z->opcode == 0x17){                             // RLA

        uint8_t cy = z->_f & FLG_C;
        z->_f &= ~(FLG_H|FLG_N|FLG_C);

        if (z->_a & 0x80)
            z->_f |= FLG_C;

        z->_a <<= 1;

        if (cy){
            z->_a |= 0x01;
        }
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if (z->opcode == 0x0F){                             // RRCA

        z->_f &= ~(FLG_H|FLG_N|FLG_C);

        uint8_t acopy = z->_a;
        z->_a >>= 1;

        if (acopy & 0x01){
            z->_f |= FLG_C;
            z->_a |= 0x80;
        }
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    if (z->opcode == 0x1F){                             // RRA

        uint8_t cy = z->_f & FLG_C;
        z->_f &= ~(FLG_H|FLG_N|FLG_C);

        if (z->_a & 0x01)
            z->_f |= FLG_C;

        z->_a >>= 1;

        if (cy){
            z->_a |= 0x80;
        }
        return;
    }

    printf("Unk. Opcode\n");
}

void z80_step(z80_t *z){

    z80_step_in(z);
    z->code_prefix = 0;
}

////////////////////////////////////////////////////////////////////////////////
void z80_dump(z80_t *z){

    printf("\nPC:%04X  SP:%04X  BC:%04x  DE:%04X  HL:%04X  IX:%04X  IY:%04X  AF:%04X   FLAGS:",
            z->pc,z->sp,z->bc,z->de,z->hl,z->ix,z->iy,z->af);

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

