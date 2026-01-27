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
#include <curses.h>
#include <unistd.h>
#include <SDL2/SDL.h>

#define RUN 1
#define ROMSZ 16384
#define RAMBASE 16384
#define RAMSZ 48*1024

#include "z80.h"
#include "romprog.h"
#include "ios.h"

#include "act.h"
#include "lcd.h"
#include "leds.h"
#include "keyb.h"
#include "vga.h"

////////////////////////////////////////////////////////////////////////////////
void *z80runner(main_data_t *act){

    char buf[255];

    int num_steps = 0;

#define NBP 4
    uint16_t bp[1+NBP] = {0};

    for (;!act->z.halted;){

        keyb_run(act);

        sched_yield();

        if (!act->z.running){
            z80_dump_regs(&act->z);
            sprintf(buf,"Step:%d\n",num_steps);
            addstr(buf);
        }
        else{
            for (int i = 0; i < 1+NBP; i++){

                if ((act->z.pc == bp[i])&&bp[i]){

                    if (i == NBP)
                        bp[NBP] = 0;

                    act->z.running = 0;
                    z80_print(&act->z);
                    z80_dump_regs(&act->z);
                    sprintf(buf,"Step:%d\n",num_steps);
                    addstr(buf);
                    break;
                }
            }
        }

        z80_step(&act->z);
        num_steps++;

        //printf("NextPC:%04x AfterPC:%04x\n",z.pc,z.afterPC);

        if (act->z.running)
            continue;
prompt:
        buf[0] = buf[1] = 0;
        addstr("\n:");
        refresh();

        //fgets(buf,sizeof(buf),stdin);
        int pbuf = 0;
        char cbuf[32];
        for (;;){

            //read (1,cbuf,sizeof(cbuf));
            cbuf[0] = getch();

            if ((cbuf[0] == 8)||
                (cbuf[0] == 127)){
                if (pbuf) --pbuf;
                else continue;
            }
            else
            if (cbuf[0] == 10){
                addstr("\n");
                break;
            }
            else
                buf[pbuf++] = cbuf[0];
            if (cbuf[0] == 127){
                addch(8);addch(' ');addch(8);
            }
            else
                addch(cbuf[0]);
            refresh();
        }

        buf[pbuf] = 0;

        if (!strncmp(buf,"rst",3)){
             z80_reset(&act->z);
             z80_print(&act->z);
             num_steps = 0;
             continue;
        }

        switch (buf[0]){

            case 'q':
                sprintf(buf,"\n==== NUM STEPS:%d ====\n",num_steps);
                addstr(buf);
                endwin();
                exit(0);

            case 'h':
                addstr("  # HELP #\n");
                addstr("  ENTER     ... Step Into\n");
                addstr("  s         ... Step Over RAM\n");
                addstr("  d         ... Dump RAM\n");
                addstr("  bl        ... List BKPTs\n");
                addstr("  bcN (0-3) ... Clear BKPT N\n");
                addstr("  bsN (0-3) ... Set BKPT N\n");
                addstr("  g         ... GO until BKPT or HALT\n");
                addstr("  rst       ... Reset CPU\n");
                addstr("  q         ... Quit\n");
                goto prompt;

            case 'd':
                if (!isxdigit(buf[1]))
                    z80_dump_mem(&act->z, RAMBASE,512);
                else{
                    int val;
                    sscanf(buf+1,"%04x",&val);
                    z80_dump_mem(&act->z, val,512);
                }
                goto prompt;

            case 'b':
                switch(buf[1]){

                    case 'l':
listbp:
                        addstr("\nBreakpoints\n");
                        for (int i = 0; i < NBP; i++){
                            sprintf(buf,"%d - %04x\n",i,bp[i]);
                            addstr(buf);
                        }
                        break;
                    case 'c':
                        if ((buf[2] >= '0') && (buf[2] <= '3')){
                            bp[buf[2]-'0'] = 0;
                            goto listbp;
                        }
                        break;
                    case 's':
                        if ((buf[2] >= '0') && (buf[2] <= '3')){
                            addstr("Enter BP val hhhh:");
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

            case 'g':
                z80_noprint(&act->z);
                act->z.running = 1;
                continue;

            case 's':
                if (act->z.afterPC){
                    bp[NBP] = act->z.afterPC;
                    z80_noprint(&act->z);
                    act->z.running = 1;
                    continue;
                }
                break;
        }
    }

    sprintf(buf,"\n==== NUM STEPS:%d ====\n",num_steps);
    addstr(buf);

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char *argv[]){

    main_data_t prdata;

    prdata.width = 1280;
    prdata.height = 960 + 60;
    prdata.sdl = sdl_init (prdata.width, prdata.height);
    if (!prdata.sdl){
        printf("Erro iniciando SDL!\n");
        return -1;
    }

#define LEDS_X_OFFSET 380

    keyb_init(&prdata);
    ios_init(&prdata);
    lcd_init(&prdata);
    prdata.leds = leds_init(LEDS_X_OFFSET, prdata.height-25, prdata.sdl->renderer);
    prdata.vga = vga_init(prdata.sdl->renderer);


    ////////////////////////////////////////////////////////////////////////////
    prdata.rom = malloc(ROMSZ);
    prdata.ram = malloc(RAMSZ);

    memset(prdata.rom,0xff,ROMSZ);
    memset(prdata.ram,0x00,RAMSZ);

    if (romprog(prdata.rom,ROMSZ,prdata.ram,RAMBASE,RAMSZ) < 0){

        addstr("Error loading ROM!\n");
        return -1;
    }

    initscr();

    idlok(stdscr,TRUE);
    scrollok(stdscr,TRUE);
    noecho();

    //addstr("\n=== RUN ===\n\n"); refresh();

    z80_initialize(&prdata.z, prdata.rom, ROMSZ, prdata.ram, RAMBASE, RAMSZ, new_out_callback, new_in_callback, new_hw_run, new_irq_sample);

    z80_reset(&prdata.z);

#if RUN
    z80_noprint(&prdata.z);
    prdata.z.running = 1;
#else
    z80_print(&prdata.z);
    prdata->z.running = 0;
#endif

    //addstr("\n=== LOOP ===\n\n");

    z80runner(&prdata);

    z80_dump_mem(&prdata.z, RAMBASE,512);

    getch();
    endwin();



    return 0;
}
