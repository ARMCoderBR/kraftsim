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

#include "main.h"
#include "lcd.h"
#include "leds.h"
#include "vga.h"

////////////////////////////////////////////////////////////////////////////////
void editprompt(char *buf, int size){

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
            if (pbuf < size)
                buf[pbuf++] = cbuf[0];
        if (cbuf[0] == 127){
            addch(8);addch(' ');addch(8);
        }
        else
            addch(cbuf[0]);
        refresh();
    }

    buf[pbuf] = 0;
}

////////////////////////////////////////////////////////////////////////////////
void *z80runner(main_data_t *maindata){

    char buf[255];

    int num_steps = 0;

#define NBP 4
    uint16_t bp[1+NBP] = {0};

    for (;!maindata->z.halted;){

        if (maindata->repaint_window){

            leds_refresh(maindata->leds,1);
            lcd_refresh(maindata->lcd, 1);
            vga_refresh(maindata->vga, 1);
            maindata->repaint_window = 0;
        }
        else{

            leds_refresh(maindata->leds,0);
            lcd_refresh(maindata->lcd, 0);
            vga_refresh(maindata->vga, 0);
        }
        //keyb_run(maindata);

        sched_yield();

        if (!maindata->z.running){
            z80_dump_regs(&maindata->z);
            //sprintf(buf,"Step:%d\n",num_steps);
            //addstr(buf);
        }
        else{
            for (int i = 0; i < 1+NBP; i++){

                if ((maindata->z.pc == bp[i])&&bp[i]){

                    if (i == NBP)
                        bp[NBP] = 0;

                    maindata->z.running = 0;
                    z80_print(&maindata->z);
                    //z80_dump_regs(&maindata->z);
                    //sprintf(buf,"Step:%d\n",num_steps);
                    addstr(buf);
                    break;
                }
            }
        }

        z80_step(&maindata->z);
        num_steps++;

        //printf("NextPC:%04x AfterPC:%04x\n",z.pc,z.afterPC);

        if (maindata->z.running)
            continue;
prompt:
        editprompt(buf,32);

        if (!strncmp(buf,"rst",3)){
             z80_reset(&maindata->z);
             z80_print(&maindata->z);
             num_steps = 0;
             continue;
        }

        switch (buf[0]){

            case 'q':
                sprintf(buf,"\n==== NUM STEPS:%d ====\n",num_steps);
                addstr(buf);
                //endwin();
                //exit(0);
                return NULL;

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
                    z80_dump_mem(&maindata->z, RAMBASE,512);
                else{
                    int val;
                    sscanf(buf+1,"%04x",&val);
                    z80_dump_mem(&maindata->z, val,512);
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
                            char buf2[32];
                            editprompt(buf2,10);
                            int val;
                            sscanf(buf2,"%04x",&val);
                            bp[buf[2]-'0'] = val;
                            goto listbp;
                        }
                        break;
                }
                goto prompt;

            case 'g':
                z80_noprint(&maindata->z);
                maindata->z.running = 1;
                continue;

            case 's':
                if (maindata->z.afterPC){
                    bp[NBP] = maindata->z.afterPC;
                    z80_noprint(&maindata->z);
                    maindata->z.running = 1;
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

    main_data_t maindata;

    maindata.width = 1280;
    maindata.height = 960 + 60;

    maindata.sdl = sdl_init (maindata.width, maindata.height);
    if (!maindata.sdl){
        printf("Erro iniciando SDL!\n");
        return -1;
    }

#define LCD_X_OFFSET 5
#define LEDS_X_OFFSET 380

    ios_init(&maindata);
    maindata.lcd = lcd_init(LCD_X_OFFSET, maindata.height-56, maindata.sdl->renderer);
    maindata.leds = leds_init(LEDS_X_OFFSET, maindata.height-25, maindata.sdl->renderer);
    maindata.vga = vga_init(maindata.sdl->renderer);

    ////////////////////////////////////////////////////////////////////////////
    maindata.rom = malloc(ROMSZ);
    maindata.ram = malloc(RAMSZ);

    memset(maindata.rom,0xff,ROMSZ);
    memset(maindata.ram,0x00,RAMSZ);

    if (romprog(maindata.rom,ROMSZ,maindata.ram,RAMBASE,RAMSZ) < 0){

        addstr("Error loading ROM!\n");
        return -1;
    }

    initscr();

    idlok(stdscr,TRUE);
    scrollok(stdscr,TRUE);
    noecho();

    //addstr("\n=== RUN ===\n\n"); refresh();

    z80_initialize(&maindata.z, maindata.rom, ROMSZ, maindata.ram, RAMBASE, RAMSZ, new_out_callback, new_in_callback, new_hw_run, new_irq_sample);

    z80_reset(&maindata.z);

#if RUN
    z80_noprint(&maindata.z);
    maindata.z.running = 1;
#else
    z80_print(&maindata.z);
    maindata->z.running = 0;
#endif

    //addstr("\n=== LOOP ===\n\n");

    z80runner(&maindata);

    //z80_dump_mem(&maindata.z, RAMBASE,512);

    //addstr("\n=== FINISHING ===\n"); refresh();

    free(maindata.rom);
    //addstr("\n=== FINISHING (1) ===\n"); refresh();
    free(maindata.ram);

    leds_close(maindata.leds);
    //addstr("\n=== FINISHING (2) ===\n"); refresh();
    lcd_close(maindata.lcd);
    //addstr("\n=== FINISHING (3) ===\n"); refresh();
    vga_close(maindata.vga);
    //addstr("\n=== FINISHING (4) ===\n"); refresh();
    sdl_close(maindata.sdl);
    addstr("\n=== FINISHED OK ===\n\n"); refresh();

    getch();
    endwin();

    return 0;
}
