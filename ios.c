#include <stdio.h>

#include "ios.h"





#define PORTBUTTONS     0x00




// SERIAL
#define PORTSERSTATUS   0x58
#define PORTSERCTL      0x58
#define    PORTSER_EN      2
#define    PORTSER_RTSON   1
#define    PORTSER_DIS     0
#define    PORTSER_RTSOFF  0
#define PORTSERDATA     0x59
////////////////////////////////////////////////////////////////////////////////
void new_out_callback (uint8_t port, uint8_t value){

    if (port == PORTSERDATA){

        printf("%c",value);
        fflush(stdout);
    }
}

////////////////////////////////////////////////////////////////////////////////
void default_out_callback (uint8_t port, uint8_t value){}

////////////////////////////////////////////////////////////////////////////////
uint8_t new_in_callback (uint8_t port){

    if (port == PORTBUTTONS)
        return 0x7f;

    if (port == PORTSERSTATUS)
        return 0;

    return 0xff;
}

////////////////////////////////////////////////////////////////////////////////
uint8_t default_in_callback (uint8_t port){

    return 0xff;
}
