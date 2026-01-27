#include <stdint.h>
#include <string.h>
#include <malloc.h>

#include <SDL2/SDL.h>

#include "vga.h"
#include "vgafont.h"

#define COLS 80
#define ROWS 48

#define GCOLS 320
#define GROWS 240

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color_t;

const color_t colors[] = {

        { 0x00, 0x00, 0x00},   // BLACK
        { 0x00, 0x00, 0xc0},   // BLUE
        { 0x00, 0xa0, 0x00},   // GREEN
        { 0x00, 0xc0, 0xc0},   // CYAN
        { 0xa0, 0x00, 0x00},   // RED
        { 0xc0, 0x00, 0xc0},   // MAGENTA
        { 0xa0, 0x60, 0x20},   // BROWN
        { 0xc0, 0xc0, 0xc0},   // GRAY
        { 0x80, 0x80, 0x80},   // DARK GRAY
        { 0x00, 0x00, 0xf0},   // LIGHT BLUE
        { 0x00, 0xf0, 0x00},   // LIGHT GREEN
        { 0x00, 0xf0, 0xf0},   // LIGHT CYAN
        { 0xf0, 0x00, 0x00},   // LIGHT RED
        { 0xf0, 0x00, 0xf0},   // LIGHT MAGENTA
        { 0xf0, 0xf0, 0x00},   // YELLOW
        { 0xf0, 0xf0, 0xf0}    // WHITE
};

////////////////////////////////////////////////////////////////////////////////
static void vga_set_textmode(vga_t *vga){

    if (vga->displayBuffer)
        free (vga->displayBuffer);

    vga->rdaddr = vga->wraddr = 0;
    vga->scrollreg = 0;

    vga->displayBuffer = malloc(COLS*ROWS);
    memset(vga->displayBuffer,0x20,COLS*ROWS);
    vga->mode = 0;
}

////////////////////////////////////////////////////////////////////////////////
static void vga_set_graphmode(vga_t *vga){

    if (vga->displayBuffer)
        free (vga->displayBuffer);

    vga->rdaddr = vga->wraddr = 0;

    vga->displayBuffer = malloc(GCOLS*GROWS/2);
    memset(vga->displayBuffer,0x00,GCOLS*GROWS);
    vga->mode = 1;
}

////////////////////////////////////////////////////////////////////////////////
static Uint32 vga_set_tick(Uint32 interval, void *param){

    vga_t *vga = param;

    vga->vgaTick = 1;
    return interval;
}

////////////////////////////////////////////////////////////////////////////////
vga_t *vga_init(SDL_Renderer* renderer){

    vga_t *vga = malloc(sizeof(vga_t));
    if (!vga) return NULL;

    SDL_Surface* tempSurface = SDL_CreateRGBSurface
        (0,     /*Uint32 flags*/
         16,    /*int width*/
         16,    /*int height*/
         32,    /*int depth*/
         0,     /*Uint32 Rmask*/
         0,     /*Uint32 Gmask*/
         0,     /*Uint32 Bmask*/
         0      /*Uint32 Amask*/
         );

    vga->renderer = renderer;

    SDL_Rect rect;
    rect.w = 2;
    rect.h = 2;

    SDL_Renderer* tempRenderer = SDL_CreateSoftwareRenderer(tempSurface);

    int vgafont_offset = 0;
    for (int charcode = 0; charcode < 256; charcode++){

        if (SDL_SetRenderDrawColor(tempRenderer, 0, 0, 0, 255) < 0) {
            // Handle error (optional)
            SDL_Log("SDL_SetRenderDrawColor failed: %s", SDL_GetError());
            exit(0);
        }

        // Clear the entire screen/window to the set color
        if (SDL_RenderClear(tempRenderer) < 0) {
            // Handle error (optional)
            SDL_Log("SDL_RenderClear failed: %s", SDL_GetError());
            exit(0);
        }

        if (SDL_SetRenderDrawColor(tempRenderer, 0, 255, 0, 255) < 0) {
            // Handle error (optional)
            SDL_Log("SDL_SetRenderDrawColor failed: %s", SDL_GetError());
            exit(0);
        }

        for (int row = 0; row < 8; row++){

            uint8_t b = vgafont[vgafont_offset+row];
            uint8_t mask = 128;

            for (int col = 0; col < 8; col++){

                if (b & mask){

                    rect.x = col*2;
                    rect.y = row*2;

                    SDL_RenderFillRect(tempRenderer, &rect);
                }

                mask >>= 1;
            }
        }
        vgafont_offset += 8;

        vga->fontTexture[charcode] = SDL_CreateTextureFromSurface(renderer, tempSurface);
    }

    SDL_FreeSurface(tempSurface);
    SDL_DestroyRenderer(tempRenderer);

    vga_set_textmode(vga);

    vga->vgaTimer = SDL_AddTimer(33, vga_set_tick, vga);

    return vga;
}

////////////////////////////////////////////////////////////////////////////////
void vga_refresh(vga_t *vga, int force){

    if (!vga->vgaTick) return;
    vga->vgaTick = 0;

    SDL_Rect rect;

    if (vga->mode == 0){

        rect.x = 0;
        rect.y = 0;
        rect.w = 16;
        rect.h = 16;

        int addr = vga->scrollreg * COLS;

        for (int row = 0; row < ROWS; row++){

            for (int col = 0; col < COLS; col++){

                SDL_RenderCopy(vga->renderer, vga->fontTexture[vga->displayBuffer[addr++]], NULL, &rect);

                if (addr >= (ROWS*COLS))
                    addr = 0;

                rect.x += 16;
            }

            rect.x = 0;
            rect.y += 20;
        }
    }
    else{

        if (force){
            rect.x = 0;
            rect.y = 0;
            rect.w = 4;
            rect.h = 4;

            int addr = 0;

            for (int row = 0; row < GROWS; row++){

                for (int col = 0; col < GCOLS; col+=2){

                    int b = vga->displayBuffer[addr];
                    int bh = b >> 4;
                    b &= 0x0f;
                    SDL_SetRenderDrawColor(vga->renderer, colors[bh].r, colors[bh].g, colors[bh].b, 255);
                    SDL_RenderFillRect(vga->renderer, &rect);

                    rect.x += 4;

                    SDL_SetRenderDrawColor(vga->renderer, colors[b].r, colors[b].g, colors[b].b, 255);
                    SDL_RenderFillRect(vga->renderer, &rect);

                    rect.x += 4;
                    addr++;
                }

                rect.x = 0;
                rect.y += 4;
            }
        }
    }

    SDL_RenderPresent(vga->renderer);
}

////////////////////////////////////////////////////////////////////////////////
void vga_update(vga_t *vga){

    if (!vga->mode) return;

    SDL_Rect rect;
    rect.x = 4*((vga->wraddr<<1)%320);
    rect.y = 4*(vga->wraddr/160);
    rect.w = 4;
    rect.h = 4;

    int b = vga->displayBuffer[vga->wraddr];
    int bh = b >> 4;
    b &= 0x0f;
    SDL_SetRenderDrawColor(vga->renderer, colors[bh].r, colors[bh].g, colors[bh].b, 255);
    SDL_RenderFillRect(vga->renderer, &rect);

    rect.x += 4;

    SDL_SetRenderDrawColor(vga->renderer, colors[b].r, colors[b].g, colors[b].b, 255);
    SDL_RenderFillRect(vga->renderer, &rect);
}

////////////////////////////////////////////////////////////////////////////////
void vga_out(vga_t *vga, uint8_t port, uint8_t value){
#define PORTDATA        0x50    // Video ports
#define PORTADDRL       0x51
#define PORTADDRH       0x52
#define PORTMODE        0x53

    switch (port){

        case PORTDATA:
            vga->displayBuffer[vga->wraddr] = value;
            vga_update(vga);
            vga->wraddr++;
            break;
        case PORTADDRL:
            vga->wraddr &= ~0xFF;
            vga->wraddr |= value;
            vga->rdaddr &= ~0xFF;
            vga->rdaddr |= value;
            break;
        case PORTADDRH:
            vga->wraddr &= 0xFF;
            vga->wraddr |= (value << 8);
            vga->rdaddr &= 0xFF;
            vga->rdaddr |= (value << 8);
            break;
        case PORTMODE:
            if (value == 0)
                vga_set_textmode(vga);
            else
            if(value == 1)
                vga_set_graphmode(vga);
            else
            if (value & 0x10){
                vga->scrollreg &= ~0x0f;
                vga->scrollreg |= (value & 0x0f);
            }
            else
            if (value & 0x20){
                vga->scrollreg &= 0x0f;
                vga->scrollreg |= ((value & 0x0f)<<4);
            }
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////
uint8_t vga_in(vga_t *vga, uint8_t port){

    if (port == PORTDATA){
        uint8_t val = vga->displayBuffer[vga->rdaddr];
        if (vga->mode == 0)
            vga->rdaddr++;
        return val;
    }
    else
        return 0xff;
}

////////////////////////////////////////////////////////////////////////////////
void vga_close(vga_t *vga){

    SDL_RemoveTimer(vga->vgaTimer);

    for (int charcode = 0; charcode < 256; charcode++)
        SDL_DestroyTexture(vga->fontTexture[charcode]);

    free(vga->displayBuffer);

    free(vga);
}
