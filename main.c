/*
 * main.c
 *
 *  Created on: 13 de out de 2022
 *      Author: milton
 */

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#define ROMSZ 8192
#define RAMSZ 8192
#define RAMBASE 8192

#include "z80.h"
#include "romprog.h"

int main (int argc, char *argv[]){

    uint8_t *rom = malloc(ROMSZ);
    uint8_t *ram = malloc(RAMSZ);
    z80_t z;

    memset(rom,0xff,ROMSZ);

    if (romprog(rom,ROMSZ,"test.hex") < 0){

        printf("Error loading ROM!\n");
        return -1;
    }

    printf("\n=== RUN ===\n\n");

    z80_initialize(&z,rom,ROMSZ,ram,RAMBASE,RAMSZ);

    z80_reset(&z);

    z80_dump(&z);

    printf("\n=== LOOP ===\n\n");

    for (;!z.halted;){

        z80_step(&z);
        z80_dump(&z);
        getchar();
    }

    z80_dump_mem(&z, RAMBASE,256);

    return 0;
}
