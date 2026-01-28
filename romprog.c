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
int proclineintelhex (uint8_t *mem, uint16_t membase, uint16_t memsize, char *buf){

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

    //printf("size:%02x addr:%04x type:%02x - %s\n",size,addr,type,buf);

    if (type == 0){ // data

        if (addr + size >= memsize){
            printf("Memory Overflow!\n");
            return -1;
        }
        else
        {

            for (int i = 0, j = 9; i < size; i++, j+=2){

                int b = parsehex8(buf+j);
                if (b < 0){
                    printf("Invalid hex line!\n");
                    return -1;
                }
                mem[addr-membase+i] = b;
            }
        }
    }

    return 1;
}

////////////////////////////////////////////////////////////////////////////////
int memprog_readintelhex(uint8_t *mem, char *fname, uint16_t membase, uint16_t memsize){

    char buf[128];

    FILE *f = fopen (fname,"r");
    if (!f){

        printf("File not found\n");
        return -1;
    }

    while (!feof(f)){

        if (fgets(buf, sizeof(buf), f)){

            int len = strlen(buf);
            if (buf[len-1] == '\n')
                buf[len-1] = 0;

            if (proclineintelhex(mem, membase, memsize, buf) < 0) break;
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

            //printf("Programming %04x Data:%s\n",pc, buf);
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

    if (system ("sdasz80 -o -l -s -g ../picalc.s")) exit(0);    // Roda no display LCD, falta simular no Hardware!
    if (system ("sdcc -mz80 --no-std-crt0 ../picalc.rel")) exit(0);
    return memprog_readintelhex(rom, "picalc.ihx", 0, size);
}

////////////////////////////////////////////////////////////////////////////////
int romprog_kraftsim(uint8_t *rom, uint16_t romsize, uint8_t *ram, uint16_t rambase, uint16_t ramsize, char *fname){

    memset(rom,0xff,romsize);

    int res = memprog_readintelhex(rom, "../kraftbios-v2.ihx", 0, romsize);
    if (res < 0) return res;
    res = memprog_readintelhex(rom, "../bas32k.ihx", 0, romsize);
    if (res < 0) return res;

    if (fname)
        if (fname[0])
            return memprog_readintelhex(ram, fname, rambase, ramsize);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
int romprog(uint8_t *rom, uint16_t romsize, uint8_t *ram, uint16_t rambase, uint16_t ramsize){

    //return romprog_picalc_old(rom, size);
    //return romprog_picalc_kraft(rom, romsize);
//#define FNAME "../wolfram.ihx"
//#define FNAME "../chiptunes2.ihx"
//#define FNAME "../chiptunes.ihx"
//#define FNAME "../clock.ihx"
//#define FNAME "../clock2.ihx"
//#define FNAME "../mandel.ihx"
#define FNAME "../invaders.ihx"
//#define FNAME "../kitt.ihx"
    return romprog_kraftsim(rom, romsize, ram, rambase, ramsize, FNAME);
}
