#include <malloc.h>
#include <unistd.h>
#include <string.h>

#include "psg.h"
#include "sound.h"

////////////////////////////////////////////////////////////////////////////////
psg_t *psg_init(void){

    psg_t *p = malloc(sizeof(psg_t));

    p->r0 = p->r1 = p->r2 = p->r3 = p->r4 = p->r5 = p->r6 = p->r7 =
    p->r8 = p->r9 = p->r10 = p->r11 = p->r12 = p->r13 = 0;

    p->toneACount = p->toneBCount = p->toneCCount = 0;
    p->chanAOut = p->chanBOut = p->chanCOut = 0;

    memset(p->sfr,0,sizeof(p->sfr));
    p->sfr[0] = 1;
    p->noiseCount = 0;
    p->noiseOut = 0;

    p->s = sound_init();

    return p;
}

////////////////////////////////////////////////////////////////////////////////
uint8_t sfr_run(psg_t *p){  // Fibonacci LFSR

    uint8_t aux;

    aux = p->sfr[10] ^ p->sfr[12] ^ p->sfr[13] ^ p->sfr[15];

    for (int i = 15; i > 0; i--)
        p->sfr[i] = p->sfr[i-1];
    p->sfr[0] = aux;

    return aux;
}

////////////////////////////////////////////////////////////////////////////////
void envelope_run(psg_t *p){

    if (p->enveState){

        switch(p->r13&0x0f){

            case 0:
            case 1:
            case 2:
            case 3:
            case 9:
                if (p->enveValue)
                    --p->enveValue;
                else
                    p->enveState = 0;
                break;

            case 4:
            case 5:
            case 6:
            case 7:
            case 15:
                if (p->enveValue<15)
                    ++p->enveValue;
                else{
                    p->enveValue = 0;
                    p->enveState = 0;
                }
                break;

            case 8:
                --p->enveValue;
                break;

            case 10:
            case 14:
                if (p->enveState == 1){
                    if (p->enveValue)
                        --p->enveValue;
                    else
                        p->enveState = 2;
                }
                else{   // ==2
                    if (p->enveValue<15)
                        ++p->enveValue;
                    else
                        p->enveState = 1;
                }
                break;

            case 12:
                ++p->enveValue;
                break;

            case 13:
                if (p->enveValue<15)
                    ++p->enveValue;
                else{
                    p->enveState = 0;
                }
                break;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
void envelope_start(psg_t *p){

    p->envePresc = p->enveCount = 0;

    switch(p->r13&0x0f){

        case 0:
        case 1:
        case 2:
        case 3:
        case 8:
        case 9:
        case 10:
        case 11:
            p->enveValue = 15;
            p->enveState = 1;
            break;

        case 4:
        case 5:
        case 6:
        case 7:
        case 12:
        case 13:
        case 15:
            p->enveValue = 0;
            p->enveState = 1;
            break;

        case 14:
            p->enveValue = 15;
            p->enveState = 2;
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////
void psg_run(psg_t *p){

    // Clock assumed: 2MHz
    // ToneA period = (clock/16)/(r1r0[11:0])
    // ToneB period = (clock/16)/(r3r2[11:0])
    // ToneC period = (clock/16)/(r5r4[11:0])

    int toneADiv = p->r1&0x0f; toneADiv <<= 8; toneADiv |= (p->r0&0xFE);
    int toneBDiv = p->r3&0x0f; toneBDiv <<= 8; toneBDiv |= (p->r2&0xFE);
    //printf("DivA:%04X  DivB:%04x\n",toneAdiv,toneBdiv);
    int toneCDiv = p->r5&0x0f; toneCDiv <<= 8; toneCDiv |= (p->r4&0xFE);
    int noiseDiv = p->r6&0x1f;

    int enveDiv = p->r12; enveDiv <<= 8; enveDiv |= p->r11;

    for (int samples = 0; samples < 20; samples++){

        if (toneADiv){

            p->toneACount+=2;
            if (p->toneACount == toneADiv){
                p->toneACount = 0;
                p->chanAOut ^= 1;
            }
        }
        else
            p->chanAOut = 0;

        /////////////////////

        if (toneBDiv){

            p->toneBCount+=2;
            if (p->toneBCount == toneBDiv){
                p->toneBCount = 0;
                p->chanBOut ^= 1;
            }
        }
        else
            p->chanBOut = 0;

        /////////////////////

        if (toneCDiv){

            p->toneCCount+=2;
            if (p->toneCCount == toneCDiv){
                p->toneCCount = 0;
                p->chanCOut ^= 1;
            }
        }
        else
            p->chanCOut = 0;

        /////////////////////

        if (noiseDiv){

            p->noiseCount++;
            if (p->noiseCount == noiseDiv){
                p->noiseCount = 0;
                p->noiseOut = sfr_run(p);
            }
        }

        /////////////////////

        if (!p->envePresc){ // Prescale from CLK/16 to CLK/256

            if (enveDiv){

                //printf("[%d %d]",enveDiv,p->enveCount);
                p->enveCount++;
                if (p->enveCount == enveDiv){
                    p->enveCount = 0;
                    envelope_run(p);
                }
            }
        }
        p->envePresc++; p->envePresc &= 15;

        /////////////////////

        uint16_t outval = 0;

        int r7 = p->r7;

        if((r7 & 0x09) == 0x09){
            outval += 12*p->enveValue;
        }
        else{
            int amp = p->r8&0x10? p->enveValue : p->r8&15;
            if (!(r7&0x01)) outval += p->chanAOut*8*amp;
            if (!(r7&0x08)) outval += p->noiseOut*4*amp;
        }

        if((r7 & 0x12) == 0x12){
            outval += 12*p->enveValue;
        }
        else{
            int amp = p->r9&0x10? p->enveValue : p->r9&15;
            //printf("R9:%d AMPB:%d  ",p->r9,amp);
            if (!(r7&0x02)) outval += p->chanBOut*8*amp;
            if (!(r7&0x10)) outval += p->noiseOut*4*amp;
        }

        if((r7 & 0x24) == 0x24){
            outval += 12*p->enveValue;
        }
        else{
            int amp = p->r10&0x10? p->enveValue : p->r10&15;
            if (!(r7&0x04)) outval += p->chanCOut*8*amp;
            if (!(r7&0x20)) outval += p->noiseOut*4*amp;
        }

        sound_send_sample(p->s,outval>>1);
    }
}

////////////////////////////////////////////////////////////////////////////////
void psg_outreg(psg_t *p,int reg,int a){

    if (p == NULL){printf("NULL REG\n"); return ; }
    switch(reg){
        case 0:
            p->r0 = a;
            p->toneACount = 0;
            break;
        case 1:
            p->r1 = a;
            p->toneACount = 0;
            break;
        case 2:
            p->r2 = a;
            p->toneBCount = 0;
            break;
        case 3:
            p->r3 = a;
            p->toneBCount = 0;
            break;
        case 4:
            p->r4 = a;
            p->toneCCount = 0;
            break;
        case 5:
            p->r5 = a;
            p->toneCCount = 0;
            break;
        case 6:
            p->r6 = a;
            break;
        case 7:
            p->r7 = a;
            break;
        case 8:
            p->r8 = a;
            break;
        case 9:
            p->r9 = a;
            break;
        case 10:
            p->r10 = a;
            break;
        case 11:
            p->r11 = a;
            break;
        case 12:
            p->r12 = a;
            break;
        case 13:
            p->r13 = a;
            envelope_start(p);
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////
void psg_end(psg_t *p){

    sound_end(p->s);

    free(p);
}
