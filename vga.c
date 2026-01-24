#include <gtk/gtk.h>
#include <stdint.h>
#include <string.h>
#include <ncurses.h>


#include "vga.h"
#include "vgafont.h"

#if 0
static cairo_surface_t *stamp_surface[256];
#endif


////////////////////////////////////////////////////////////////////////////////
void vga_init(activate_data_t *act){

#if 0
    GdkWindow *window = gtk_widget_get_window(act->drawing_area);
    cairo_t *cr0;

    int vgafont_offset = 0;
    for (int charcode = 0; charcode < 256; charcode++){

        stamp_surface[charcode] = gdk_window_create_similar_surface(
                window, CAIRO_CONTENT_COLOR_ALPHA, 16, 16);

        cr0 = cairo_create(stamp_surface[charcode]);

        cairo_set_source_rgb(cr0, 0, 0, 0);
        cairo_paint(cr0);

        cairo_set_source_rgb(cr0, 0, 1, 0);
        for (int row = 0; row < 7; row++){

            uint8_t b = vgafont[vgafont_offset+row];
            uint8_t mask = 128;

            for (int col = 0; col < 7; col++){

                if (b & mask){

                    cairo_rectangle(cr0, col*2, row*2, 2, 2);
                    cairo_fill(cr0);
                }

                mask >>= 1;
            }
        }
        vgafont_offset += 8;
        cairo_destroy(cr0);
    }

    /* Paint to the surface, where we store our state */
    cairo_t *cr = cairo_create(*act->psurface);

    cairo_set_source_surface(cr, stamp_surface[65], 0, 0);
    cairo_paint(cr);
    cairo_set_source_surface(cr, stamp_surface[66], 16, 0);
    cairo_paint(cr);
    cairo_set_source_surface(cr, stamp_surface[67], 32, 0);
    cairo_paint(cr);

    /* Now invalidate the affected region of the drawing area. */
    gtk_widget_queue_draw_area(act->drawing_area, 0, 0, 48, 16);

    cairo_destroy(cr);
#endif
}

