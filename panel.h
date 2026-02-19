/*
 * panel.h
 *
 *  Created on: 17 de fev. de 2026
 *      Author: milton
 */

#ifndef PANEL_H_
#define PANEL_H_

#include <stdint.h>
#include <SDL2/SDL.h>

#include "leds.h"
#include "lcd.h"
#include "buttons.h"

////////////////////////////////////////////////////////////////////////////////
typedef struct {

    SDL_Window* window_panel;
    SDL_Renderer* renderer_panel;
    int panel_width;
    int panel_height;
    int wminimized_panel;
    int repaint_window_panel;
    leds_t *leds;
    lcd_t *lcd;
    buttons_t *buttons;
} panel_t;

////////////////////////////////////////////////////////////////////////////////

panel_t *panel_init(int show_window);
void panel_check_refresh(panel_t *panel);
void panel_close (panel_t *panel);

#endif /* PANEL_H_ */
