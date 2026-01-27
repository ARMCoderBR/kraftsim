/*
 * lcd.h
 *
 *  Created on: 20 de jan. de 2026
 *      Author: milton
 */

#ifndef LCD_H_
#define LCD_H_

#include <stdint.h>

#include "main.h"

void lcd_init(main_data_t *act);

void lcd_out(uint8_t value);

void lcd_refresh(main_data_t *act);

#endif /* LCD_H_ */
