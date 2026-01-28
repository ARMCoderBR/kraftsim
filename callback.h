/*
 * callback.h
 *
 *  Created on: 14 de jan. de 2026
 *      Author: milton
 */

#ifndef CALLBACK_H_
#define CALLBACK_H_

#include <stdint.h>
#include "ios.h"

typedef void (*outcallback_t)(ios_t *ios, uint8_t port, uint8_t value);
typedef uint8_t (*incallback_t)(ios_t *ios, uint8_t port);
typedef void (*hw_run_t)(ios_t *ios);
typedef int (*irq_sample_t)(ios_t *ios);

#endif /* CALLBACK_H_ */
