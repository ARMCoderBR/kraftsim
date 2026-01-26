/*
 * vga.h
 *
 *  Created on: 22 de jan. de 2026
 *      Author: milton
 */

#ifndef VGA_H_
#define VGA_H_

#include <stdint.h>

#include "act.h"

void vga_init(activate_data_t *act);

void vga_out(uint8_t value);

void vga_close(activate_data_t *act);

#endif /* VGA_H_ */
