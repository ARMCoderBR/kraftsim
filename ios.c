#include <stdio.h>
#include <ncurses.h>
#include <sys/select.h>
#include <pthread.h>
#include <unistd.h>

#include "ios.h"
#include "psg.h"
#include "leds.h"
#include "lcd.h"
#include "main.h"
#include "vga.h"

#define PORTLEDS        0x00
#define PORTBUTTONS     0x00
#define PORTDISP        0x10

#define PORTDATA        0x50    // Video ports
#define PORTADDRL       0x51
#define PORTADDRH       0x52
#define PORTMODE        0x53

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

uint8_t buttons_state = 0xff;

psg_t *psg;

main_data_t *maindata;


uint8_t ps2_queue[16];
int ps2_head;
int ps2_tail;
int ps2_qty;

////////////////////////////////////////////////////////////////////////////////
void default_out_callback (uint8_t port, uint8_t value){}

////////////////////////////////////////////////////////////////////////////////
void new_out_callback (uint8_t port, uint8_t value){

    switch (port){
        case PORTLEDS:
            leds_out(maindata->leds,value);
            break;
        case PORTDISP:
            lcd_out(maindata->lcd, value);
            break;
        case PORTSERDATA:
            if (value != 0x0d){
                addch(value);
                refresh();
            }
            break;
        case PORTDATA:
        case PORTADDRL:
        case PORTADDRH:
        case PORTMODE:
            vga_out(maindata->vga, port, value);
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

        case PORTDATA:
            return vga_in(maindata->vga, port);

        case PORTBUTTONS:
            return buttons_state;

        case PORTSERSTATUS:
            return 0;

        case PORTFPGASTATUS:
            return fpga_status;

        case PORTKEY:
            pthread_mutex_lock(&ios_mutex);
            uint8_t val = 0xff;
            if (ps2_qty){

                val = ps2_queue[ps2_tail++];
                if (ps2_tail == sizeof(ps2_queue))
                    ps2_tail = 0;
                ps2_qty--;
            }
            if (!ps2_qty)
                fpga_status &= ~0x01;
            pthread_mutex_unlock(&ios_mutex);
            return val;

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
pthread_t ps2thread;
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

////////////////////////////////////////////////////////////////////////////////
void ps2_insert(uint8_t code){

    pthread_mutex_lock(&ios_mutex);

    if (ps2_qty < sizeof(ps2_queue)){

        ps2_queue[ps2_head++] = code;
        if (ps2_head == sizeof(ps2_queue))
            ps2_head = 0;

        fpga_status |= 0x01;
        ps2_qty++;
    }

    pthread_mutex_unlock(&ios_mutex);
}



////////////////////////////////////////////////////////////////////////////////

const uint8_t xlataz[] = { 0x1c, 0x32, 0x21, 0x23, 0x24, 0x2b, 0x34, 0x33,
                           0x43, 0x3b, 0x42, 0x4b, 0x3a, 0x31, 0x44, 0x4D,
                           0x15, 0x2d, 0x1b, 0x2c, 0x3c, 0x2a, 0x1d, 0x22,
                           0x35, 0x1a };

const uint8_t xlat09[] = { 0x45, 0x16, 0x1e, 0x26, 0x25, 0x2e, 0x3d, 0x3e,
                           0x3e, 0x46 };


void proc_keydown(int asccode){

    switch(asccode){

        case SDLK_F1:
            buttons_state &= ~0b10000000;
            break;
        case SDLK_F2:
            buttons_state &= ~0b01000000;
            break;
        case SDLK_F3:
            buttons_state &= ~0b00100000;
            break;
        case SDLK_F4:
            buttons_state &= ~0b00010000;
            break;
        case SDLK_F5:
            buttons_state &= ~0b00001000;
            break;
        case SDLK_F6:
            buttons_state &= ~0b00000100;
            break;
        case SDLK_F7:
            buttons_state &= ~0b00000010;
            break;
        case SDLK_F8:
            buttons_state &= ~0b00000001;
            break;

        case 8:
            ps2_insert(0x66);
            break;

        case 13:
            ps2_insert(0x5A);
            break;

        case 32:
            ps2_insert(0x29);
            break;

        default:
            if ((asccode >= 'a') && (asccode <= 'z')){
                ps2_insert(xlataz[asccode-'a']);
            }
            else
            if ((asccode >= '0') && (asccode <= '9')){
                ps2_insert(xlat09[asccode-'0']);
            }
            break;
    }
}

void proc_keyup(int asccode){

    switch(asccode){

        case SDLK_F1:
            buttons_state |= 0b10000000;
            break;
        case SDLK_F2:
            buttons_state |= 0b01000000;
            break;
        case SDLK_F3:
            buttons_state |= 0b00100000;
            break;
        case SDLK_F4:
            buttons_state |= 0b00010000;
            break;
        case SDLK_F5:
            buttons_state |= 0b00001000;
            break;
        case SDLK_F6:
            buttons_state |= 0b00000100;
            break;
        case SDLK_F7:
            buttons_state |= 0b00000010;
            break;
        case SDLK_F8:
            buttons_state |= 0b00000001;
            break;

        case 8:
            ps2_insert(0xF0);
            ps2_insert(0x66);
            break;

        case 13:
            ps2_insert(0xF0);
            ps2_insert(0x5A);
            break;

        case 32:
            ps2_insert(0xF0);
            ps2_insert(0x29);
            break;

        default:
            if ((asccode >= 'a') && (asccode <= 'z')){
                ps2_insert(0xf0);
                ps2_insert(xlataz[asccode-'a']);
            }
            else
            if ((asccode >= '0') && (asccode <= '9')){
                ps2_insert(0xf0);
                ps2_insert(xlat09[asccode-'0']);
            }
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////
void *thread_keyb_ps2(void *arg){

    SDL_Event event;

    for (;!endthreads;){

        while (SDL_PollEvent(&event)) {
            //printf("poll\n");
            if (event.type == SDL_QUIT) {
                // Handle quit event
            } else if (event.type == SDL_KEYDOWN) {
                printf("DOWN:EventSym:%d\n",event.key.keysym.sym);
                proc_keydown(event.key.keysym.sym);
//                switch (event.key.keysym.sym) {
//
//                    case SDLK_UP:
//                        // Handle up arrow key press
//                        break;
//                    case SDLK_DOWN:
//                        // Handle down arrow key press
//                        break;
//                    case SDLK_a:
//                        // Handle 'a' key press
//                        break;
//                    // ... more keys
//                }
            } else if (event.type == SDL_KEYUP) {
                printf("UP:EventSym:%d\n",event.key.keysym.sym);
                proc_keyup(event.key.keysym.sym);
//                switch (event.key.keysym.sym) {
//                    case SDLK_ESCAPE:
//                        // Handle escape key release
//                        break;
//                    // ... more keys
//                }
            }
        }
        usleep(10000);
    }

    return NULL;
}


int presc = 0;
////////////////////////////////////////////////////////////////////////////////
void new_hw_run(void){

    if (!initted){

        buttons_state = 0xff;

        psg = psg_init();

        ps2_head = ps2_tail = ps2_qty = 0;

        pthread_create(&serialthread, NULL, thread_serial, NULL);
        pthread_create(&timerthread, NULL, thread_timer, NULL);
        pthread_create(&psgthread, NULL, thread_psg, NULL);
        pthread_create(&ps2thread, NULL, thread_keyb_ps2, NULL);

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

////////////////////////////////////////////////////////////////////////////////
void ios_init(void *act){

    maindata = act;
}
