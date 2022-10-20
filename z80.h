/*
 * z80.h
 *
 *  Created on: 13 de out de 2022
 *      Author: milton
 */

#ifndef Z80_H_
#define Z80_H_

#include <stdint.h>

#define FLG_S 0x80
#define FLG_Z 0x40
#define FLG_H 0x10
#define FLG_PV 0x04
#define FLG_N 0x02
#define FLG_C 0x01

#define CODE_PREFIX_DD 0x08
#define CODE_PREFIX_FD 0x04
#define CODE_PREFIX_CB 0x02
#define CODE_PREFIX_ED 0x01

typedef union {
        struct {
            uint8_t l;
            uint8_t h;
        } r8;
        uint16_t r16;
    } reg_t;

typedef struct {

    uint16_t pc;
    uint16_t sp;
    uint16_t ix;
    uint16_t iy;

    reg_t _ir;
    reg_t _bc;
    reg_t _de;
    reg_t _hl;
    reg_t _af;

    reg_t _bca;
    reg_t _dea;
    reg_t _hla;
    reg_t _afa;

    uint8_t iff1;
    uint8_t iff2;
    uint8_t im;

    uint8_t opcode;
    uint8_t code_prefix;

    uint8_t halted;

    const uint8_t *rom;
    uint8_t *ram;
    uint16_t romsz;
    uint16_t rambase;
    uint16_t ramsz;
    uint16_t ramend;

#define bc _bc.r16
#define de _de.r16
#define hl _hl.r16
#define af _af.r16

#define bca _bca.r16
#define dea _dea.r16
#define hla _hla.r16
#define afa _afa.r16

#define _b _bc.r8.h
#define _c _bc.r8.l

#define _d _de.r8.h
#define _e _de.r8.l

#define _h _hl.r8.h
#define _l _hl.r8.l

#define _a _af.r8.h
#define _f _af.r8.l

#define _i _ir.r8.h
#define _r _ir.r8.l

} z80_t;

void z80_initialize(z80_t *z, const uint8_t *rom, uint16_t romsz, uint8_t *ram, uint16_t rambase, uint16_t ramsz);
void z80_reset (z80_t *z);
void z80_step(z80_t *z);
void z80_dump(z80_t *z);

#endif /* Z80_H_ */
