/*
 * romprog.c
 *
 *  Created on: 15 de out de 2022
 *      Author: milton
 */


#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "romprog.h"

#define DEBUG 0

uint16_t pc;
uint8_t bufwrite[5];
uint8_t nbufwrite = 0;

////////////////////////////////////////////////////////////////////////////////
const char *src[]={
        "org 0x0000",
        "nop",
        "ld sp,0x4000",
        "ld a,1",
        "ld b,a",
        "ld a,(hl)",
        "ld a,(ix)",
        "ld a,(ix+3)",
        "ld a,(ix-3)",
        "ld a,(iy)",
        "ld a,(iy+2)",
        "ld a,(iy-2)",
        "ld a,(10000)",
        "ld de,1234",
        "ld de,(1234)",
        "ld hl,(16396)",
        "ld (1234),hl",
        "push ix",
        "pop de",
        "halt",
        NULL
        };

////////////////////////////////////////////////////////////////////////////////
int parse_immediate8(const char *s, uint8_t *dst){

    uint16_t res = 0;
    if (!strncmp(s,"0X",2)){    //Hex

        s += 2;
        if (!isxdigit(*s))
            return -1;

        while(*s){

            if (!isxdigit(*s))
                return -1;
            res *= 16;
            if (isdigit(*s))
                res += (*s - '0');
            else
                res += ((toupper(*s) - 'A') + 10);
            if (res & 0xFF00)// Overflow
                return -1;
            s++;
        }
        *dst = res&0xFF;
        return 0;
    }
    else
    if (!strncmp(s,"0",1)){     //Octal

        while(*s){

            if (!isdigit(*s))
                return -1;
            if (*s > '7')
                return -1;
            res *= 8;
            res += (*s - '0');
            if (res&0xFF00)// Overflow
                return -1;
            s++;
        }
        *dst = res&0xFF;
        return 0;
    }
    else
    if (isdigit(*s)){

        while(*s){

            if (!isdigit(*s))
                return -1;
            res *= 10;
            res += (*s - '0');
            if (res&0xFF00)// Overflow
                return -1;
            s++;
        }
        *dst = res&0xFF;
        return 0;
    }

    return -1;
}

////////////////////////////////////////////////////////////////////////////////
int parse_immediate16(const char *s, uint16_t *dst){

    uint16_t res = 0;
    uint16_t resold = 0;
    if (!strncmp(s,"0X",2)){    //Hex

        s += 2;
        if (!isxdigit(*s))
            return -1;

        while(*s){

            if (!isxdigit(*s))
                return -1;
            resold = res;
            res *= 16;
            if (isdigit(*s))
                res += (*s - '0');
            else
                res += ((toupper(*s) - 'A') + 10);
            if (resold > res)// Overflow
                return -1;
            s++;
        }
        *dst = res;
        return 0;
    }
    else
    if (!strncmp(s,"0",1)){     //Octal

        while(*s){

            if (!isdigit(*s))
                return -1;
            if (*s > '7')
                return -1;
            resold = res;
            res *= 8;
            res += (*s - '0');
            if (resold > res)// Overflow
                return -1;
            s++;
        }
        *dst = res;
        return 0;
    }
    else
    if (isdigit(*s)){

        while(*s){

            if (!isdigit(*s))
                return -1;
            resold = res;
            res *= 10;
            res += (*s - '0');
            if (resold > res)// Overflow
                return -1;
            s++;
        }
        *dst = res;
        return 0;
    }

    return -1;
}

////////////////////////////////////////////////////////////////////////////////
int parse_immediate16_paren(const char *s, uint16_t *dst){

    if (s[0] != '(') return -1;
    int len = strlen(s);
    if (len<3) return -1;
    if (s[len-1] != ')') return -1;

    char buf[10];
    memcpy(buf,s+1,len-2);
    buf[len-2] = 0;

    return parse_immediate16(buf, dst);
}

////////////////////////////////////////////////////////////////////////////////
const char *reg8[] = {"B","C","D","E","H","L","","A",NULL};
const char *reg16[] = {"BC","DE","HL","SP",NULL};
const char *reg16a[] = {"BC","DE","HL","AF",NULL};

int get_index(char *s, const char *table[]){

    int i = 0;

    while(table[i]){

        if (!strcmp(table[i],s))
            return i;
        i++;
    }

    return -1;
}

////////////////////////////////////////////////////////////////////////////////
int parse_ixy_d(char *op, uint8_t *dst){

    if (!op[0]) return -1;

    if (!strcmp(op,")")){

        *dst = 0;
        return 0;
    }

    int sig = 0;
    if (op[0] == '+')
        sig = 0;
    else
    if (op[0] == '-')
        sig = 1;
    else
        return -1;

    op++;

    int l = strlen(op);
    if (op[l-1] != ')')
        return -1;

    char buf[10];
    memcpy(buf,op,l-1);
    buf[l-1] = 0;

    uint8_t res;
    if (parse_immediate8(buf, &res) < 0)
        return -1;

    if (sig){
        if (res > 128) return -1;
        res ^= 0xFF;
        res++;
        *dst = res;
        return 0;
    }
    else{
        if (res > 127) return -1;
        *dst = res;
        return 0;
    }

    return -1;
}

////////////////////////////////////////////////////////////////////////////////
int proc_ld (char *op1, char *op2){

    int index1 = get_index(op1,reg8);
    int index2 = get_index(op2,reg8);
    uint8_t dst;
    uint8_t imm;
    uint16_t dst16;
    uint16_t imm16;

    if (index1 >= 0){

        if (index2 >= 0){                       // LD r,r'

            bufwrite[0] = 0b01000000 | (index1 << 3) | index2;
            nbufwrite = 1;
            return 0;
        }
        else{   // index2 < 0

            if (parse_immediate8(op2, &imm) >= 0){  // LD r,n

                bufwrite[0] = 0b00000110 | (index1 << 3);
                bufwrite[1] = imm;
                nbufwrite = 2;
                return 0;
            }
            else
            if (!strcmp(op2,"(HL)")){               // LD r,(HL)
                bufwrite[0] = 0b01000110 | (index1 << 3);
                nbufwrite = 1;
                return 0;
            }
            else
            if (!strncmp(op2,"(IX",3)){             // LD r,(IX+d)

                if (parse_ixy_d(op2+3,&dst)<0)
                    return -1;
                bufwrite[0] = 0xDD;
                bufwrite[1] = 0b01000110 | (index1 << 3);
                bufwrite[2] = dst;
                nbufwrite = 3;
                return 0;
            }
            else
            if (!strncmp(op2,"(IY",3)){             // LD r,(IY+d)

                if (parse_ixy_d(op2+3,&dst)<0)
                    return -1;
                bufwrite[0] = 0xFD;
                bufwrite[1] = 0b01000110 | (index1 << 3);
                bufwrite[2] = dst;
                nbufwrite = 3;
                return 0;
            }
            else
            if (index1 == 7){   // Reg a

                if (parse_immediate16_paren(op2,&dst16)>=0){   // LD A,(nn)
                    bufwrite[0] = 0x3a;
                    bufwrite[1] = dst16&0xFF;
                    bufwrite[2] = dst16>>8;
                    nbufwrite = 3;
                    return 0;
                }
            }
        }

        return -1;
    }
    else{   //index1 < 0
        if (index2 >= 0){

            if (!strcmp(op1,"(HL)")){               // LD (HL),r
                bufwrite[0] = 0b01110000 | index2;
                nbufwrite = 1;
                return 0;
            }
            else
            if (!strncmp(op1,"(IX",3)){             // LD (IX+d),r
                if (parse_ixy_d(op1+3,&dst)<0)
                    return -1;
                bufwrite[0] = 0xDD;
                bufwrite[1] = 0b01110000 | index2;
                bufwrite[2] = dst;
                nbufwrite = 3;
                return 0;
            }
            else
            if (!strncmp(op1,"(IY",3)){             // LD (IY+d),r
                if (parse_ixy_d(op1+3,&dst)<0)
                    return -1;
                bufwrite[0] = 0xFD;
                bufwrite[1] = 0b01110000 | index2;
                bufwrite[2] = dst;
                nbufwrite = 3;
                return 0;
            }
            else
            if (index2 == 7){

                if (parse_immediate16_paren(op1,&dst16)>=0){   // LD (nn),A
                    bufwrite[0] = 0x32;
                    bufwrite[1] = dst16&0xFF;
                    bufwrite[2] = dst16>>8;
                    nbufwrite = 3;
                    return 0;
                }
            }

            return -1;
        }
        else{   //index2 < 0

            if (parse_immediate8(op2, &imm) >= 0){

                if (!strcmp(op1,"(HL)")){               // LD (HL),n
                    bufwrite[0] = 0b00110110;
                    bufwrite[1] = imm;
                    nbufwrite = 2;
                    return 0;
                }
                else
                if (!strncmp(op1,"(IX",3)){             // LD (IX+d),n
                    if (parse_ixy_d(op1+3,&dst)<0)
                        return -1;
                    bufwrite[0] = 0xDD;
                    bufwrite[1] = 0b00110110;
                    bufwrite[2] = dst;
                    bufwrite[3] = imm;
                    nbufwrite = 4;
                    return 0;
                }
                else
                if (!strncmp(op1,"(IY",3)){             // LD (IY+d),n
                    if (parse_ixy_d(op1+3,&dst)<0)
                        return -1;
                    bufwrite[0] = 0xFD;
                    bufwrite[1] = 0b00110110;
                    bufwrite[2] = dst;
                    bufwrite[3] = imm;
                    nbufwrite = 4;
                    return 0;
                }
            }

            ///// LD 16 bits

            if (parse_immediate16(op2, &imm16) >= 0){

                int index1 = get_index(op1,reg16);

                if (index1 >= 0){                   // LD dd,nn

                    bufwrite[0] = 0b00000001 | (index1<<4);
                    bufwrite[1] = imm16&0xFF;
                    bufwrite[2] = imm16>>8;
                    nbufwrite = 3;
                    return 0;
                }
                else
                if (!strcmp(op1,"IX")){             // LD IX,nn

                    bufwrite[0] = 0xDD;
                    bufwrite[1] = 0x21;
                    bufwrite[2] = imm16&0xFF;
                    bufwrite[3] = imm16>>8;
                    nbufwrite = 4;
                    return 0;
                }
                else
                if (!strcmp(op1,"IY")){             // LD IY,nn

                    bufwrite[0] = 0xFD;
                    bufwrite[1] = 0x21;
                    bufwrite[2] = imm16&0xFF;
                    bufwrite[3] = imm16>>8;
                    nbufwrite = 4;
                    return 0;
                }

                return -1;
            }
            else
            if (parse_immediate16_paren(op2, &dst16) >= 0){

                int index1 = get_index(op1,reg16);

                if (!strcmp(op1,"HL")){             // LD HL,(nn)

                    bufwrite[0] = 0x2A;
                    bufwrite[1] = dst16&0xFF;
                    bufwrite[2] = dst16>>8;
                    nbufwrite = 3;
                    return 0;
                }
                else
                if (index1 >= 0){

                    bufwrite[0] = 0xED;
                    bufwrite[1] = 0b01001011 | (index1 << 4);
                    bufwrite[2] = dst16&0xFF;
                    bufwrite[3] = dst16>>8;
                    nbufwrite = 4;
                    return 0;
                }
                else
                if (!strcmp(op1,"IX")){             // LD IX,(nn)

                    bufwrite[0] = 0xDD;
                    bufwrite[1] = 0x2A;
                    bufwrite[2] = dst16&0xFF;
                    bufwrite[3] = dst16>>8;
                    nbufwrite = 4;
                    return 0;
                }
                else
                if (!strcmp(op1,"IY")){             // LD IY,(nn)

                    bufwrite[0] = 0xFD;
                    bufwrite[1] = 0x2A;
                    bufwrite[2] = dst16&0xFF;
                    bufwrite[3] = dst16>>8;
                    nbufwrite = 4;
                    return 0;
                }
                return -1;
            }
            else
            if (parse_immediate16_paren(op1, &dst16) >= 0){

                int index2 = get_index(op2,reg16);

                if (!strcmp(op2,"HL")){             // LD (nn),HL

                    bufwrite[0] = 0x22;
                    bufwrite[1] = dst16&0xFF;
                    bufwrite[2] = dst16>>8;
                    nbufwrite = 3;
                    return 0;
                }
                else
                if (index2 >= 0){                   // LD (nn),dd

                    bufwrite[0] = 0xED;
                    bufwrite[1] = 0b01000011 | (index2 << 4);
                    bufwrite[2] = dst16&0xFF;
                    bufwrite[3] = dst16>>8;
                    nbufwrite = 4;
                    return 0;
                }
                else
                if (!strcmp(op2,"IX")){             // LD (nn),IX

                    bufwrite[0] = 0xDD;
                    bufwrite[1] = 0x22;
                    bufwrite[2] = dst16&0xFF;
                    bufwrite[3] = dst16>>8;
                    nbufwrite = 4;
                    return 0;
                }
                else
                if (!strcmp(op2,"IY")){             // LD (nn),IY

                    bufwrite[0] = 0xFD;
                    bufwrite[1] = 0x22;
                    bufwrite[2] = dst16&0xFF;
                    bufwrite[3] = dst16>>8;
                    nbufwrite = 4;
                    return 0;
                }
                return -1;
            }

            return -1;
        }
    }


    return 0;
}

////////////////////////////////////////////////////////////////////////////////
const char *opcodesimple[]={

        "LD A,(BC)",
        "LD A,(DE)",
        "LD A,I",
        "LD A,R",

        "LD (BC),A",
        "LD (DE),A",
        "LD I,A",
        "LD R,A",

        "LD SP,HL",
        "LD SP,IX",
        "LD SP,IY",

        "PUSH IX",
        "PUSH IY",
        "POP IX",
        "POP IY",

        "EX DE,HL",
        "EX AF,AF'",
        "EXX",
        "EX (SP),HL",
        "EX (SP),IX",
        "EX (SP),IY",

        "LDI",
        "LDIR",
        "LDD",
        "LDDR",
        "CPI",
        "CPIR",
        "CPD",
        "CPDR",

        "ADD A,(HL)",
        "INC (HL)",

        "DAA",
        "CPL",
        "NEG",
        "CCF",
        "SCF",
        "NOP",
        "HALT",
        "DI",
        "EI",
        "IM 0",
        "IM 1",
        "IM 2",

        "INC IX",
        "INC IY",
        "DEC IX",
        "DEC IY",

        "RLCA",
        "RLA",
        "RRCA",
        "RRA",
        "RLC (HL)",
        "RLD",
        "RRD",

        "RET",
        "RETI",
        "RETN",

        "INI",
        "INIR",
        "IND",
        "INDR",
        "OUTI",
        "OTIR",
        "OUTD",
        "OTDR",
        NULL
};

const uint8_t opcodesimplecode[]={

        0x00,0x0A,  //ld a,(bc)
        0x00,0x1A,  //ld a,(de)
        0xED,0x57,  //ld a,i
        0xED,0x5F,  //ld a,r

        0x00,0x02,  //ld (bc),a
        0x00,0x12,  //ld (de),a
        0xED,0x47,  //ld i,a
        0xED,0x4F,  //ld r,a

        0x00,0xF9,  //ld sp,hl
        0xDD,0xF9,  //ld sp,ix
        0xFD,0xF9,  //ld sp,iy

        0xDD,0xE5,  //push ix
        0xFD,0xE5,  //push iy
        0xDD,0xE1,  //pop ix
        0xFD,0xE1,  //pop iy

        0x00,0xEB,  //ex de,hl
        0x00,0x08,  //ex af,af'
        0x00,0xD9,  //exx
        0x00,0xE3,  //ex (sp),hl
        0xDD,0xE3,  //ex (sp),ix
        0xFD,0xE3,  //ex (sp),iy

        0xED,0xA0,  //ldi
        0xED,0xB0,  //ldir
        0xED,0xA8,  //ldd
        0xED,0xB8,  //lddr
        0xED,0xA1,  //cpi
        0xED,0xB1,  //cpir
        0xED,0xA9,  //cpd
        0xED,0xB9,  //cpdr

        0x00,0x86,  //add a,(hl)
        0x00,0x34,  //inc (hl)

        0x00,0x27,  //daa
        0x00,0x2F,  //cpl
        0xED,0x44,  //neg
        0x00,0x3F,  //ccf
        0x00,0x37,  //scf
        0x00,0x00,  //nop
        0x00,0x76,  //halt
        0x00,0xF3,  //di
        0x00,0xFB,  //ei
        0xED,0x46,  //im 0
        0xED,0x56,  //im 1
        0xED,0x5E,  //im 2

        0xDD,0x23,  //inc ix
        0xFD,0x23,  //inc iy
        0xDD,0x2B,  //dec ix
        0xFD,0x2B,  //dec iy

        0x00,0x07,  //rlca
        0x00,0x17,  //rla
        0x00,0x0F,  //rrca
        0x00,0x1F,  //rra
        0xCB,0x06,  //rlc (hl)
        0xED,0x6F,  //rld
        0xED,0x67,  //rrd

        0x00,0xC9,  //ret
        0xED,0x4D,  //reti
        0xED,0x45,  //retn

        0xED,0xA2,  //ini
        0xED,0xB2,  //inir
        0xED,0xAA,  //ind
        0xED,0xBA,  //indr
        0xED,0xA3,  //outi
        0xED,0xB3,  //otir
        0xED,0xAB,  //outd
        0xED,0xBB,  //otdr
};

////////////////////////////////////////////////////////////////////////////////
int procline (uint8_t *rom, const char *l){

    char buf[80];
    strncpy(buf,l,sizeof(buf));

    printf("========\nLine:%s\n",buf);

    int i = 0;
    while (buf[i]){

        buf[i] = toupper(buf[i]);
        i++;
    }

    int index = get_index(buf,opcodesimple);
    if (index >= 0){

        index *=2;
        uint8_t prefix = opcodesimplecode[index];
        uint8_t code = opcodesimplecode[index+1];
        nbufwrite = 0;
        if (prefix)
            bufwrite[nbufwrite++] = prefix;
        bufwrite[nbufwrite++] = code;
        return 0;
    }

    char *cmd = strtok(buf," ");

#if DEBUG
    printf("cmd:%s\n",cmd);
#endif

    char *op1 = strtok(NULL,",");
    char *op2 = strtok(NULL,",");

#if DEBUG
    printf("op1:%s op2:%s\n",op1,op2);
#endif

    if (!strcmp(cmd,"ORG")){

        if (op2) return -1;
        return parse_immediate16(op1,&pc);
    }
    else
    if (!strcmp(cmd,"LD")){

        return proc_ld(op1,op2);
    }
    else
    if (!strcmp(cmd,"PUSH")){

        if (!op1) return -1;
        if (op2) return -1;
        int index = get_index(op1,reg16a);
        if (index >= 0){
            bufwrite[0] = 0b11000101 | (index << 4);
            nbufwrite = 1;
            return 0;
        }
        else
            return -1;
    }
    else
    if (!strcmp(cmd,"POP")){

        if (!op1) return -1;
        if (op2) return -1;
        int index = get_index(op1,reg16a);
        if (index >= 0){
            bufwrite[0] = 0b11000001 | (index << 4);
            nbufwrite = 1;
            return 0;
        }
        else
            return -1;
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
int romprog(uint8_t *rom, uint16_t size){

    pc = 0;
    int i;

    // step 1
    int line = 0;
    while (src[line] != NULL){
        if (procline(rom,src[line]) < 0){

            printf("Error!\n");
            //return -1;
        }

        if (nbufwrite)
            printf("%04X ",pc);
        for (i = 0; i < nbufwrite; i++){

            rom[pc++] = bufwrite[i];
            printf("%02X ",bufwrite[i]);
        }
        if (nbufwrite)
            printf("\n");
        line++;
    }

    return -1;
}
