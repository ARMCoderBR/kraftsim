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
#include <string.h>

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
int proclineintelhex (uint8_t *mem, uint16_t membase, uint16_t memsize, uint16_t load_offset, char *buf){

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

    addr += load_offset;

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
int memprog_readintelhex(uint8_t *mem, char *fname, uint16_t membase, uint16_t memsize, uint16_t load_offset){

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

            if (proclineintelhex(mem, membase, memsize, load_offset, buf) < 0) break;
        }
    }

    fclose(f);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
int memload_ihx(uint8_t *mem, uint16_t membase, uint16_t memsize, char *fname, uint16_t load_offset){

    return memprog_readintelhex(mem, fname, membase, memsize, load_offset);
}

////////////////////////////////////////////////////////////////////////////////
int memload_bin(uint8_t *mem, uint16_t membase, uint16_t memsize, char *fname, uint16_t load_offset){

    uint8_t buf[128];

    FILE *f = fopen (fname,"rb");
    if (!f){

        printf("File not found\n");
        return -1;
    }

    int addr = load_offset;

    while (!feof(f)){

        int nb = fread(buf,1,sizeof(buf),f);
        memcpy(mem+addr-membase, buf, nb);
        addr += nb;
    }

    fclose(f);

    return 0;
}
