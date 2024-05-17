/*
 * main.c
 *
 *  Created on: 13 de out de 2022
 *      Author: milton
 */

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>

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

    z80_print(&z);

    printf("\n=== LOOP ===\n\n");

    char buf[80];

    #define NBP 10
    uint16_t bp[NBP] = {0};
    int running = 0;

    for (;!z.halted;){

        if (!running)
            z80_dump_regs(&z);
        else{

            for (int i = 0; i < NBP; i++){

                if (z.pc == bp[i]){

                    running = 0;
                    z80_print(&z);
                    z80_dump_regs(&z);
                    break;
                }
            }
        }

        z80_step(&z);

        if (running)
            continue;
prompt:
        buf[0] = '.';
        printf("\n:");
        fgets(buf,sizeof(buf),stdin);
        switch (buf[0]){

            case 'q':
                exit(0);

            case 'h':
                printf("  # HELP #\n");
                printf("  ENTER     ... Step\n");
                printf("  d         ... Dump RAM\n");
                printf("  bl        ... List BKPTs\n");
                printf("  bcN (0-9) ... Clear BKPT N\n");
                printf("  bsN (0-9) ... Set BKPT N\n");
                printf("  r         ... RUN until BKPT or HALT\n");
                printf("  q         ... Quit\n");
                goto prompt;

            case 'd':
                z80_dump_mem(&z, RAMBASE,512);
                goto prompt;

            case 'b':
                switch(buf[1]){

                    case 'l':
listbp:
                        printf("\nBreakpoints\n");
                        for (int i = 0; i < NBP; i++)
                            printf("%d - %04x\n",i,bp[i]);
                        break;
                    case 'c':
                        if ((buf[2] >= '0') && (buf[2] <= '9')){
                            bp[buf[2]-'0'] = 0;
                            goto listbp;
                        }
                        break;
                    case 's':
                        if ((buf[2] >= '0') && (buf[2] <= '9')){
                            printf("Enter BP val hhhh:");
                            char buf2[8];
                            fgets(buf2,sizeof(buf2),stdin);
                            int val;
                            sscanf(buf2,"%04x",&val);
                            bp[buf[2]-'0'] = val;
                            goto listbp;
                        }
                        break;
                }
                goto prompt;

            case 'r':
                z80_noprint(&z);
                running = 1;
                continue;

        }
    }

    z80_dump_mem(&z, RAMBASE,512);

    return 0;
}
