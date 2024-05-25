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
#include <ctype.h>

#define ROMSZ 8192
#define RAMSZ 8192
#define RAMBASE 8192

#include "z80.h"
#include "romprog.h"


////////////////////////////////////////////////////////////////////////////////
OUT_CALLBACK_FND(new_out_callback){

    if (port == 0){

        printf("%c",value);
        fflush(stdout);
    }
}

////////////////////////////////////////////////////////////////////////////////
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

    z80_initialize(&z,rom,ROMSZ,ram,RAMBASE,RAMSZ, new_out_callback);

    z80_reset(&z);

    z80_print(&z);

    printf("\n=== LOOP ===\n\n");

    char buf[80];

    #define NBP 4
    uint16_t bp[1+NBP] = {0};
    int running = 0;

    int num_steps = 0;

    for (;!z.halted;){

        if (!running){
            z80_dump_regs(&z);
            printf("Step:%d\n",num_steps);
        }
#if 1
        else{
            for (int i = 0; i < 1+NBP; i++){

                if (z.pc == bp[i]){

                    if (i == NBP)
                        bp[NBP] = 0;

                    running = 0;
                    z80_print(&z);
                    z80_dump_regs(&z);
                    printf("Step:%d\n",num_steps);
                    break;
                }
            }
        }
#endif

        z80_step(&z);
        num_steps++;

        //printf("NextPC:%04x AfterPC:%04x\n",z.pc,z.afterPC);

        if (running)
            continue;
prompt:
        buf[0] = buf[1] = 0;
        printf("\n:");
        fgets(buf,sizeof(buf),stdin);
        switch (buf[0]){

            case 'q':
                printf("\n==== NUM STEPS:%d ====\n",num_steps);
                exit(0);

            case 'h':
                printf("  # HELP #\n");
                printf("  ENTER     ... Step Into\n");
                printf("  s         ... Step Over RAM\n");
                printf("  d         ... Dump RAM\n");
                printf("  bl        ... List BKPTs\n");
                printf("  bcN (0-3) ... Clear BKPT N\n");
                printf("  bsN (0-3) ... Set BKPT N\n");
                printf("  r         ... RUN until BKPT or HALT\n");
                printf("  rst       ... Reset CPU\n");
                printf("  q         ... Quit\n");
                goto prompt;

            case 'd':
                if (!isxdigit(buf[1]))
                    z80_dump_mem(&z, RAMBASE,512);
                else{
                    int val;
                    sscanf(buf+1,"%04x",&val);
                    z80_dump_mem(&z, val,512);
                }
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
                        if ((buf[2] >= '0') && (buf[2] <= '3')){
                            bp[buf[2]-'0'] = 0;
                            goto listbp;
                        }
                        break;
                    case 's':
                        if ((buf[2] >= '0') && (buf[2] <= '3')){
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
                if (!isalpha(buf[1])){
                    z80_noprint(&z);
                    running = 1;
                    continue;
                }
                else{
                    if ((buf[1] == 's') && (buf[2] == 't')){
                        z80_reset(&z);
                        z80_print(&z);
                        num_steps = 0;
                        continue;
                    }
                }
                break;

            case 's':
                if (z.afterPC){
                    bp[NBP] = z.afterPC;
                    z80_noprint(&z);
                    running = 1;
                    continue;
                }
                break;
        }
    }

    z80_dump_mem(&z, RAMBASE,512);

    printf("\n==== NUM STEPS:%d ====\n",num_steps);

    return 0;
}
