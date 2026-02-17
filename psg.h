#ifndef __PSG_C__
#define __PSG_C__

#include <stdint.h>
#include <pthread.h>

#include "sound.h"

#define NUM_STAGES 64
typedef struct {

    uint8_t r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r13;

    uint8_t chanAOut,chanBOut,chanCOut;
    uint16_t toneACount,toneBCount,toneCCount;
    uint8_t noiseOut;
    uint8_t noiseCount;
    uint8_t sfr[16];

    uint8_t enveValue;
    uint8_t envePresc;
    uint16_t enveCount;
    uint16_t enveState;

    uint16_t values[NUM_STAGES];

    sound_t *s;
} psg_t;


psg_t *psg_init(void);
void psg_reset(psg_t *p);
void psg_run(psg_t *p);
void psg_outreg(psg_t *p,int reg,int a);
void psg_end(psg_t *p);

#endif
