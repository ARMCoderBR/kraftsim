/*
 * leds.h
 *
 *  Created on: 21 de jan. de 2026
 *      Author: milton
 */

#ifndef LEDS_H_
#define LEDS_H_

#include <stdint.h>

#include "act.h"

void leds_init(activate_data_t *act);

void leds_out(uint8_t value);


#endif /* LEDS_H_ */
