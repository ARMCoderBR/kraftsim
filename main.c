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

int main (int argc, char *argv[]){

    uint8_t *rom = malloc(ROMSZ);
    uint8_t *ram = malloc(RAMSZ);
    z80_t z;

    memset(rom,0xff,ROMSZ);
    // TODO: programar a rom aqui

    if (romprog(rom,ROMSZ) < 0){

        printf("Error assembling!\n");
        return -1;
    }

    z80_initialize(&z,rom,ROMSZ,ram,RAMBASE,RAMSZ);

    z80_reset(&z);

    for (;;){

        z80_step(&z);
    }

    return 0;
}
