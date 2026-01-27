/*
 * lcd.h
 *
 *  Created on: 20 de jan. de 2026
 *      Author: milton
 */

#ifndef LCD_H_
#define LCD_H_

#include <stdint.h>
#include <SDL2/SDL.h>

typedef struct {

    SDL_TimerID lcdTimer;
    int lcdTick;
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

    //main_data_t *lcdact;
    int baseX,baseY;
    SDL_Renderer* renderer;

} lcd_t;

lcd_t *lcd_init(int x, int y, SDL_Renderer* renderer);

void lcd_out(lcd_t *lcd, uint8_t value);

void lcd_refresh(lcd_t *lcd, int force);

#endif /* LCD_H_ */
