#include <stdio.h>
#include <ncurses.h>
#include <sys/select.h>

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
uint8_t portserdata = 0;

////////////////////////////////////////////////////////////////////////////////
void default_out_callback (uint8_t port, uint8_t value){}

////////////////////////////////////////////////////////////////////////////////
void new_out_callback (uint8_t port, uint8_t value){

    if (port == PORTSERDATA){

        if (value != 0x0d){
            addch(value);
            refresh();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
uint8_t default_in_callback (uint8_t port){

    return 0xff;
}

////////////////////////////////////////////////////////////////////////////////
uint8_t new_in_callback (uint8_t port){

    char buf[128];

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
            if (portserdata == 0x0a)
                portserdata = 0x0d;
            //sprintf(buf,"RD FOM INT:%02x\n",portserdata); addstr(buf); refresh();
            return portserdata;
    }

    return 0xff;
}

////////////////////////////////////////////////////////////////////////////////
void default_hw_run(void){}

fd_set readfds;
struct timeval tv;
int presc = 0;

////////////////////////////////////////////////////////////////////////////////
void new_hw_run(void){

    if (presc){
        --presc; return;
    }

    presc = 100;

    //addch('1'); refresh();

    FD_ZERO (&readfds);
    FD_SET (0,&readfds);
    tv.tv_sec = 0;
    tv.tv_usec = 10;
    select (1,&readfds,NULL,NULL,&tv);

    if (FD_ISSET(0,&readfds)) {

        portserdata = getch();
        fpga_status |= 0x04;
        //addch(portserdata); refresh();
    }
    //addch('2'); refresh();
}

////////////////////////////////////////////////////////////////////////////////
int default_irq_sample(void){

    return 0;
}
////////////////////////////////////////////////////////////////////////////////
int new_irq_sample(void){

    return fpga_status & 0x07;
}
