#include <stdio.h>
#include <ncurses.h>
#include <sys/select.h>
#include <pthread.h>
#include <unistd.h>

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
uint8_t portserctl = 0;

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
    else
    if (port == PORTSERCTL){

        portserctl = value;
    }

}

////////////////////////////////////////////////////////////////////////////////
uint8_t default_in_callback (uint8_t port){

    return 0xff;
}

pthread_mutex_t ios_mutex = PTHREAD_MUTEX_INITIALIZER;;

////////////////////////////////////////////////////////////////////////////////
uint8_t new_in_callback (uint8_t port){

    //char buf[128];

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
            pthread_mutex_lock(&ios_mutex);
            fpga_status &= ~0x04;
            if (portserdata == 0x0a)
                portserdata = 0x0d;
            //sprintf(buf,"RD FOM INT:%02x\n",portserdata); addstr(buf); refresh();
            pthread_mutex_unlock(&ios_mutex);
            return portserdata;
    }

    return 0xff;
}

////////////////////////////////////////////////////////////////////////////////
void default_hw_run(void){}

fd_set readfds;
struct timeval tv;
pthread_t mythread;
int initted = 0;

////////////////////////////////////////////////////////////////////////////////
void *thread_ios(void *arg){

    for (;;){

        if ( ( (portserctl & (PORTSER_EN|PORTSER_RTSON)) == (PORTSER_EN|PORTSER_RTSON) )
               &&  (!(fpga_status & 0x04) ) ){

            FD_ZERO (&readfds);
            FD_SET (0,&readfds);
            tv.tv_sec = 0;
            tv.tv_usec = 10;
            select (1,&readfds,NULL,NULL,&tv);

            if (FD_ISSET(0,&readfds)) {
                pthread_mutex_lock(&ios_mutex);
                portserdata = getch();
                fpga_status |= 0x04;
                pthread_mutex_unlock(&ios_mutex);
                //addch(portserdata); refresh();
            }
        }
        else
            usleep(10);
    }
    return NULL;
}

int presc = 0;
////////////////////////////////////////////////////////////////////////////////
void new_hw_run(void){

    if (!initted){

        /*int res =*/ pthread_create(&mythread, NULL, thread_ios, NULL);

        initted = 1;
    }

    if (!presc){

        usleep(20);
        presc = 50;
    }
    else
        --presc;
}

////////////////////////////////////////////////////////////////////////////////
int default_irq_sample(void){

    return 0;
}
////////////////////////////////////////////////////////////////////////////////
int new_irq_sample(void){

    return fpga_status & 0x07;
}
