#include <stdio.h>
#include <ncurses.h>
#include <sys/select.h>
#include <pthread.h>
#include <unistd.h>

#include "ios.h"
#include "psg.h"

#define PORTBUTTONS     0x00
#define PORTDISP        0x10
#define PORTTIMER       0x54    // TIMER ENABLE/EOI
#define PORTKEY         0x55    // PS/2 DATA
#define PORTAYADDR      0x56
#define PORTAYDATA      0x57
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
uint8_t porttimer = 0;
uint8_t portserdata = 0;
uint8_t portserctl = 0;
uint8_t psgaddr = 0;

psg_t *psg;


////////////////////////////////////////////////////////////////////////////////
void default_out_callback (uint8_t port, uint8_t value){}

////////////////////////////////////////////////////////////////////////////////
void new_out_callback (uint8_t port, uint8_t value){

    switch (port){
        case PORTDISP:
            lcd_out(value);
            break;
        case PORTSERDATA:
            if (value != 0x0d){
                addch(value);
                refresh();
            }
            break;
        case PORTTIMER:
            porttimer = value;
            break;
        case PORTSERCTL:
            portserctl = value;
            break;
        case PORTAYADDR:
            psgaddr = value;
            break;
        case PORTAYDATA:
            psg_outreg(psg, psgaddr, value);
            break;
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
pthread_t serialthread;
pthread_t timerthread;
pthread_t psgthread;
int initted = 0;
int endthreads = 0;

////////////////////////////////////////////////////////////////////////////////
void *thread_timer(void *arg){

    for (;!endthreads;){

        usleep(3333);   // 300Hz
        if (porttimer & 0x01)
            fpga_status |= 0x02;
    }
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
void *thread_serial(void *arg){

    for (;!endthreads;){

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

////////////////////////////////////////////////////////////////////////////////
void *thread_psg(void *arg){

    for (;!endthreads;){
        psg_run(psg);
    }

    return NULL;
}

int presc = 0;
////////////////////////////////////////////////////////////////////////////////
void new_hw_run(void){

    if (!initted){

        psg = psg_init();

        pthread_create(&serialthread, NULL, thread_serial, NULL);
        pthread_create(&timerthread, NULL, thread_timer, NULL);
        pthread_create(&psgthread, NULL, thread_psg, NULL);

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
