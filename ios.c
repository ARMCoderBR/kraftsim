#include <stdio.h>
#include <ncurses.h>
#include <sys/select.h>
#include <pthread.h>
#include <unistd.h>
#include "z80.h"
#include "ios.h"
#include "psg.h"
#include "leds.h"
#include "lcd.h"
#include "main.h"
#include "vga.h"
#include "main.h"

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
void default_out_callback (ios_t *ios, uint8_t port, uint8_t value){}

////////////////////////////////////////////////////////////////////////////////
void new_out_callback (ios_t *ios, uint8_t port, uint8_t value){

    main_data_t *maindata = ios->maindata;

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
            vga_out(maindata->vga, port, value, maindata->sdl->wminimized);
            break;
        case PORTTIMER:
            ios->porttimer = value;
            break;
        case PORTSERCTL:
            ios->portserctl = value;
            break;
        case PORTAYADDR:
            ios->psgaddr = value;
            break;
        case PORTAYDATA:
            psg_outreg(ios->psg, ios->psgaddr, value);
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////
uint8_t default_in_callback (ios_t *ios, uint8_t port){

    return 0xff;
}

pthread_mutex_t ios_mutex = PTHREAD_MUTEX_INITIALIZER;

////////////////////////////////////////////////////////////////////////////////
uint8_t new_in_callback (ios_t *ios, uint8_t port){

    main_data_t *maindata = ios->maindata;

    switch (port){

        case PORTDATA:
            return vga_in(maindata->vga, port);

        case PORTBUTTONS:
            return ios->buttons_state;

        case PORTSERSTATUS:
            return 0;

        case PORTFPGASTATUS:
            return ios->fpga_status;

        case PORTKEY:
            pthread_mutex_lock(&ios_mutex);
            uint8_t val = 0xff;
            if (ios->ps2_qty){

                val = ios->ps2_queue[ios->ps2_tail++];
                if (ios->ps2_tail == sizeof(ios->ps2_queue))
                    ios->ps2_tail = 0;
                ios->ps2_qty--;
            }
            if (!ios->ps2_qty)
                ios->fpga_status &= ~0x01;
            pthread_mutex_unlock(&ios_mutex);
            return val;

        case PORTTIMER:
            ios->fpga_status &= ~0x02;
            return 0xFF;

        case PORTSERDATA:
            pthread_mutex_lock(&ios_mutex);
            ios->fpga_status &= ~0x04;
            if (ios->portserdata == 0x0a)
                ios->portserdata = 0x0d;

            pthread_mutex_unlock(&ios_mutex);
            return ios->portserdata;
    }

    return 0xff;
}

////////////////////////////////////////////////////////////////////////////////
void default_hw_run(ios_t *ios){}

////////////////////////////////////////////////////////////////////////////////
void *thread_timer(void *arg){

    ios_t *ios = arg;

    time_t start = time(NULL);
    time_t now;
    int ticks = 0;

    for (;!ios->endthreads;){

        now = time(NULL);

        int deltaticks = 300 * (now - start);
        if (labs(deltaticks) > (300*300)){
            start = now;
            ticks = deltaticks = 0;
        }

        int adjust = 0;
        int delta2 = ticks - deltaticks;
        if (delta2 < -200)
            adjust = -133;
        else
        if (delta2 < -100)
            adjust = -33;
        else
        if (delta2 > 200)
            adjust = 133;
        else
        if (delta2 > 100)
            adjust = 33;

        usleep(3333+adjust);   // 300Hz

        ticks++;

        if (ios->porttimer & 0x01)
            ios->fpga_status |= 0x02;
    }
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
void *thread_serial(void *arg){

    ios_t *ios = arg;

    fd_set readfds;
    struct timeval tv;

    for (;!ios->endthreads;){

        if ( ( (ios->portserctl & (PORTSER_EN|PORTSER_RTSON)) == (PORTSER_EN|PORTSER_RTSON) )
               &&  (!(ios->fpga_status & 0x04) ) ){

            FD_ZERO (&readfds);
            FD_SET (0,&readfds);
            tv.tv_sec = 0;
            tv.tv_usec = 10;
            select (1,&readfds,NULL,NULL,&tv);

            if (FD_ISSET(0,&readfds)) {
                pthread_mutex_lock(&ios_mutex);
                ios->portserdata = getch();
                ios->fpga_status |= 0x04;
                pthread_mutex_unlock(&ios_mutex);
            }
        }
        else
            usleep(10);
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
void *thread_psg(void *arg){

    ios_t *ios = arg;

    for (;!ios->endthreads;){
        psg_run(ios->psg);
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
void ps2_insert(ios_t *ios, uint8_t code){

    pthread_mutex_lock(&ios_mutex);

    if (ios->ps2_qty < sizeof(ios->ps2_queue)){

        ios->ps2_queue[ios->ps2_head++] = code;
        if (ios->ps2_head == sizeof(ios->ps2_queue))
            ios->ps2_head = 0;

        ios->fpga_status |= 0x01;
        ios->ps2_qty++;
    }

    pthread_mutex_unlock(&ios_mutex);
}

////////////////////////////////////////////////////////////////////////////////
typedef struct {

    int keycode;
    uint8_t scancode1;
    uint8_t scancode2;
} kcode_t;

const kcode_t kcodes[] = {
        {'0',0x45,0x00},
        {'1',0x16,0x00},
        {'2',0x1e,0x00},
        {'3',0x26,0x00},
        {'4',0x25,0x00},
        {'5',0x2e,0x00},
        {'6',0x36,0x00},
        {'7',0x3d,0x00},
        {'8',0x3e,0x00},
        {'9',0x46,0x00},
        {'a',0x1c,0x00},
        {'b',0x32,0x00},
        {'c',0x21,0x00},
        {'d',0x23,0x00},
        {'e',0x24,0x00},
        {'f',0x2b,0x00},
        {'g',0x34,0x00},
        {'h',0x33,0x00},
        {'i',0x43,0x00},
        {'j',0x3b,0x00},
        {'k',0x42,0x00},
        {'l',0x4b,0x00},
        {'m',0x3a,0x00},
        {'n',0x31,0x00},
        {'o',0x44,0x00},
        {'p',0x4D,0x00},
        {'q',0x15,0x00},
        {'r',0x2d,0x00},
        {'s',0x1b,0x00},
        {'t',0x2c,0x00},
        {'u',0x3c,0x00},
        {'v',0x2a,0x00},
        {'w',0x1d,0x00},
        {'x',0x22,0x00},
        {'y',0x35,0x00},
        {'z',0x1a,0x00},
        {'-',0x4e,0x00},
        {'=',0x55,0x00},
        {',',0x41,0x00},
        {'.',0x49,0x00},
        {'/',0x51,0x00},
        {'[',0x5b,0x00},
        {']',0x5d,0x00},
        {'\\',0x61,0x00},
        {';',0x4a,0x00},
        {'`',0x54,0x00},
        {0x27,0x0e,0x00},
        {'~',0x52,0x00},
        {SDLK_BACKSPACE,0x66,0x00},
        {SDLK_TAB,0x0d,0x00},
        {SDLK_RETURN,0x5a,0x00},
        {SDLK_ESCAPE,0x76,0x00},
        {SDLK_SPACE,0x29,0x00},
        {SDLK_RIGHT,0xe0,0x74},
        {SDLK_LEFT,0xe0,0x6b},
        {SDLK_DOWN,0xe0,0x72},
        {SDLK_UP,0xe0,0x75},
        {SDLK_CAPSLOCK,0x58,0x00},
        {SDLK_LSHIFT,0x12,0x00},
        {SDLK_RSHIFT,0x59,0x00},
        {SDLK_LCTRL,0x14,0x00},
        {SDLK_RCTRL,0xe0,0x14},
        {SDLK_INSERT,0xe0,0x70},
        {SDLK_DELETE,0xe0,0x71},
        {SDLK_HOME,0xe0,0x6c},
        {SDLK_END,0xe0,0x69},
        {SDLK_PAGEUP,0xe0,0x7d},
        {SDLK_PAGEDOWN,0xe0,0x7a},
        {0x00,0x00,0x00}
};

////////////////////////////////////////////////////////////////////////////////
const kcode_t *find_kcode(int keycode){

    const kcode_t *k = kcodes;

    for (;k->keycode;){

        if (k->keycode == keycode) return k;
        k++;
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
void proc_keydown(ios_t *ios, int asccode){

    main_data_t *maindata = ios->maindata;

    switch(asccode){

        case SDLK_F1:
            ios->buttons_state &= ~0b00000001;
            break;
        case SDLK_F2:
            ios->buttons_state &= ~0b00000010;
            break;
        case SDLK_F3:
            ios->buttons_state &= ~0b00000100;
            break;
        case SDLK_F4:
            ios->buttons_state &= ~0b00001000;
            break;
        case SDLK_F5:
            ios->buttons_state &= ~0b00010000;
            break;
        case SDLK_F6:
            ios->buttons_state &= ~0b00100000;
            break;
        case SDLK_F7:
            ios->buttons_state &= ~0b01000000;
            break;
        case SDLK_F8:
            ios->buttons_state &= ~0b10000000;
            break;
        case SDLK_F9:
            z80_reset(&maindata->z);
            leds_reset(maindata->leds);
            psg_reset(ios->psg);
            vga_reset(maindata->vga);
            break;
        case SDLK_F12:
            z80_break(&maindata->z);
            break;

        default:
            const kcode_t *k = find_kcode(asccode);
            if (k){
                ps2_insert(ios, k->scancode1);
                if (k->scancode2)
                    ps2_insert(ios, k->scancode2);
            }
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////
void proc_keyup(ios_t *ios, int asccode){

    switch(asccode){

        case SDLK_F1:
            ios->buttons_state |= 0b00000001;
            break;
        case SDLK_F2:
            ios->buttons_state |= 0b00000010;
            break;
        case SDLK_F3:
            ios->buttons_state |= 0b00000100;
            break;
        case SDLK_F4:
            ios->buttons_state |= 0b00001000;
            break;
        case SDLK_F5:
            ios->buttons_state |= 0b00010000;
            break;
        case SDLK_F6:
            ios->buttons_state |= 0b00100000;
            break;
        case SDLK_F7:
            ios->buttons_state |= 0b01000000;
            break;
        case SDLK_F8:
            ios->buttons_state |= 0b10000000;
            break;
        default:
            const kcode_t *k = find_kcode(asccode);
            if (k){
                if (!k->scancode2){
                    ps2_insert(ios, 0xF0);
                    ps2_insert(ios, k->scancode1);
                }
                else{
                    ps2_insert(ios, k->scancode1);
                    ps2_insert(ios, 0xF0);
                    ps2_insert(ios, k->scancode2);
                }
            }
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////
void *thread_sdl_events(void *arg){

    ios_t *ios = arg;
    main_data_t *maindata = ios->maindata;
    SDL_Event event;

    for (;!ios->endthreads;){

        while (SDL_PollEvent(&event)) {

            if (event.type == SDL_WINDOWEVENT){
                if (event.window.event == SDL_WINDOWEVENT_RESTORED){
                    maindata->sdl->wminimized = 0;
                    maindata->sdl->repaint_window = 1;
                }
                else if (event.window.event == SDL_WINDOWEVENT_MINIMIZED){
                    maindata->sdl->wminimized = 1;
                }
            } else if (event.type == SDL_QUIT) {
                // Handle quit event
            } else if (event.type == SDL_KEYDOWN) {
//                char buf[80];
//                sprintf(buf,"Event:%02x Scancode:%02x ",event.key.keysym.sym, event.key.keysym.scancode);
//                addstr(buf); refresh();
                if (event.key.keysym.sym == 0x40000000){
                    if (event.key.keysym.scancode == 0x34)
                        event.key.keysym.sym = '~';
                    if (event.key.keysym.scancode == 0x2f)
                        event.key.keysym.sym = '`';
                }
                proc_keydown(ios, event.key.keysym.sym);
            } else if (event.type == SDL_KEYUP) {
                proc_keyup(ios, event.key.keysym.sym);
            }
        }
        usleep(10000);
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
void new_hw_run(ios_t *ios){

    if (!ios->presc){

        usleep(20);
        ios->presc = 50;
    }
    else
        --ios->presc;
}

////////////////////////////////////////////////////////////////////////////////
int default_irq_sample(ios_t *ios){

    return 0;
}
////////////////////////////////////////////////////////////////////////////////
int new_irq_sample(ios_t *ios){

    return ios->fpga_status & 0x07;
}

////////////////////////////////////////////////////////////////////////////////
ios_t *ios_init(void *main){

    ios_t *ios = malloc(sizeof(ios_t));
    if (!ios) return NULL;

    ios->maindata = main;

    ios->fpga_status = 0;
    ios->porttimer = 0;
    ios->portserdata = 0;
    ios->portserctl = 0;
    ios->psgaddr = 0;
    ios->buttons_state = 0xff;
    ios->endthreads = 0;
    ios->presc = 0;

    ios->psg = psg_init();

    ios->ps2_head = ios->ps2_tail = ios->ps2_qty = 0;

    pthread_create(&ios->serialthread, NULL, thread_serial, ios);
    pthread_create(&ios->timerthread, NULL, thread_timer, ios);
    pthread_create(&ios->psgthread, NULL, thread_psg, ios);
    pthread_create(&ios->sdleventthread, NULL, thread_sdl_events, ios);

    return ios;
}

////////////////////////////////////////////////////////////////////////////////
void ios_close(ios_t *ios){

    ios->endthreads = 1;

    pthread_join(ios->serialthread, NULL);
    pthread_join(ios->timerthread, NULL);
    pthread_join(ios->psgthread, NULL);
    pthread_join(ios->sdleventthread, NULL);

    psg_end(ios->psg);

    free(ios);
}
