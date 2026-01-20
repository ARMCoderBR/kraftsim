/*
 * lcd.h
 *
 *  Created on: 20 de jan. de 2026
 *      Author: milton
 */

#ifndef LCD_H_
#define LCD_H_

#include <gtk/gtk.h>
#include <stdint.h>

#include "act.h"

void lcd_init(activate_data_t *act);

void lcd_out(uint8_t value);

#endif /* LCD_H_ */
