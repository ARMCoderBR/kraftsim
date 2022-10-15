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
        "halt",
        NULL
        };

////////////////////////////////////////////////////////////////////////////////
int parse_immediate8(const char *s, uint8_t *dst){

    uint16_t res = 0;
    if (!strncmp(s,"0x",2)){    //Hex

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
    if (!strncmp(s,"0x",2)){    //Hex

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
const char *reg8[] = {"b","c","d","e","h","l","","a",NULL};
const char *reg16[] = {"bc","de","hl","sp",NULL};
const char *reg16a[] = {"bc","de","hl","af",NULL};

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

    if (index1 >= 0){

        if (index2 >= 0){                       // LD r,r'

            bufwrite[0] = 0b01000000 | (index1 << 3) | index2;
            nbufwrite = 1;
            return 0;
        }
        else{   // index2 == 0

            if (parse_immediate8(op2, &imm) >= 0){  // LD r,n

                bufwrite[0] = 0b00000110 | (index1 << 3);
                bufwrite[1] = imm;
                nbufwrite = 2;
            }
            else
            if (!strcmp(op2,"(hl)")){               // LD r,(HL)
                bufwrite[0] = 0b01000110 | (index1 << 3);
                nbufwrite = 1;
                return 0;
            }
            else
            if (!strncmp(op2,"(ix",3)){             // LD r,(IX+d)

                if (parse_ixy_d(op2+3,&dst)<0)
                    return -1;
                bufwrite[0] = 0xDD;
                bufwrite[1] = 0b01000110 | (index1 << 3);
                bufwrite[2] = dst;
                nbufwrite = 3;
                return 0;
            }
            else
            if (!strncmp(op2,"(iy",3)){             // LD r,(IY+d)

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

                if (!strcmp(op2,"(bc)")){               // LD A,(BC)
                    bufwrite[0] = 0x0a;
                    nbufwrite = 1;
                    return 0;
                }
                else
                if (!strcmp(op2,"(de)")){               // LD A,(DE)
                    bufwrite[0] = 0x1a;
                    nbufwrite = 1;
                    return 0;
                }
                else
                if (parse_immediate_16_paren(op2,&dst16)>=0){

                }

            }


        }

        return -1;
    }

    if (index2 >= 0){

        if (!strcmp(op1,"(hl)")){               // LD (HL),r
            bufwrite[0] = 0b01110000 | index2;
            nbufwrite = 1;
            return 0;
        }
        else
        if (!strncmp(op1,"(ix",3)){             // LD (IX+d),r
            if (parse_ixy_d(op1+3,&dst)<0)
                return -1;
            bufwrite[0] = 0xDD;
            bufwrite[1] = 0b01110000 | index2;
            bufwrite[2] = dst;
            nbufwrite = 3;
            return 0;
        }
        else
        if (!strncmp(op1,"(iy",3)){             // LD (IY+d),r
            if (parse_ixy_d(op1+3,&dst)<0)
                return -1;
            bufwrite[0] = 0xFD;
            bufwrite[1] = 0b01110000 | index2;
            bufwrite[2] = dst;
            nbufwrite = 3;
            return 0;
        }

        return -1;
    }

    if (parse_immediate8(op2, &imm) >= 0){

        if (!strcmp(op1,"(hl)")){               // LD (HL),n
            bufwrite[0] = 0b00110110;
            bufwrite[1] = imm;
            nbufwrite = 2;
            return 0;
        }
        else
        if (!strncmp(op1,"(ix",3)){             // LD (IX+d),n
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
        if (!strncmp(op1,"(iy",3)){             // LD (IY+d),n
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








    return 0;
}

////////////////////////////////////////////////////////////////////////////////
int procline (uint8_t *rom, const char *l){

    char buf[80];
    strncpy(buf,l,sizeof(buf));

    printf("========\nLine:%s\n",buf);

    char *cmd = strtok(buf," ");

    printf("cmd:%s\n",cmd);

    char *op1 = strtok(NULL,",");
    char *op2 = strtok(NULL,",");

    printf("op1:%s op2:%s\n",op1,op2);

    nbufwrite = 0;

    if (!strcmp(cmd,"org")){

        return parse_immediate16(op1,&pc);
    }

    if (!strcmp(cmd,"ld")){

        return proc_ld(op1,op2);
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
            return -1;
        }

        if (nbufwrite)
            printf("%04x ",pc);
        for (i = 0; i < nbufwrite; i++){

            rom[pc++] = bufwrite[i];
            printf("%02x ",bufwrite[i]);
        }
        if (nbufwrite)
            printf("\n");
        line++;
    }

    return -1;
}
