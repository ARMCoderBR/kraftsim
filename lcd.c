#include <gtk/gtk.h>
#include <stdint.h>
#include <string.h>
#include <ncurses.h>

#include "lcdrom.h"
#include "lcd.h"

#define DEBUG 0

////////////////////////////////////////////////////////////////////////////////
/* Draw a rectangle on the surface at the given position */
void draw_lcdback(GtkWidget *widget, gdouble x, gdouble y,
        cairo_surface_t *surface) {

    cairo_t *cr;

    /* Paint to the surface, where we store our state */
    cr = cairo_create(surface);

    cairo_rectangle(cr, x, y, (16*17), (2*27));
    //cairo_set_source_rgb(cr, 0, 0.4, 0.2);
    cairo_set_source_rgb(cr, 0, 0.8, 0.3);
    cairo_fill(cr);

    cairo_destroy(cr);

    /* Now invalidate the affected region of the drawing area. */
    gtk_widget_queue_draw_area(widget, x, y, (16*17), (2*27));
}

////////////////////////////////////////////////////////////////////////////////
/* Draw a rectangle on the surface at the given position */
void draw_lcdpoint(GtkWidget *widget, gdouble x, gdouble y,
        cairo_surface_t *surface, int onoff) {

    cairo_t *cr;

    /* Paint to the surface, where we store our state */
    cr = cairo_create(surface);

    cairo_rectangle(cr, x, y, 3, 3);
    cairo_set_source_rgb(cr, 0, 0.7, 0.3);
    cairo_fill(cr);


    cairo_rectangle(cr, x, y, 2, 2);
    if (onoff)
        cairo_set_source_rgb(cr, 0, 0, 0);
    else
        cairo_set_source_rgb(cr, 0, 0.8, 0.3);
    cairo_fill(cr);

    cairo_destroy(cr);

    /* Now invalidate the affected region of the drawing area. */
    gtk_widget_queue_draw_area(widget, x, y, 3, 3);
}

////////////////////////////////////////////////////////////////////////////////
void lcd_out_symbol(activate_data_t *act, int px, int py, uint8_t code){

    if (code > 0x7f) code = 0x20;

    int rom_ofs = 8*(code-0x10);

    for (int row = 0; row < 8; row++){

        int colmask = 0x10;

        for (int col = 0; col < 5; col++){

            int state = 0;
            if (lcdrom[rom_ofs] & colmask)
                state = 1;

            draw_lcdpoint(act->drawing_area, px+3*col, py+3*row,
                    *act->psurface, state);

            colmask >>= 1;
        }

        rom_ofs++;
    }
}

uint8_t ddram[64+40];
uint8_t cgram[64];
uint8_t lcd_row1[16];
uint8_t lcd_row2[16];

uint8_t lcd_active;
uint8_t ddram_addr;
uint8_t cgram_addr;
uint8_t modeset_id_s;
uint8_t disp_control_d_c_b;
uint8_t function_dl_n_f;
uint8_t old_value;
uint8_t last_addr_is_cg;
uint8_t value8;
uint8_t value8_state;

activate_data_t *lcdact;

////////////////////////////////////////////////////////////////////////////////
void lcd_refresh(activate_data_t *act){

    for (int i = 0; i < 16; i++){

        int addr = i;

        if (lcd_row1[i] != ddram[addr]){

            lcd_out_symbol(act, 1+i*17, 1+0, ddram[addr]);
            lcd_row1[i] = ddram[addr];
        }
    }

    for (int i = 0; i < 16; i++){

        int addr = 64+i;

        if (lcd_row2[i] != ddram[addr]){

            lcd_out_symbol(act, 1+i*17, 1+27, ddram[addr]);
            lcd_row2[i] = ddram[addr];
        }
    }
}

gboolean on_timeout(gpointer user_data) {

    activate_data_t *act = (activate_data_t *)user_data;
    lcd_refresh(act);

    // Return TRUE to continue the timer, FALSE to stop
    return TRUE;
}
////////////////////////////////////////////////////////////////////////////////
void lcd_init(activate_data_t *act) {

    memset(ddram,0x20,sizeof(ddram));
    memset(cgram,0x00,sizeof(cgram));
    memset(lcd_row1,0x00,sizeof(lcd_row1));
    memset(lcd_row2,0x00,sizeof(lcd_row2));
    lcd_active =
    ddram_addr =
    cgram_addr =
    modeset_id_s =
    disp_control_d_c_b =
    old_value =
    last_addr_is_cg =
    value8_state = 0;

    function_dl_n_f = 0x10;

    draw_lcdback(act->drawing_area, 0, 0,
            *act->psurface);

    g_timeout_add(100, on_timeout, act);
    //lcd_refresh(act);
}

////////////////////////////////////////////////////////////////////////////////
int proc_cmd8(uint8_t value){

#if DEBUG
    char buf[100];
    sprintf(buf," [PROC:%02x] ",value);
    addstr(buf);
#endif

    if (value & 0x80){  // Sets DDRAM Addr
        ddram_addr = value & 0x7f;
        last_addr_is_cg = 0;
#if DEBUG
        addstr(" =setDDRAM=\n");
#endif
        }
    else
    if (value & 0x40){  // Sets CGRAM Addr
        cgram_addr = value & 0x3f;
        last_addr_is_cg = 1;
#if DEBUG
        addstr(" =setCGRAM=\n");
#endif
        }
    else
    if (value & 0x20){  // Sets DL, N and F
        function_dl_n_f = value & 0x1f;
        value8_state = 0;
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
        disp_control_d_c_b = value & 0x07;
    }
    else
    if (value & 0x04){  // Sets Cursor move dir and disp shift during write
        modeset_id_s = value & 0x03;
#if DEBUG
        addstr(" =setIDS=\n");
#endif
        }
    else
    if (value & 0x02){  // Return Home

        ddram_addr = 0;
        last_addr_is_cg = 0;
#if DEBUG
        addstr(" =ReturnHOME=\n");
#endif
        }
    else
    if (value & 0x01){  // Clear Display
        memset(ddram,0x20,sizeof(ddram));
        ddram_addr = 0;
        last_addr_is_cg = 0;
#if DEBUG
        addstr(" =ClearDISP=\n");
#endif
        }
#if DEBUG
    else
        addstr(" =?????=\n");
#endif

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
void proc_data8(uint8_t value){

#if DEBUG
    char buf[100];
    sprintf(buf," [DATA:%02x] ",value);
    addstr(buf);
#endif

    if (last_addr_is_cg){
        cgram[cgram_addr++] = value;
        if (cgram_addr == sizeof(cgram))
            cgram_addr = 0;
#if DEBUG
        addstr(" =writeCGRAM=\n");
#endif
        }
    else{
        ddram[ddram_addr++] = value;
        if (ddram_addr == sizeof(ddram))
            ddram_addr = 0;
#if DEBUG
        addstr(" =writeDDRAM=\n");
#endif
        //lcd_refresh(lcdact);
    }
}

////////////////////////////////////////////////////////////////////////////////
void lcd_out(uint8_t value){

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

    if ((value & 0x02) && !(old_value & 0x02)){ // E transitou de 0 para 1

        int res = 0;

        if (function_dl_n_f & 0x10){

            if (value & 0x01)
                proc_data8(value);
            else
                res = proc_cmd8(value);
        }
        else{
            if (!value8_state){
                value8 = value & 0xF0;
            }
            else{
                value8 |= (value >> 4);

                if (value & 0x01)
                    proc_data8(value8);
                else
                    res = proc_cmd8(value8);
            }
            if (!res) value8_state ^= 1;
        }
    }

    old_value = value;
}
