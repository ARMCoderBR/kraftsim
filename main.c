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
#include <getopt.h>

#include "z80.h"
#include "romprog.h"
#include "ios.h"
#include "main.h"
#include "lcd.h"
#include "leds.h"
#include "vga.h"

////////////////////////////////////////////////////////////////////////////////

#define RUN             1
#define DEBUGPROMPT     0

#define ROMSZ_MODE0     16384
#define RAMBASE_MODE0   16384
#define RAMSZ_MODE0     48*1024

#define ROMSZ_MODE1     8192
#define RAMBASE_MODE1   8192
#define RAMSZ_MODE1     56*1024

////////////////////////////////////////////////////////////////////////////////

#define MAIN_WIDTH      1280
#define MAIN_HEIGHT     960
#define LCD_X_OFFSET    5
#define LEDS_X_OFFSET   380

////////////////////////////////////////////////////////////////////////////////
void editprompt(char *buf, int size){

    buf[0] = buf[1] = 0;
    addstr("\n:");
    refresh();

    int pbuf = 0;
    char cbuf[32];
    for (;;){

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

#if DEBUGPROMPT

    char buf[255];

#define NBP 4
    uint16_t bp[1+NBP] = {0};
#endif

    int num_steps = 0;

    for (;/*!maindata->z.halted*/;){

        if (!maindata->sdl->wminimized_main){

            if (maindata->sdl->repaint_window_main){

                vga_refresh(maindata->vga, 1);
                maindata->sdl->repaint_window_main = 0;
            }
            else{

                vga_refresh(maindata->vga, 0);
            }
        }

        if (!maindata->sdl->wminimized_panel){

            if (maindata->sdl->repaint_window_panel){

                sdl_drawpanelback(maindata->sdl);
                leds_refresh(maindata->leds,1);
                lcd_refresh(maindata->lcd, 1);
                buttons_refresh(maindata->buttons,1);
                maindata->sdl->repaint_window_panel = 0;
            }
            else{

                leds_refresh(maindata->leds,0);
                lcd_refresh(maindata->lcd, 0);
                buttons_refresh(maindata->buttons,0);
            }
        }

        sched_yield();

#if DEBUG_PROMPT
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
                    maindata->z.iff1 = maindata->z.iff2 = 0;

                    z80_print(&maindata->z);
                    z80_dump_regs(&maindata->z);
                    //sprintf(buf,"Step:%d\n",num_steps);
                    //addstr(buf);
                    break;
                }
            }
        }
#endif

        z80_step(&maindata->z, maindata->ios);
        num_steps++;

        //printf("NextPC:%04x AfterPC:%04x\n",z.pc,z.afterPC);

        if (maindata->z.running)
            continue;
#if DEBUGPROMPT
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
#else
        break;
#endif //#if DEBUGPROMPT
    }

#if DEBUGPROMPT
    sprintf(buf,"\n==== NUM STEPS:%d ====\n",num_steps);
    addstr(buf);
#endif

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
typedef enum {

    COD_VERSION = 1,
    COD_RAMLOAD_IHX = 2,
    COD_ROM1LOAD_IHX = 3,
    COD_ROM2LOAD_IHX = 4,
    COD_RAMLOAD_BIN = 5,
    COD_MMAP = 6,
    COD_HELP = 7
} cod_option_t;

struct option longopts[] = {
        {
            "mmap",                 //const char *name;
            required_argument,      //int         has_arg;
            0,                      //int        *flag;
            COD_MMAP,               //int         val;
        },
        {
            "loadramihx",           //const char *name;
            required_argument,      //int         has_arg;
            0,                      //int        *flag;
            COD_RAMLOAD_IHX,        //int         val;
        },
        {
            "loadram",              //const char *name;
            required_argument,      //int         has_arg;
            0,                      //int        *flag;
            COD_RAMLOAD_BIN,        //int         val;
        },
        {
            "rom1",                 //const char *name;
            required_argument,      //int         has_arg;
            0,                      //int        *flag;
            COD_ROM1LOAD_IHX,           //int         val;
        },
        {
            "rom2",                 //const char *name;
            required_argument,      //int         has_arg;
            0,                      //int        *flag;
            COD_ROM2LOAD_IHX,           //int         val;
        },
        {
            "version",              //const char *name;
            0,                      //int         has_arg;
            0,                      //int        *flag;
            COD_VERSION,            //int         val;
        },
        {
            "v",                    //const char *name;
            0,                      //int         has_arg;
            0,                      //int        *flag;
            COD_VERSION,            //int         val;
        },
        {
            "h",                    //const char *name;
            0,                      //int         has_arg;
            0,                      //int        *flag;
            COD_HELP,               //int         val;
        },
        {
            "help",                 //const char *name;
            0,                      //int         has_arg;
            0,                      //int        *flag;
            COD_HELP,               //int         val;
        },
        {
            0,                      //const char *name;
            0,                      //int         has_arg;
            0,                      //int        *flag;
            0,                      //int         val;
        }
    };

////////////////////////////////////////////////////////////////////////////////
void print_version(){

    printf("\nKraftSim v1.0.0\n(c)2026-02-09 ARMCoderBR\n\n");
}

////////////////////////////////////////////////////////////////////////////////
void print_help(){

    print_version();
    printf("Use:\n");
    printf("  kraftsim -rom1 <imgfile.ihx> [-rom2 <imgfile.ihx>] [-loadram <imgfile.bin>] [-mmap n]\n");
    printf("    At least one 'rom1' image must be loaded and must start at 0x0000.\n");
    printf("    Images cannot be loaded to 'rom2' when using 'mmap 1'.\n");
    printf("    Images loaded to RAM only make sense if the program in 'rom1' makes any use of it.\n");
    printf("    'mmap' defines the memory map 0 or 1 (default 0). Some ROMs may require 'mmap 1'.\n");
}

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char *argv[]){

    int res;
    int longindex;
    char filename[256] = {0};

    int romSZ = ROMSZ_MODE0;
    int ramBASE = RAMBASE_MODE0;
    int ramSZ = RAMSZ_MODE0;

    int loaded_rom = 0;
    int mmap = 0;

    for (;;) {

        res = getopt_long_only(argc, argv, "", longopts, &longindex);

        if (res == -1) break;

        if (res == 63)
            return -1;

        int val = longopts[longindex].val;

        switch (val) {
        case COD_HELP:
            print_help();
            return 0;

        case COD_VERSION:
            print_version();
            return 0;

        case COD_MMAP:
            if (optarg)
                if (optarg[0] == '1'){
                    romSZ = ROMSZ_MODE1;
                    ramBASE = RAMBASE_MODE1;
                    ramSZ = RAMSZ_MODE1;
                    mmap = 1;
                }
            break;
        }
    }

    main_data_t maindata;

    ////////////////////////////////////////////////////////////////////////////
    maindata.rom = malloc(romSZ);
    maindata.ram = malloc(ramSZ);

    memset(maindata.rom,0xff,romSZ);
    memset(maindata.ram,0x00,ramSZ);

    optind = 1; // Reinicia busca das opções

    for (;;) {

        res = getopt_long_only(argc, argv, "", longopts, &longindex);

        if ((res == -1) || (res == 63))
            break;

        int val = longopts[longindex].val;

        switch (val) {

        case COD_RAMLOAD_IHX:

            if (optarg){
                strncpy(filename,optarg,sizeof(filename));
                res = memload_ihx(maindata.ram, ramBASE, ramSZ, filename, 0x0000);
                if (res < 0){
                    return -1;
                }
            }
            break;

        case COD_RAMLOAD_BIN:

            if (optarg){
                strncpy(filename,optarg,sizeof(filename));
                res = memload_bin(maindata.ram, ramBASE, ramSZ, filename, 0x4200);
                if (res < 0){
                    return -1;
                }
            }
            break;

        case COD_ROM1LOAD_IHX:

            if (optarg){
                loaded_rom = 1;
                strncpy(filename,optarg,sizeof(filename));
                res = memload_ihx(maindata.rom, 0, romSZ, filename, 0x0000);
                if (res < 0){
                    return -1;
                }
            }
            break;

        case COD_ROM2LOAD_IHX:

            if (mmap){
                printf("ROM2 not allowed in mmap 1.\n");
                return 0;
            }

            if (optarg){
                strncpy(filename,optarg,sizeof(filename));
                res = memload_ihx(maindata.rom, 0, romSZ, filename, 0x2000);
                if (res < 0){
                    return -1;
                }
            }
            break;
        }
    }

    if (!loaded_rom){
        printf("You need to load at least one ROM1 image.\n");
        return 0;
    }

    maindata.width = MAIN_WIDTH;
    maindata.height = MAIN_HEIGHT;

    maindata.sdl = sdl_init (maindata.width, maindata.height);
    if (!maindata.sdl){
        printf("Error starting SDL!\n");
        return -1;
    }

    sdl_drawpanelback(maindata.sdl);

    maindata.ios = ios_init(&maindata);
    maindata.lcd = lcd_init(86, 8, maindata.sdl->renderer_panel);
    maindata.leds = leds_init(86+68, 108, maindata.sdl->renderer_panel);
    maindata.buttons = buttons_init(10, 140, maindata.sdl->renderer_panel);
    maindata.vga = vga_init(maindata.sdl->renderer_main);

    initscr();

    ////usleep(1000000);

    idlok(stdscr,TRUE);
    scrollok(stdscr,TRUE);
    noecho();

    z80_initialize(&maindata.z, maindata.rom, romSZ, maindata.ram, ramBASE, ramSZ, new_out_callback, new_in_callback, new_hw_run, new_irq_sample);
    z80_reset(&maindata.z);

#if RUN
    z80_noprint(&maindata.z);
    maindata.z.running = 1;
#else
    z80_print(&maindata.z);
    maindata->z.running = 0;
#endif

    z80runner(&maindata);

    //z80_dump_mem(&maindata.z, RAMBASE,512);

    ios_close(maindata.ios);

    free(maindata.rom);
    free(maindata.ram);
    leds_close(maindata.leds);
    lcd_close(maindata.lcd);
    vga_close(maindata.vga);
    buttons_close(maindata.buttons);

    sdl_close(maindata.sdl);
    endwin();

    return 0;
}
