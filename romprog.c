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

int dechex8(char *buf){

    char h = toupper(buf[0]);
    char l = toupper(buf[1]);

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
int proclinehex (uint8_t *rom, uint16_t romsize, char *buf, int pc){

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

        int b = dechex8(buf+i);
        if (b < 0) {

            printf("Error parsing Hex\n");
            return -1;
        }

        if ((pc+j) < romsize)
            rom[pc+j] = b & 0xff;
    }

    return j;
}

////////////////////////////////////////////////////////////////////////////////
int romprog_picalc(uint8_t *rom, uint16_t size){

    char buf[128];

    //system("z80asm ../test.asm -o - | xxd -ps -c 16 > test.hex");
    if (system("z80asm ../picalc.asm -lpicalc.lst -o - | xxd -ps -c 16 > test.hex")) exit (0);
    // Para DEBUG if (system("z80asm picalc.asm -o - | xxd -ps -c 16 > test.hex")) exit (0);


    FILE *f = fopen ("test.hex","r");
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
            len = proclinehex(rom, size, buf, pc);

            if (len <= 0) break;
            pc += len;
        }
    }

    fclose(f);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
int romprog_basesim(uint8_t *rom, uint16_t size){

    char buf[128];


    FILE *f = fopen ("crt0.ihx","r");
    if (!f){

        printf("File not found\n");
        return -1;
    }

    while (!feof(f)){

        if (fgets(buf, sizeof(buf), f)){

            int len = strlen(buf);
            if (buf[len-1] == '\n')
                buf[len-1] = 0;

            if (buf[0] != ':') return -1;
            char *p = buf+1;

            len = dechex8(p); p += 2;
            if (len < 0) return -1;
            if (!len) break;
            int addr = dechex8(p); p += 2;
            int addr2 = dechex8(p); p += 2;
            if ((addr < 0) || (addr2 < 0)) return -1;
            int type = dechex8(p); p += 2;
            if (type) return -1;
            addr <<= 8;
            addr |= addr2;
            printf("Programming %04x Data:%s\n",addr, buf);
            for (int i = 0; i < len; i++){
                int b = dechex8(p); p += 2;
                if (b < 0) return -1;
                rom[i+addr] = b;
            }
        }
    }

    fclose(f);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
int romprog(uint8_t *rom, uint16_t size){

    return romprog_picalc(rom, size);//, fname);
    //return romprog_basesim(rom, size);
}
