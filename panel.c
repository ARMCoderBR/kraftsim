/*
 * panel.c
 *
 *  Created on: 17 de fev. de 2026
 *      Author: milton
 */

#include <malloc.h>
#include "panel.h"

////////////////////////////////////////////////////////////////////////////////
static void sdl_drawpanelback(panel_t *panel){

    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = panel->panel_width;
    rect.h = panel->panel_height;

    SDL_SetRenderDrawColor(panel->renderer_panel, 8, 36, 8, 255);
    SDL_RenderFillRect(panel->renderer_panel, &rect);
}

////////////////////////////////////////////////////////////////////////////////
panel_t *panel_init(){

    panel_t *panel = malloc(sizeof(panel_t));
    if (!panel) return NULL;

    panel->panel_width = 482;
    panel->panel_height = 244;

    panel->window_panel = SDL_CreateWindow(
            "Kraft80 Panel",             // Window title
            SDL_WINDOWPOS_UNDEFINED,     // Initial x position
            SDL_WINDOWPOS_UNDEFINED,     // Initial y position
            panel->panel_width,            // Width in pixels
            panel->panel_height,           // Height in pixels
            SDL_WINDOW_SHOWN             // Flags (SDL_WINDOW_SHOWN is default)
        );

    if (!panel->window_panel){
        printf("Window for Panel could not be created! SDL_Error: %s\n", SDL_GetError()); fflush(stdout);
        exit(0);
    }

    panel->renderer_panel = SDL_CreateRenderer(panel->window_panel, -1, SDL_RENDERER_SOFTWARE);

    if (panel->renderer_panel == NULL) {
        printf("Renderer for Panel could not be created! SDL_Error: %s\n", SDL_GetError()); fflush(stdout);
        SDL_DestroyWindow(panel->window_panel);
        return NULL;
    }

    sdl_drawpanelback(panel);

    panel->lcd = lcd_init(64, 34, panel->renderer_panel);
    panel->leds = leds_init(12, 8, panel->renderer_panel);
    panel->buttons = buttons_init(10, 190, panel->renderer_panel);

    return panel;
}

////////////////////////////////////////////////////////////////////////////////
void panel_check_refresh(panel_t *panel){

    if (!panel->wminimized_panel){

        if (panel->repaint_window_panel){

            sdl_drawpanelback(panel);
            leds_refresh(panel->leds,1);
            lcd_refresh(panel->lcd, 1);
            buttons_refresh(panel->buttons,1);
            panel->repaint_window_panel = 0;
        }
        else{

            leds_refresh(panel->leds,0);
            lcd_refresh(panel->lcd, 0);
            buttons_refresh(panel->buttons,0);
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
void panel_close (panel_t *panel){

    leds_close(panel->leds);
    lcd_close(panel->lcd);
    buttons_close(panel->buttons);

    SDL_DestroyRenderer(panel->renderer_panel);
    SDL_DestroyWindow(panel->window_panel);
    free(panel);
}
