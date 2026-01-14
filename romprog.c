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
#include <stdlib.h>

#include "romprog.h"

#define DEBUG 0

// z80asm test.asm -o - | xxd -ps -c 16 > test.hex

////////////////////////////////////////////////////////////////////////////////
int parsehex8(char *s){

    char h = toupper(s[0]);
    char l = toupper(s[1]);

    if (!isxdigit(h)||(!isxdigit(l))) return -1;

    int b = 0;
    if (isdigit(h))
        b = (h - '0') * 16;
    else
        b = (10 + h - 'A') * 16;

    if (isdigit(l))
        b += (l - '0');
    else
        b += (10 + l - 'A');

    return b;
}

////////////////////////////////////////////////////////////////////////////////
int parsehex16(char *s){

    int bh = parsehex8(s);
    int bl = parsehex8(s+2);

    if ((bh < 0)||(bl < 0)) return -1;

    return (bh << 8) | bl;
}

////////////////////////////////////////////////////////////////////////////////
int proclineintelhex (uint8_t *rom, uint16_t romsize, char *buf){

    int type = parsehex8(buf+7);
    int addr = parsehex16(buf+3);
    int size = parsehex8(buf+1);

    if ((buf[0] != ':')||
        (type < 0) ||
        (addr < 0) ||
        (size < 0)) {
        printf("Invalid hex line!\n");
        return -1;
    }

    printf("size:%2x addr:%04x type:%02x - %s\n",size,addr,type,buf);

    if (type == 0){ // data

        if (addr + size >= romsize){
            printf("ROM Overflow!\n");
            return -1;
        }
        else{

            for (int i = 0, j = 9; i < size; i++, j+=2){

                int b = parsehex8(buf+j);
                if (b < 0){
                    printf("Invalid hex line!\n");
                    return -1;
                }
                rom[addr+i] = b;
            }
        }
    }

    return 1;
}

////////////////////////////////////////////////////////////////////////////////
int romprog_readintelhex(uint8_t *rom, char *fname, uint16_t size){

    char buf[128];

    FILE *f = fopen (fname,"r");
    if (!f){

        printf("File not found\n");
        return -1;
    }

    memset(rom,0xff,size);

    while (!feof(f)){

        if (fgets(buf, sizeof(buf), f)){

            int len = strlen(buf);
            if (buf[len-1] == '\n')
                buf[len-1] = 0;

            if (proclineintelhex(rom, size, buf) < 0) break;
        }
    }

    fclose(f);

    return 0;
}


////////////////////////////////////////////////////////////////////////////////
int proclineplainhex (uint8_t *rom, uint16_t romsize, char *buf, int pc){

    int len = strlen(buf);

    if (!len) return 0;
    if (buf[len-1] == '\n')
        --len;

    if (!len) return 0;

    if (len & 1){
        printf("Invalid line len\n");
        return -1;
    }

    int i,j;
    for (i = 0,j = 0; i < len; i+=2, j++){

        int b = parsehex8(buf+i);
        if (b < 0) {

            printf("Error parsing Hex\n");
            return -1;
        }

        if ((pc+j) < romsize)
            rom[pc+j] = b & 0xff;
        else{
            printf("ROM Overflow!\n");
            return -1;
        }
    }

    return j;
}

////////////////////////////////////////////////////////////////////////////////
int romprog_readplainhex(uint8_t *rom, char *fname, uint16_t size){

    char buf[128];

    FILE *f = fopen (fname,"r");
    if (!f){

        printf("File not found\n");
        return -1;
    }

    int pc = 0;

    while (!feof(f)){

        if (fgets(buf, sizeof(buf), f)){

            int len = strlen(buf);
            if (buf[len-1] == '\n')
                buf[len-1] = 0;

            printf("Programming %04x Data:%s\n",pc, buf);
            len = proclineplainhex(rom, size, buf, pc);

            if (len <= 0) break;
            pc += len;
        }
    }

    fclose(f);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
int romprog_picalc_old(uint8_t *rom, uint16_t size){

    //system("z80asm ../test.asm -o - | xxd -ps -c 16 > test.hex");
    if (system("z80asm ../picalc.asm -lpicalc.lst -o - | xxd -ps -c 16 > test.hex")) exit (0);
    // Para DEBUG if (system("z80asm picalc.asm -o - | xxd -ps -c 16 > test.hex")) exit (0);

    return romprog_readplainhex(rom, "test.hex", size);
}


////////////////////////////////////////////////////////////////////////////////
int romprog_picalc_kraft(uint8_t *rom, uint16_t size){

    if (system ("sdasz80 -o -l -s -g picalc.s")) exit(0);
    if (system ("sdcc -mz80 --no-std-crt0 picalc.rel")) exit(0);
    return romprog_readintelhex(rom, "picalc.ihx", size);
}

////////////////////////////////////////////////////////////////////////////////
int romprog_kraftsim(uint8_t *rom, uint16_t size){

    //return romprog_readhex(rom, "crt0.ihx", size);
    return romprog_readintelhex(rom, "kraftbios-v2.ihx", size);
}

////////////////////////////////////////////////////////////////////////////////
int romprog(uint8_t *rom, uint16_t size){

    return romprog_picalc_old(rom, size);
    //return romprog_picalc_kraft(rom, size);
    //return romprog_kraftsim(rom, size);
}
