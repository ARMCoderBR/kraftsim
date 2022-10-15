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

uint16_t pc;
uint8_t bufwrite[5];
uint8_t nbufwrite = 0;

////////////////////////////////////////////////////////////////////////////////
const char *src[]={
        "org 0x0000",
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
        "ld hl,(16396)",
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

                if (!strcmp(op2,"(BC)")){               // LD A,(BC)
                    bufwrite[0] = 0x0a;
                    nbufwrite = 1;
                    return 0;
                }
                else
                if (!strcmp(op2,"(DE)")){               // LD A,(DE)
                    bufwrite[0] = 0x1a;
                    nbufwrite = 1;
                    return 0;
                }
                else
                if (parse_immediate16_paren(op2,&dst16)>=0){   // LD A,(nn)
                    bufwrite[0] = 0x3a;
                    bufwrite[1] = dst16&0xFF;
                    bufwrite[2] = dst16>>8;
                    nbufwrite = 3;
                    return 0;
                }
                else
                if (!strcmp(op2,"I")){               // LD A,I
                    bufwrite[0] = 0xED;
                    bufwrite[1] = 0x57;
                    nbufwrite = 2;
                    return 0;
                }
                else
                if (!strcmp(op2,"R")){               // LD A,R
                    bufwrite[0] = 0xED;
                    bufwrite[1] = 0x5F;
                    nbufwrite = 2;
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

                if (!strcmp(op1,"(BC)")){               // LD (BC),A
                    bufwrite[0] = 0x02;
                    nbufwrite = 1;
                    return 0;
                }
                else
                if (!strcmp(op1,"(DE)")){               // LD (DE),A
                    bufwrite[0] = 0x12;
                    nbufwrite = 1;
                    return 0;
                }
                else
                if (parse_immediate16_paren(op1,&dst16)>=0){   // LD (nn),A
                    bufwrite[0] = 0x32;
                    bufwrite[1] = dst16&0xFF;
                    bufwrite[2] = dst16>>8;
                    nbufwrite = 3;
                    return 0;
                }
                else
                if (!strcmp(op1,"I")){               // LD I,A
                    bufwrite[0] = 0xED;
                    bufwrite[1] = 0x47;
                    nbufwrite = 2;
                    return 0;
                }
                else
                if (!strcmp(op1,"R")){               // LD R,A
                    bufwrite[0] = 0xED;
                    bufwrite[1] = 0x4F;
                    nbufwrite = 2;
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

            if (!strcmp(op1,"SP")){

                if (!strcmp(op2,"HL")){

                    bufwrite[0] = 0xF9;
                    nbufwrite = 1;
                    return 0;
                }
                else
                if (!strcmp(op2,"IX")){

                    bufwrite[0] = 0xDD;
                    bufwrite[1] = 0xF9;
                    nbufwrite = 2;
                    return 0;
                }
                else
                if (!strcmp(op2,"IY")){

                    bufwrite[0] = 0xFD;
                    bufwrite[1] = 0xF9;
                    nbufwrite = 2;
                    return 0;
                }

                return -1;
            }

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
int procline (uint8_t *rom, const char *l){

    char buf[80];
    strncpy(buf,l,sizeof(buf));

    printf("========\nLine:%s\n",buf);

    int i = 0;
    while (buf[i]){

        buf[i] = toupper(buf[i]);
        i++;
    }

    char *cmd = strtok(buf," ");

    printf("cmd:%s\n",cmd);

    char *op1 = strtok(NULL,",");
    char *op2 = strtok(NULL,",");

    printf("op1:%s op2:%s\n",op1,op2);

    nbufwrite = 0;

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
        if (!strcmp(op1,"IX")){
            bufwrite[0] = 0xDD;
            bufwrite[1] = 0xE5;
            nbufwrite = 2;
            return 0;
        }
        else
        if (!strcmp(op1,"IY")){
            bufwrite[0] = 0xFD;
            bufwrite[1] = 0xE5;
            nbufwrite = 2;
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
        if (!strcmp(op1,"IX")){
            bufwrite[0] = 0xDD;
            bufwrite[1] = 0xE1;
            nbufwrite = 2;
            return 0;
        }
        else
        if (!strcmp(op1,"IY")){
            bufwrite[0] = 0xFD;
            bufwrite[1] = 0xE1;
            nbufwrite = 2;
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
