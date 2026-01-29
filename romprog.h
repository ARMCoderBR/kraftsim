/*
 * romprog.h
 *
 *  Created on: 15 de out de 2022
 *      Author: milton
 */

#ifndef ROMPROG_H_
#define ROMPROG_H_

#include <stdint.h>

//int apprun_kraftsim(uint8_t *rom, uint16_t romsize, uint8_t *ram, uint16_t rambase, uint16_t ramsize, char *fname);
//int romload(uint8_t *rom, uint16_t romsize, char *rom_fname);
int memload(uint8_t *mem, uint16_t membase, uint16_t memsize, char *fname);

#endif /* ROMPROG_H_ */
