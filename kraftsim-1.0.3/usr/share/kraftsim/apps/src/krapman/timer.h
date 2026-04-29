#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdint.h>

void init_timer(void) __naked;
uint16_t set_timer(uint16_t time) __naked __sdcccall(1);
uint16_t get_timer(void) __naked;

#endif

