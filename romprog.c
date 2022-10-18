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
        "add a,(hl)",
        "adc a,(ix+4)",
        "sub b",
        "sbc a,(iy-4)",
        "and 32",
        "or c",
        "xor 33",
        "cp d",
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

    if (!s)
        return -1;

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

    nbufwrite = 0;

    if (index1 >= 0){

        if (index2 >= 0){                       // LD r,r'

            bufwrite[nbufwrite++] = 0b01000000 | (index1 << 3) | index2;
            return 0;
        }
        else{   // index2 < 0

            if (parse_immediate8(op2, &imm) >= 0){  // LD r,n

                bufwrite[nbufwrite++] = 0b00000110 | (index1 << 3);
                bufwrite[nbufwrite++] = imm;
                return 0;
            }
            else
            if (!strcmp(op2,"(HL)")){               // LD r,(HL)
                bufwrite[nbufwrite++] = 0b01000110 | (index1 << 3);
                return 0;
            }
            else
            if (!strncmp(op2,"(IX",3)){             // LD r,(IX+d)

                if (parse_ixy_d(op2+3,&dst)<0)
                    return -1;
                bufwrite[nbufwrite++] = 0xDD;
                bufwrite[nbufwrite++] = 0b01000110 | (index1 << 3);
                bufwrite[nbufwrite++] = dst;
                return 0;
            }
            else
            if (!strncmp(op2,"(IY",3)){             // LD r,(IY+d)

                if (parse_ixy_d(op2+3,&dst)<0)
                    return -1;
                bufwrite[nbufwrite++] = 0xFD;
                bufwrite[nbufwrite++] = 0b01000110 | (index1 << 3);
                bufwrite[nbufwrite++] = dst;
                return 0;
            }
            else
            if (index1 == 7){   // Reg a

                if (parse_immediate16_paren(op2,&dst16)>=0){   // LD A,(nn)
                    bufwrite[nbufwrite++] = 0x3a;
                    goto w_dst16;
                }
            }
        }

        return -1;
    }
    else{   //index1 < 0
        if (index2 >= 0){

            if (!strcmp(op1,"(HL)")){               // LD (HL),r
                bufwrite[nbufwrite++] = 0b01110000 | index2;
                return 0;
            }
            else
            if (!strncmp(op1,"(IX",3)){             // LD (IX+d),r
                if (parse_ixy_d(op1+3,&dst)<0)
                    return -1;
                bufwrite[nbufwrite++] = 0xDD;
                bufwrite[nbufwrite++] = 0b01110000 | index2;
                bufwrite[nbufwrite++] = dst;
                return 0;
            }
            else
            if (!strncmp(op1,"(IY",3)){             // LD (IY+d),r
                if (parse_ixy_d(op1+3,&dst)<0)
                    return -1;
                bufwrite[nbufwrite++] = 0xFD;
                bufwrite[nbufwrite++] = 0b01110000 | index2;
                bufwrite[nbufwrite++] = dst;
                return 0;
            }
            else
            if (index2 == 7){

                if (parse_immediate16_paren(op1,&dst16)>=0){   // LD (nn),A
                    bufwrite[nbufwrite++] = 0x32;
                    goto w_dst16;
                }
            }

            return -1;
        }
        else{   //index2 < 0

            if (parse_immediate8(op2, &imm) >= 0){

                if (!strcmp(op1,"(HL)")){               // LD (HL),n
                    bufwrite[nbufwrite++] = 0b00110110;
                    bufwrite[nbufwrite++] = imm;
                    return 0;
                }
                else
                if (!strncmp(op1,"(IX",3)){             // LD (IX+d),n
                    if (parse_ixy_d(op1+3,&dst)<0)
                        return -1;
                    bufwrite[nbufwrite++] = 0xDD;
                    goto ld_ixiy_d_n;
                }
                else
                if (!strncmp(op1,"(IY",3)){             // LD (IY+d),n
                    if (parse_ixy_d(op1+3,&dst)<0)
                        return -1;
                    bufwrite[nbufwrite++] = 0xFD;
ld_ixiy_d_n:
                    bufwrite[nbufwrite++] = 0b00110110;
                    bufwrite[nbufwrite++] = dst;
                    bufwrite[nbufwrite++] = imm;
                    return 0;
                }
            }

            ///// LD 16 bits

            if (parse_immediate16(op2, &imm16) >= 0){

                int index1 = get_index(op1,reg16);

                if (index1 >= 0){                   // LD dd,nn

                    bufwrite[nbufwrite++] = 0b00000001 | (index1<<4);
                    goto w_imm16;
                }
                else
                if (!strcmp(op1,"IX")){             // LD IX,nn

                    bufwrite[nbufwrite++] = 0xDD;
                    bufwrite[nbufwrite++] = 0x21;
                    goto w_imm16;
                }
                else
                if (!strcmp(op1,"IY")){             // LD IY,nn

                    bufwrite[nbufwrite++] = 0xFD;
                    bufwrite[nbufwrite++] = 0x21;
w_imm16:
                    bufwrite[nbufwrite++] = imm16&0xFF;
                    bufwrite[nbufwrite++] = imm16>>8;
                    return 0;
                }

                return -1;
            }
            else
            if (parse_immediate16_paren(op2, &dst16) >= 0){

                int index1 = get_index(op1,reg16);

                if (!strcmp(op1,"HL")){             // LD HL,(nn)
ld_hl_nnad:
                    bufwrite[nbufwrite++] = 0x2A;
                    goto w_dst16;
                }
                else
                if (index1 >= 0){                   // LD dd,(nn)

                    bufwrite[nbufwrite++] = 0xED;
                    bufwrite[nbufwrite++] = 0b01001011 | (index1 << 4);
                    goto w_dst16;
                }
                else
                if (!strcmp(op1,"IX")){             // LD IX,(nn)

                    bufwrite[nbufwrite++] = 0xDD;
                    goto ld_hl_nnad;
                }
                else
                if (!strcmp(op1,"IY")){             // LD IY,(nn)

                    bufwrite[nbufwrite++] = 0xFD;
                    goto ld_hl_nnad;
                }
                return -1;
            }
            else
            if (parse_immediate16_paren(op1, &dst16) >= 0){

                int index2 = get_index(op2,reg16);

                if (!strcmp(op2,"HL")){             // LD (nn),HL
ld_nnad_hl:
                    bufwrite[nbufwrite++] = 0x22;
w_dst16:
                    bufwrite[nbufwrite++] = dst16&0xFF;
                    bufwrite[nbufwrite++] = dst16>>8;
                    return 0;
                }
                else
                if (index2 >= 0){                   // LD (nn),dd

                    bufwrite[nbufwrite++] = 0xED;
                    bufwrite[nbufwrite++] = 0b01000011 | (index2 << 4);
                    goto w_dst16;
                }
                else
                if (!strcmp(op2,"IX")){             // LD (nn),IX

                    bufwrite[nbufwrite++] = 0xDD;
                    goto ld_nnad_hl;
                }
                else
                if (!strcmp(op2,"IY")){             // LD (nn),IY

                    bufwrite[nbufwrite++] = 0xFD;
                    goto ld_nnad_hl;
                }
                return -1;
            }

            return -1;
        }
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
int proc_add (char *op1, char *op2){

    int index1 = get_index(op1,reg8);
    int index2 = get_index(op2,reg8);
    uint8_t dst;
    uint8_t imm;

    nbufwrite = 0;

    if (index1 == 0x07){    // ADD A,...

        if (index2 >= 0){                       // ADD A,r

            bufwrite[nbufwrite++] = 0b10000000 | index2;
            return 0;
        }

        if (parse_immediate8(op2, &imm) >= 0){  // ADD A,n

            bufwrite[nbufwrite++] = 0xC6;
            bufwrite[nbufwrite++] = imm;
            return 0;
        }

        if (!strncmp(op2,"(IX",3)){             // ADD A,(IX+d)
            if (parse_ixy_d(op2+3,&dst)<0)
                return -1;
            bufwrite[nbufwrite++] = 0xDD;
            goto add_a_ixiy_d;
        }
        else
        if (!strncmp(op2,"(IY",3)){             // ADD A,(IY+d)
            if (parse_ixy_d(op2+3,&dst)<0)
                return -1;
            bufwrite[nbufwrite++] = 0xFD;
add_a_ixiy_d:
            bufwrite[nbufwrite++] = 0x86;
            bufwrite[nbufwrite++] = dst;
            return 0;
        }

        return -1;
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
int proc_adc (char *op1, char *op2){

    int index1 = get_index(op1,reg8);
    int index2 = get_index(op2,reg8);
    uint8_t dst;
    uint8_t imm;

    nbufwrite = 0;

    if (index1 == 0x07){    // ADC A,...

        if (index2 >= 0){                       // ADC A,r

            bufwrite[nbufwrite++] = 0b10001000 | index2;
            return 0;
        }

        if (parse_immediate8(op2, &imm) >= 0){  // ADC A,n

            bufwrite[nbufwrite++] = 0xCE;
            bufwrite[nbufwrite++] = imm;
            return 0;
        }

        if (!strncmp(op2,"(IX",3)){             // ADC A,(IX+d)
            if (parse_ixy_d(op2+3,&dst)<0)
                return -1;
            bufwrite[nbufwrite++] = 0xDD;
            goto adc_a_ixiy_d;
        }
        else
        if (!strncmp(op2,"(IY",3)){             // ADC A,(IY+d)
            if (parse_ixy_d(op2+3,&dst)<0)
                return -1;
            bufwrite[nbufwrite++] = 0xFD;
adc_a_ixiy_d:
            bufwrite[nbufwrite++] = 0x86;
            bufwrite[nbufwrite++] = dst;
            return 0;
        }

        return -1;
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
int proc_sub (char *op1, char *op2){

    int index1 = get_index(op1,reg8);
    //int index2 = get_index(op2,reg8);
    uint8_t dst;
    uint8_t imm;

    nbufwrite = 0;

    if (op2 == NULL){    // SUB ...

        if (index1 >= 0){                       // SUB r

            bufwrite[nbufwrite++] = 0b10010000 | index1;
            return 0;
        }

        if (parse_immediate8(op1, &imm) >= 0){  // SUB n

            bufwrite[nbufwrite++] = 0xD6;
            bufwrite[nbufwrite++] = imm;
            return 0;
        }

        if (!strncmp(op1,"(IX",3)){             // SUB (IX+d)
            if (parse_ixy_d(op1+3,&dst)<0)
                return -1;
            bufwrite[nbufwrite++] = 0xDD;
            goto adc_a_ixiy_d;
        }
        else
        if (!strncmp(op1,"(IY",3)){             // SUB (IY+d)
            if (parse_ixy_d(op1+3,&dst)<0)
                return -1;
            bufwrite[nbufwrite++] = 0xFD;
adc_a_ixiy_d:
            bufwrite[nbufwrite++] = 0x96;
            bufwrite[nbufwrite++] = dst;
            return 0;
        }

        return -1;
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
int proc_sbc (char *op1, char *op2){

    int index1 = get_index(op1,reg8);
    int index2 = get_index(op2,reg8);
    uint8_t dst;
    uint8_t imm;

    nbufwrite = 0;

    if (index1 == 7){    // SBC A,...

        if (index2 >= 0){                       // SBC A,r

            bufwrite[nbufwrite++] = 0b10011000 | index1;
            return 0;
        }

        if (parse_immediate8(op2, &imm) >= 0){  // SBC A,n

            bufwrite[nbufwrite++] = 0xDE;
            bufwrite[nbufwrite++] = imm;
            return 0;
        }

        if (!strncmp(op2,"(IX",3)){             // SBC A,(IX+d)
            if (parse_ixy_d(op2+3,&dst)<0)
                return -1;
            bufwrite[nbufwrite++] = 0xDD;
            goto sbc_a_ixiy_d;
        }
        else
        if (!strncmp(op2,"(IY",3)){             // SBC A,(IY+d)
            if (parse_ixy_d(op2+3,&dst)<0)
                return -1;
            bufwrite[nbufwrite++] = 0xFD;
sbc_a_ixiy_d:
            bufwrite[nbufwrite++] = 0x9E;
            bufwrite[nbufwrite++] = dst;
            return 0;
        }

        return -1;
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
int proc_and (char *op1, char *op2){

    int index1 = get_index(op1,reg8);
    //int index2 = get_index(op2,reg8);
    uint8_t dst;
    uint8_t imm;

    nbufwrite = 0;

    if (op2 == NULL){    // AND ...

        if (index1 >= 0){                       // AND r

            bufwrite[nbufwrite++] = 0b10100000 | index1;
            return 0;
        }

        if (parse_immediate8(op1, &imm) >= 0){  // AND n

            bufwrite[nbufwrite++] = 0xE6;
            bufwrite[nbufwrite++] = imm;
            return 0;
        }

        if (!strncmp(op1,"(IX",3)){             // AND (IX+d)
            if (parse_ixy_d(op1+3,&dst)<0)
                return -1;
            bufwrite[nbufwrite++] = 0xDD;
            goto and_a_ixiy_d;
        }
        else
        if (!strncmp(op1,"(IY",3)){             // AND (IY+d)
            if (parse_ixy_d(op1+3,&dst)<0)
                return -1;
            bufwrite[nbufwrite++] = 0xFD;
and_a_ixiy_d:
            bufwrite[nbufwrite++] = 0xA6;
            bufwrite[nbufwrite++] = dst;
            return 0;
        }

        return -1;
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
int proc_or (char *op1, char *op2){

    int index1 = get_index(op1,reg8);
    //int index2 = get_index(op2,reg8);
    uint8_t dst;
    uint8_t imm;

    nbufwrite = 0;

    if (op2 == NULL){    // OR ...

        if (index1 >= 0){                       // OR r

            bufwrite[nbufwrite++] = 0b10110000 | index1;
            return 0;
        }

        if (parse_immediate8(op1, &imm) >= 0){  // OR n

            bufwrite[nbufwrite++] = 0xF6;
            bufwrite[nbufwrite++] = imm;
            return 0;
        }

        if (!strncmp(op1,"(IX",3)){             // OR (IX+d)
            if (parse_ixy_d(op1+3,&dst)<0)
                return -1;
            bufwrite[nbufwrite++] = 0xDD;
            goto or_a_ixiy_d;
        }
        else
        if (!strncmp(op1,"(IY",3)){             // OR (IY+d)
            if (parse_ixy_d(op1+3,&dst)<0)
                return -1;
            bufwrite[nbufwrite++] = 0xFD;
or_a_ixiy_d:
            bufwrite[nbufwrite++] = 0xB6;
            bufwrite[nbufwrite++] = dst;
            return 0;
        }

        return -1;
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
int proc_xor (char *op1, char *op2){

    int index1 = get_index(op1,reg8);
    //int index2 = get_index(op2,reg8);
    uint8_t dst;
    uint8_t imm;

    nbufwrite = 0;

    if (op2 == NULL){    // XOR ...

        if (index1 >= 0){                       // XOR r

            bufwrite[nbufwrite++] = 0b10101000 | index1;
            return 0;
        }

        if (parse_immediate8(op1, &imm) >= 0){  // XOR n

            bufwrite[nbufwrite++] = 0xEE;
            bufwrite[nbufwrite++] = imm;
            return 0;
        }

        if (!strncmp(op1,"(IX",3)){             // XOR (IX+d)
            if (parse_ixy_d(op1+3,&dst)<0)
                return -1;
            bufwrite[nbufwrite++] = 0xDD;
            goto xor_a_ixiy_d;
        }
        else
        if (!strncmp(op1,"(IY",3)){             // XOR (IY+d)
            if (parse_ixy_d(op1+3,&dst)<0)
                return -1;
            bufwrite[nbufwrite++] = 0xFD;
xor_a_ixiy_d:
            bufwrite[nbufwrite++] = 0xAE;
            bufwrite[nbufwrite++] = dst;
            return 0;
        }

        return -1;
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
int proc_cp (char *op1, char *op2){

    int index1 = get_index(op1,reg8);
    //int index2 = get_index(op2,reg8);
    uint8_t dst;
    uint8_t imm;

    nbufwrite = 0;

    if (op2 == NULL){    // XOR ...

        if (index1 >= 0){                       // CP r

            bufwrite[nbufwrite++] = 0b10111000 | index1;
            return 0;
        }

        if (parse_immediate8(op1, &imm) >= 0){  // CP n

            bufwrite[nbufwrite++] = 0xFE;
            bufwrite[nbufwrite++] = imm;
            return 0;
        }

        if (!strncmp(op1,"(IX",3)){             // CP (IX+d)
            if (parse_ixy_d(op1+3,&dst)<0)
                return -1;
            bufwrite[nbufwrite++] = 0xDD;
            goto xor_a_ixiy_d;
        }
        else
        if (!strncmp(op1,"(IY",3)){             // CP (IY+d)
            if (parse_ixy_d(op1+3,&dst)<0)
                return -1;
            bufwrite[nbufwrite++] = 0xFD;
xor_a_ixiy_d:
            bufwrite[nbufwrite++] = 0xBE;
            bufwrite[nbufwrite++] = dst;
            return 0;
        }

        return -1;
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
int proc_inc (char *op1, char *op2){

    int index1 = get_index(op1,reg8);
    //int index2 = get_index(op2,reg8);
    uint8_t dst;

    nbufwrite = 0;

    if (op2 == NULL){    // INC ...

        if (index1 >= 0){                       // INC r

            bufwrite[nbufwrite++] = 0b00000100 | (index1 << 3);
            return 0;
        }

        if (!strncmp(op1,"(IX",3)){             // INC (IX+d)
            if (parse_ixy_d(op1+3,&dst)<0)
                return -1;
            bufwrite[nbufwrite++] = 0xDD;
            goto inc_ixiy_d;
        }
        else
        if (!strncmp(op1,"(IY",3)){             // INC (IY+d)
            if (parse_ixy_d(op1+3,&dst)<0)
                return -1;
            bufwrite[nbufwrite++] = 0xFD;
inc_ixiy_d:
            bufwrite[nbufwrite++] = 0x34;
            bufwrite[nbufwrite++] = dst;
            return 0;
        }

        return -1;
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
int proc_dec (char *op1, char *op2){

    int index1 = get_index(op1,reg8);
    //int index2 = get_index(op2,reg8);
    uint8_t dst;

    nbufwrite = 0;

    if (op2 == NULL){    // DEC ...

        if (index1 >= 0){                       // DEC r

            bufwrite[nbufwrite++] = 0b00000101 | (index1 << 3);
            return 0;
        }

        if (!strncmp(op1,"(IX",3)){             // DEC (IX+d)
            if (parse_ixy_d(op1+3,&dst)<0)
                return -1;
            bufwrite[nbufwrite++] = 0xDD;
            goto inc_ixiy_d;
        }
        else
        if (!strncmp(op1,"(IY",3)){             // DEC (IY+d)
            if (parse_ixy_d(op1+3,&dst)<0)
                return -1;
            bufwrite[nbufwrite++] = 0xFD;
inc_ixiy_d:
            bufwrite[nbufwrite++] = 0x35;
            bufwrite[nbufwrite++] = dst;
            return 0;
        }

        return -1;
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
        "ADC A,(HL)",
        "SUB (HL)",
        "SBC A,(HL)",
        "AND (HL)",
        "OR (HL)",
        "XOR (HL)",
        "CP (HL)",
        "INC (HL)",
        "DEC (HL)",

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

        "JP (HL)"
        "JP (IX)"
        "JP (IY)"

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
        0x00,0x8E,  //adc a,(hl)
        0x00,0x96,  //sub (hl)
        0x00,0x9E,  //sbc a,(hl)
        0x00,0xA6,  //and (hl)
        0x00,0xB6,  //or (hl)
        0x00,0xAE,  //xor (hl)
        0x00,0xBE,  //cp (hl)
        0x00,0x34,  //inc (hl)
        0x00,0x35,  //dec (hl)

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

        0x00,0xE9,  //jp (hl)
        0xDD,0xE9,  //jp (ix)
        0xFD,0xE9,  //jp (iy)

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
    if (!strcmp(cmd,"ADD")){

        return proc_add(op1,op2);
    }
    else
    if (!strcmp(cmd,"ADC")){

        return proc_adc(op1,op2);
    }
    else
    if (!strcmp(cmd,"SUB")){

        return proc_sub(op1,op2);
    }
    else
    if (!strcmp(cmd,"SBC")){

        return proc_sbc(op1,op2);
    }
    else
    if (!strcmp(cmd,"AND")){

        return proc_and(op1,op2);
    }
    else
    if (!strcmp(cmd,"OR")){

        return proc_or(op1,op2);
    }
    else
    if (!strcmp(cmd,"XOR")){

        return proc_xor(op1,op2);
    }
    else
    if (!strcmp(cmd,"CP")){

        return proc_cp(op1,op2);
    }
    else
    if (!strcmp(cmd,"INC")){

        return proc_inc(op1,op2);
    }
    else
    if (!strcmp(cmd,"DEC")){

        return proc_dec(op1,op2);
    }
    else
    if (!strcmp(cmd,"PUSH")){       //push qq

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
    if (!strcmp(cmd,"POP")){        //pop qq

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
