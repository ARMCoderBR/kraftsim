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
#include <gtk/gtk.h>

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

////////////////////////////////////////////////////////////////////////////////
static void clear_surface(activate_data_t *act) {

    cairo_t *cr;

    cairo_surface_t **psurface = act->psurface;
    cr = cairo_create(*psurface);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_paint(cr);
    cairo_destroy(cr);
}

////////////////////////////////////////////////////////////////////////////////
void *z80runner(activate_data_t *act){

    char buf[255];

    int num_steps = 0;

#define NBP 4
    uint16_t bp[1+NBP] = {0};

    for (;!act->z.halted;){

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
/* Create a new surface of the appropriate size to store our scribbles */
static gboolean configure_event_cb(GtkWidget *widget, GdkEventConfigure *event,
        gpointer data) {

    if (event->type == GDK_CONFIGURE) {
        //printf("configure_event_cb()\n");

        activate_data_t *act = (activate_data_t*)data;
        cairo_surface_t **psurface = act->psurface;

        if (*psurface)
            cairo_surface_destroy(*psurface);

        *psurface = gdk_window_create_similar_surface(gtk_widget_get_window(widget),
                CAIRO_CONTENT_COLOR, gtk_widget_get_allocated_width(widget),
                gtk_widget_get_allocated_height(widget));

        act->width = event->width;
        act->height = event->height;

        /* Initialize the surface to black */
        clear_surface(act);

        lcd_init(act);
        leds_init(act);
        vga_init(act);

        pthread_create(&act->z80thread, NULL, z80runner, data);
        //mandel(act);
    }

    /* We've handled the configure event, no need for further processing. */
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/* Redraw the screen from the surface. Note that the ::draw
 * signal receives a ready-to-be-used cairo_t that is already
 * clipped to only draw the exposed areas of the widget
 */
static gboolean draw_cb(GtkWidget *widget, cairo_t *cr, gpointer data) {

    if (widget == NULL) return FALSE;
    if (cr == NULL) return FALSE;
    if (data == NULL) return FALSE;

    //printf("draw_cb()\n");

    activate_data_t *act = (activate_data_t*)data;
    cairo_surface_t **psurface = act->psurface;

    cairo_set_source_surface(cr, *psurface, 0, 0);
    cairo_paint(cr);

    return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
static void close_window(GtkWidget *object, gpointer data) {

    //printf("close_window()\n");

    activate_data_t *act = (activate_data_t*)data;
    cairo_surface_t **psurface = act->psurface;

    if (*psurface)
        cairo_surface_destroy(*psurface);
}

////////////////////////////////////////////////////////////////////////////////
static void activate(GtkApplication *app, gpointer user_data) {

    GtkWidget *window;
    activate_data_t *act = (activate_data_t*) user_data;

    window = gtk_application_window_new(app);

    gtk_window_set_title(GTK_WINDOW(window), "KRAFT80 Monitor");

    g_signal_connect(window, "destroy", G_CALLBACK (close_window), act);

    gtk_container_set_border_width(GTK_CONTAINER(window), 8);

    act->drawing_area = gtk_drawing_area_new();

    /* set a minimum size */
    gtk_widget_set_size_request(act->drawing_area, act->width, act->height);

    GtkWidget *main_grid = gtk_grid_new();
    gtk_container_add (GTK_CONTAINER (window), main_grid);

    gtk_widget_set_hexpand(main_grid,TRUE);
    gtk_widget_set_vexpand(main_grid,TRUE);

    g_signal_connect(act->drawing_area, "configure-event",
            G_CALLBACK (configure_event_cb), act);

    /* Signals used to handle the backing surface */
    g_signal_connect(act->drawing_area, "draw", G_CALLBACK (draw_cb), act);

    gtk_widget_set_hexpand(act->drawing_area,TRUE);
    gtk_widget_set_vexpand(act->drawing_area,TRUE);

#if 0
    /* Event signals */
    g_signal_connect(drawing_area, "motion-notify-event",
            G_CALLBACK (motion_notify_event_cb), psurface);

    g_signal_connect(drawing_area, "button-press-event",
            G_CALLBACK (button_press_event_cb), psurface);
#endif

//    GtkWidget *zoomin = gtk_button_new_with_label ("ZOOM+");
//    g_signal_connect (zoomin, "clicked", G_CALLBACK (fn_zoomin), user_data);
//
//    GtkWidget *zoomout = gtk_button_new_with_label ("ZOOM-");
//    g_signal_connect (zoomout, "clicked", G_CALLBACK (fn_zoomout), user_data);
//
//    GtkWidget *zoomres = gtk_button_new_with_label ("ZOOMRes");
//    g_signal_connect (zoomres, "clicked", G_CALLBACK (fn_zoomres), user_data);
//
//    GtkWidget *setcenter = gtk_button_new_with_label ("SetCenter");
//    g_signal_connect (setcenter, "clicked", G_CALLBACK (fn_setcenter), user_data);
//
//    GtkWidget *itup = gtk_button_new_with_label ("IT+");
//    g_signal_connect (itup, "clicked", G_CALLBACK (fn_itup), user_data);
//
//    GtkWidget *itdown = gtk_button_new_with_label ("IT-");
//    g_signal_connect (itdown, "clicked", G_CALLBACK (fn_itdown), user_data);
//
//    GtkWidget *itzero = gtk_button_new_with_label ("IT0");
//    g_signal_connect (itzero, "clicked", G_CALLBACK (fn_itzero), user_data);
//
//    gtk_grid_attach ((GtkGrid*)main_grid, zoomin,    1, 1, 1, 1);
//    gtk_grid_attach ((GtkGrid*)main_grid, zoomout,   2, 1, 1, 1);
//    gtk_grid_attach ((GtkGrid*)main_grid, zoomres,   3, 1, 1, 1);
//    gtk_grid_attach ((GtkGrid*)main_grid, setcenter, 4, 1, 1, 1);
//    gtk_grid_attach ((GtkGrid*)main_grid, itup,      5, 1, 1, 1);
//    gtk_grid_attach ((GtkGrid*)main_grid, itdown,    6, 1, 1, 1);
//    gtk_grid_attach ((GtkGrid*)main_grid, itzero,    7, 1, 1, 1);

    gtk_grid_attach ((GtkGrid*)main_grid, act->drawing_area,1, 2, 7, 1);

    gtk_widget_add_events(act->drawing_area, GDK_BUTTON_PRESS_MASK /*| GDK_POINTER_MOTION_MASK*/);

    /* Ask to receive events the drawing area doesn't normally
     * subscribe to. In particular, we need to ask for the
     * button press and motion notify events that want to handle.
     */
//    gtk_widget_set_events(drawing_area,
//            gtk_widget_get_events(drawing_area) | GDK_BUTTON_PRESS_MASK
//                    | GDK_POINTER_MOTION_MASK);

    gtk_widget_show_all(window);
}

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char *argv[]){

    //keyb_run();
    //exit(0);


    cairo_surface_t *surface = NULL;
    GtkApplication *app;
    activate_data_t act;

    act.width = 1280;
    act.height = 960 + 60;
    act.drawing_area = NULL;
    act.psurface = &surface;

    act.rom = malloc(ROMSZ);
    act.ram = malloc(RAMSZ);

    char procname[100];
    sprintf(procname, "kraftsim-%d.br.com.cpstecnologia", getpid());
    app = gtk_application_new(procname, G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK (activate), &act);

    memset(act.rom,0xff,ROMSZ);
    memset(act.ram,0x00,RAMSZ);

    if (romprog(act.rom,ROMSZ,act.ram,RAMBASE,RAMSZ) < 0){

        addstr("Error loading ROM!\n");
        return -1;
    }

    initscr();

    idlok(stdscr,TRUE);
    scrollok(stdscr,TRUE);
    noecho();

    //addstr("\n=== RUN ===\n\n"); refresh();

    z80_initialize(&act.z, act.rom, ROMSZ, act.ram, RAMBASE, RAMSZ, new_out_callback, new_in_callback, new_hw_run, new_irq_sample);

    z80_reset(&act.z);

#if RUN
    z80_noprint(&act.z);
    act.z.running = 1;
#else
    z80_print(&act.z);
    act->z.running = 0;
#endif

    //addstr("\n=== LOOP ===\n\n");

    /*int status =*/ g_application_run(G_APPLICATION(app), 1, argv);

    //z80runner(&act);

    z80_dump_mem(&act.z, RAMBASE,512);

    getch();
    endwin();

    g_object_unref(app);

    return 0;
}
