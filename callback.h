/*
 * callback.h
 *
 *  Created on: 14 de jan. de 2026
 *      Author: milton
 */

#ifndef CALLBACK_H_
#define CALLBACK_H_

#include <stdint.h>

typedef void (*outcallback_t)(uint8_t port, uint8_t value);
typedef uint8_t (*incallback_t)(uint8_t port);
typedef void (*hw_run_t)(void);
typedef int (*irq_sample_t)(void);

#endif /* CALLBACK_H_ */
