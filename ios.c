#include <stdio.h>
#include "ios.h"

#define PORTBUTTONS     0x00

#define PORTTIMER       0x54    // TIMER EOI
#define PORTKEY         0x55    // PS/2 DATA
#define PORTSERSTATUS   0x58
#define PORTSERCTL      0x58
#define    PORTSER_EN      2
#define    PORTSER_RTSON   1
#define    PORTSER_DIS     0
#define    PORTSER_RTSOFF  0
#define PORTSERDATA     0x59    // SERIAL DATA
#define PORTFPGASTATUS  0x5F

////////////////////////////////////////////////////////////////////////////////
uint8_t fpga_status = 0;

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

    switch (port){

        case PORTBUTTONS:
            return 0x7f;

        case PORTSERSTATUS:
            return 0;

        case PORTFPGASTATUS:
            return fpga_status;

        case PORTKEY:
            fpga_status &= ~0x01;
            return 0xFF;

        case PORTTIMER:
            fpga_status &= ~0x02;
            return 0xFF;

        case PORTSERDATA:
            fpga_status &= ~0x04;
            return 0xFF;
    }

    return 0xff;
}

////////////////////////////////////////////////////////////////////////////////
uint8_t default_in_callback (uint8_t port){

    return 0xff;
}
