#include <stdint.h>
#include <string.h>
#include <ncurses.h>

#include "lcdrom.h"
#include "lcd.h"

#define DEBUG 0

#define DOT_SPACE   3
#define DOT_FILL    2
#define CHAR_HSPACE (DOT_SPACE*6)
#define CHAR_VSPACE (DOT_SPACE*9)
#define LCDBORDER   16

////////////////////////////////////////////////////////////////////////////////
static void draw_lcdback(SDL_Renderer* renderer, int x, int y) {

    SDL_Rect rect;


    rect.x = x;
    rect.y = y;
    rect.w = (16*CHAR_HSPACE)+2*LCDBORDER;
    rect.h = (2*CHAR_VSPACE)+2*LCDBORDER;

    SDL_SetRenderDrawColor(renderer, 64, 64, 64, 255);
    SDL_RenderFillRect(renderer, &rect);

    rect.x = x+LCDBORDER;
    rect.y = y+LCDBORDER;
    rect.w = (16*CHAR_HSPACE);
    rect.h = (2*CHAR_VSPACE);

    SDL_SetRenderDrawColor(renderer, 0, 204, 77, 255);
    SDL_RenderFillRect(renderer, &rect);

    SDL_RenderPresent(renderer);
}

////////////////////////////////////////////////////////////////////////////////
static void draw_lcdpoint(SDL_Renderer* renderer, int x, int y, int onoff) {

    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = DOT_SPACE;
    rect.h = DOT_SPACE;
    SDL_SetRenderDrawColor(renderer, 0, 179, 77, 255);
    SDL_RenderFillRect(renderer, &rect);

    rect.w = DOT_FILL;
    rect.h = DOT_FILL;

    if (onoff)
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    else
        SDL_SetRenderDrawColor(renderer, 0, 204, 77, 255);

    SDL_RenderFillRect(renderer, &rect);
}

////////////////////////////////////////////////////////////////////////////////
static void lcd_out_symbol(SDL_Renderer* renderer, int px, int py, uint8_t code){

    if (code > 0x7f) code = 0x20;

    int rom_ofs = 8*(code-0x10);

    for (int row = 0; row < 8; row++){

        int colmask = 0x10;

        for (int col = 0; col < 5; col++){

            int state = 0;
            if (lcdrom[rom_ofs] & colmask)
                state = 1;
            draw_lcdpoint(renderer, px+DOT_SPACE*col, py+DOT_SPACE*row, state);

            colmask >>= 1;
        }

        rom_ofs++;
    }
}

////////////////////////////////////////////////////////////////////////////////
static Uint32 lcd_set_tick(Uint32 interval, void *param){

    lcd_t *lcd = param;

    lcd->lcdTick = 1;
    return interval;
}

////////////////////////////////////////////////////////////////////////////////
static int proc_cmd8(lcd_t *lcd, uint8_t value){

#if DEBUG
    char buf[100];
    sprintf(buf," [PROC:%02x] ",value);
    addstr(buf);
#endif

    if (value & 0x80){  // Sets DDRAM Addr
        lcd->ddram_addr = value & 0x7f;
        lcd->last_addr_is_cg = 0;
#if DEBUG
        addstr(" =setDDRAM=\n");
#endif
        }
    else
    if (value & 0x40){  // Sets CGRAM Addr
        lcd->cgram_addr = value & 0x3f;
        lcd->last_addr_is_cg = 1;
#if DEBUG
        addstr(" =setCGRAM=\n");
#endif
        }
    else
    if (value & 0x20){  // Sets DL, N and F
        lcd->function_dl_n_f = value & 0x1f;
        lcd->value8_state = 0;
#if DEBUG
        addstr(" =setDLNF=\n");
#endif
        return 1;
    }
    else
    if (value & 0x10){  // Moves cursor & Shifts
        //...
#if DEBUG
        addstr(" =movCURS=\n");
#endif
        }
    else
    if (value & 0x08){  // Sets D, C and B
#if DEBUG
        addstr(" =setDCB=\n");
#endif
        lcd->disp_control_d_c_b = value & 0x07;
    }
    else
    if (value & 0x04){  // Sets Cursor move dir and disp shift during write
        lcd->modeset_id_s = value & 0x03;
#if DEBUG
        addstr(" =setIDS=\n");
#endif
        }
    else
    if (value & 0x02){  // Return Home

        lcd->ddram_addr = 0;
        lcd->last_addr_is_cg = 0;
#if DEBUG
        addstr(" =ReturnHOME=\n");
#endif
        }
    else
    if (value & 0x01){  // Clear Display
        memset(lcd->ddram,0x20,sizeof(lcd->ddram));
        lcd->ddram_addr = 0;
        lcd->last_addr_is_cg = 0;
#if DEBUG
        addstr(" =ClearDISP=\n");
#endif
        }
#if DEBUG
    else
        addstr(" = ????? =\n");
#endif

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
static void proc_data8(lcd_t *lcd, uint8_t value){

#if DEBUG
    char buf[100];
    sprintf(buf," [DATA:%02x] ",value);
    addstr(buf);
#endif

    if (lcd->last_addr_is_cg){
        lcd->cgram[lcd->cgram_addr++] = value;
        if (lcd->cgram_addr == sizeof(lcd->cgram))
            lcd->cgram_addr = 0;
#if DEBUG
        addstr(" =writeCGRAM=\n");
#endif
        }
    else{
        lcd->ddram[lcd->ddram_addr++] = value;
        if (lcd->ddram_addr == sizeof(lcd->ddram))
            lcd->ddram_addr = 0;
#if DEBUG
        addstr(" =writeDDRAM=\n");
#endif
    }
}

////////////////////////////////////////////////////////////////////////////////
lcd_t *lcd_init(int x, int y, SDL_Renderer* renderer) {

    lcd_t *lcd = malloc(sizeof(lcd_t));
    if (!lcd) return NULL;

    lcd->baseX = x;
    lcd->baseY = y;
    lcd->renderer = renderer;

    memset(lcd->ddram,0x20,sizeof(lcd->ddram));
    memset(lcd->cgram,0x00,sizeof(lcd->cgram));
    memset(lcd->lcd_row1,0x00,sizeof(lcd->lcd_row1));
    memset(lcd->lcd_row2,0x00,sizeof(lcd->lcd_row2));
    lcd->lcd_active =
    lcd->ddram_addr =
    lcd->cgram_addr =
    lcd->modeset_id_s =
    lcd->disp_control_d_c_b =
    lcd->old_value =
    lcd->last_addr_is_cg =
    lcd->value8_state = 0;

    lcd->function_dl_n_f = 0x10;

    draw_lcdback(renderer, x, y);

    lcd->lcdTimer = SDL_AddTimer(100, lcd_set_tick, lcd);

    return lcd;
}

////////////////////////////////////////////////////////////////////////////////
void lcd_refresh(lcd_t *lcd, int force){

    if (force)
        draw_lcdback(lcd->renderer, lcd->baseX, lcd->baseY);
    else
        if (!lcd->lcdTick) return;

    lcd->lcdTick = 0;

    for (int i = 0; i < 16; i++){

        int addr = i;

        if (force||(lcd->lcd_row1[i] != lcd->ddram[addr])){

            lcd_out_symbol(lcd->renderer, 1+lcd->baseX+i*CHAR_HSPACE+LCDBORDER, 1+lcd->baseY+LCDBORDER, lcd->ddram[addr]);
            lcd->lcd_row1[i] = lcd->ddram[addr];
        }
    }

    for (int i = 0; i < 16; i++){

        int addr = 64+i;

        if (force||(lcd->lcd_row2[i] != lcd->ddram[addr])){

            lcd_out_symbol(lcd->renderer, 1+lcd->baseX+i*CHAR_HSPACE+LCDBORDER, 1+lcd->baseY+CHAR_VSPACE+LCDBORDER, lcd->ddram[addr]);
            lcd->lcd_row2[i] = lcd->ddram[addr];
        }
    }

    SDL_RenderPresent(lcd->renderer);
}

////////////////////////////////////////////////////////////////////////////////
void lcd_out(lcd_t *lcd, uint8_t value){

/*
D7 ... DB7
D6 ... DB6
D5 ... DB5
D4 ... DB4
D1 ... E
D0 ... RS
*/
#if DEBUG
  char buf[100];
    sprintf(buf," [OUT:%02x/%d] ",value,value8_state);
    addstr(buf);
#endif

    if ((value & 0x02) && !(lcd->old_value & 0x02)){ // E transitou de 0 para 1

        int res = 0;

        if (lcd->function_dl_n_f & 0x10){

            if (value & 0x01)
                proc_data8(lcd, value);
            else
                res = proc_cmd8(lcd, value);
        }
        else{
            if (!lcd->value8_state){
                lcd->value8 = value & 0xF0;
            }
            else{
                lcd->value8 |= (value >> 4);

                if (value & 0x01)
                    proc_data8(lcd, lcd->value8);
                else
                    res = proc_cmd8(lcd, lcd->value8);
            }
            if (!res) lcd->value8_state ^= 1;
        }
    }

    lcd->old_value = value;
}

////////////////////////////////////////////////////////////////////////////////
void lcd_close(lcd_t *lcd){

    SDL_RemoveTimer(lcd->lcdTimer);

    free(lcd);
}
