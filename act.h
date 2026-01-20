/*
 * act.h
 *
 *  Created on: 20 de jan. de 2026
 *      Author: milton
 */

#ifndef ACT_H_
#define ACT_H_

#include <gtk/gtk.h>
#include <stdint.h>
#include <pthread.h>

#include "z80.h"

////////////////////////////////////////////////////////////////////////////////
typedef struct {

    int width;
    int height;
    GtkWidget *drawing_area;
    cairo_surface_t **psurface;
    uint8_t *rom;
    uint8_t *ram;
    z80_t z;
    pthread_t z80thread;
} activate_data_t;

#endif /* ACT_H_ */
