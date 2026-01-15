/*
 * ios.h
 *
 *  Created on: 14 de jan. de 2026
 *      Author: milton
 */

#ifndef IOS_H_
#define IOS_H_

#include <stdint.h>

#include "callback.h"
#include "z80.h"

void default_out_callback (uint8_t port, uint8_t value);
void new_out_callback (uint8_t port, uint8_t value);

uint8_t default_in_callback (uint8_t port);
uint8_t new_in_callback (uint8_t port);

void default_hw_run();
void new_hw_run();

int default_irq_sample(void);
int new_irq_sample(void);

#endif /* IOS_H_ */
