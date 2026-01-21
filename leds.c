#include <gtk/gtk.h>
#include <stdint.h>
#include <string.h>
#include <ncurses.h>

#include "leds.h"

uint8_t leds_port;
uint8_t leds_port_old;

#define LEDS_X_OFFSET 380
#define LEDS_Y_OFFSET 505

////////////////////////////////////////////////////////////////////////////////
void leds_refresh(activate_data_t *act){

    if (leds_port == leds_port_old) return;

    cairo_t *cr;

    /* Paint to the surface, where we store our state */
    cr = cairo_create(*act->psurface);

    uint8_t mask = 0x80;
    for (int i = 0; i < 8; i++){

        if ((leds_port ^ leds_port_old) & mask){

            cairo_rectangle(cr, LEDS_X_OFFSET+20*i, LEDS_Y_OFFSET, 16, 16);

            if (leds_port & mask)
                cairo_set_source_rgb(cr, 1, 1, 0);
            else
                cairo_set_source_rgb(cr, 0.2, 0.2, 0);

            cairo_fill(cr);

            /* Now invalidate the affected region of the drawing area. */
            gtk_widget_queue_draw_area(act->drawing_area, LEDS_X_OFFSET+20*i, LEDS_Y_OFFSET, 16, 16);
        }

        mask >>= 1;
    }

    leds_port_old = leds_port;

    cairo_destroy(cr);
}

////////////////////////////////////////////////////////////////////////////////
gboolean on_timeout_led(gpointer user_data) {

    activate_data_t *act = (activate_data_t *)user_data;
    leds_refresh(act);

    // Return TRUE to continue the timer, FALSE to stop
    return TRUE;
}


////////////////////////////////////////////////////////////////////////////////
void leds_init(activate_data_t *act){

    leds_port = 0;
    leds_port_old = 0xff;

    g_timeout_add(11, on_timeout_led, act);
}

////////////////////////////////////////////////////////////////////////////////
void leds_out(uint8_t value){

    leds_port = value;
}
