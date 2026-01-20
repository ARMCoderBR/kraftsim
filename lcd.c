#include <gtk/gtk.h>

#include "lcdrom.h"
#include "lcd.h"

////////////////////////////////////////////////////////////////////////////////
/* Draw a rectangle on the surface at the given position */
void draw_lcdback(GtkWidget *widget, gdouble x, gdouble y,
        cairo_surface_t *surface) {

    cairo_t *cr;

    /* Paint to the surface, where we store our state */
    cr = cairo_create(surface);

    cairo_rectangle(cr, x, y, (16*17), (2*27));
    //cairo_set_source_rgb(cr, 0, 0.4, 0.2);
    cairo_set_source_rgb(cr, 0.2, 1, 0.4);
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
    gtk_widget_queue_draw_area(widget, x, y, 1, 1);
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

////////////////////////////////////////////////////////////////////////////////
void lcd_init(activate_data_t *act) {

    draw_lcdback(act->drawing_area, 0, 0,
            *act->psurface);

    for (int i = 0; i < 16; i++)
        lcd_out_symbol(act, i*17, 0, 'A'+i);

    for (int i = 0; i < 16; i++)
        lcd_out_symbol(act, i*17, 26, 'Q'+i);
}
