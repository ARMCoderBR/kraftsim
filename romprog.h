/*
 * romprog.h
 *
 *  Created on: 15 de out de 2022
 *      Author: milton
 */

#ifndef ROMPROG_H_
#define ROMPROG_H_

#include <stdint.h>

int romprog(uint8_t *rom, uint16_t romsize, uint8_t *ram, uint16_t rambase, uint16_t ramsize);

#endif /* ROMPROG_H_ */
