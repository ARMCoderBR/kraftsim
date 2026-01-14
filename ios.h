/*
 * ios.h
 *
 *  Created on: 14 de jan. de 2026
 *      Author: milton
 */

#ifndef IOS_H_
#define IOS_H_

#include <stdint.h>

//#define OUT_CALLBACK_FND(f)  void f (uint8_t port, uint8_t value)
//#define OUT_CALLBACK_FN(f)  void (*f) (uint8_t port, uint8_t value)


typedef void (*outcallback_t)(uint8_t port, uint8_t value);

void new_out_callback (uint8_t port, uint8_t value);
void default_out_callback (uint8_t port, uint8_t value);


typedef uint8_t (*incallback_t)(uint8_t port);

uint8_t new_in_callback (uint8_t port);
uint8_t default_in_callback (uint8_t port);

#endif /* IOS_H_ */
