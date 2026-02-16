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
#include "psg.h"

typedef struct {

    uint8_t fpga_status;
    uint8_t porttimer;
    uint8_t portserdata;
    uint8_t portserctl;
    uint8_t psgaddr;

    uint16_t buttons_state;

    psg_t *psg;

    void *maindata;

    uint8_t ps2_queue[16];
    int ps2_head;
    int ps2_tail;
    int ps2_qty;

    pthread_t serialthread;
    pthread_t timerthread;
    pthread_t psgthread;
    pthread_t sdleventthread;
    int endthreads;

    int presc;
} ios_t;

void default_out_callback (ios_t *ios, uint8_t port, uint8_t value);
void new_out_callback (ios_t *ios, uint8_t port, uint8_t value);

uint8_t default_in_callback (ios_t *ios, uint8_t port);
uint8_t new_in_callback (ios_t *ios, uint8_t port);

void default_hw_run(ios_t *ios);
void new_hw_run(ios_t *ios);

int default_irq_sample(ios_t *ios);
int new_irq_sample(ios_t *ios);

ios_t *ios_init(void *new_maindata);
void ios_close(ios_t *ios);

#endif /* IOS_H_ */
