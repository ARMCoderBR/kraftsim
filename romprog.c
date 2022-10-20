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

// z80asm test.asm -o - | xxd -ps -c 16 > test.hex



////////////////////////////////////////////////////////////////////////////////
int procline (uint8_t *rom, char *buf, int pc){

    int len = strlen(buf);
    if (len & 1)
        return -1;


}

////////////////////////////////////////////////////////////////////////////////
int romprog(uint8_t *rom, uint16_t size, char *fname){

    char buf[128];

    FILE *f = fopen (fname,"r");
    if (!f){

        printf("File not found\n");
        return -1;
    }

    pc = 0;

    while (!feof(f)){

        if (fgets(buf, sizeof(buf), f)){

            int len = procline(rom,buf,pc);

            if (len <= 0) break;
            pc += len;
        }
    }

    fclose(f);

    return 0;
}
