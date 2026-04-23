#ifndef __INPUTS_H__
#define __INPUTS_H__

void init_inputs(void) __naked;
uint8_t update_inputs(void) __naked __sdcccall(1);

#endif

