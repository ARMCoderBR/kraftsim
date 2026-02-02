/*
 * romprog.h
 *
 *  Created on: 15 de out de 2022
 *      Author: milton
 */

#ifndef ROMPROG_H_
#define ROMPROG_H_

#include <stdint.h>

int memload_ihx(uint8_t *mem, uint16_t membase, uint16_t memsize, char *fname, uint16_t load_offset);

int memload_bin(uint8_t *mem, uint16_t membase, uint16_t memsize, char *fname, uint16_t load_offset);

#endif /* ROMPROG_H_ */
