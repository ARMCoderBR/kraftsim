/*
 * ch376.c
 *
 *  Created on: 21 de abr. de 2026
 *      Author: milton
 */

#include <malloc.h>
#include <string.h>
#include <unistd.h>

#include "ch376.h"

#define PORTSPISTATUS   0x5A
#define PORTSPICTL      0x5A
#define PORTSPIDATA     0x5B

#define CTL_CS          0x01
#define STAT_INTR       0x02
#define CTL_RESET       0x04


////////////////////////////////////////////////////////////////////////////////
void read_sec_chunk(ch376_t *p){
    //stub, will not implement
}

////////////////////////////////////////////////////////////////////////////////
void ch376_exec(ch376_t *p){

    if (!p->wptr) return;

    switch(p->wbuf[0]){

        case CMD_GET_IC_VER:
            if (p->wptr == 1){
                p->rbuf[0] = 0x44;
                p->rptr = 0;
                printf("\r\nCMD_GET_IC_VER\r\n");
            }
            break;

        case CMD_CHECK_EXIST:
            if (p->wptr == 2){
                p->rbuf[0] = ~p->wbuf[1];
                p->rptr = 0;
                printf("\r\nCMD_CHECK_EXIST\r\n");
            }
            break;

        case CMD_SET_USB_MODE:
            if (p->wptr == 2){
                p->rbuf[0] = ANSW_RET_SUCCESS;
                p->rptr = 0;
                p->nextintr = ANSW_USB_INT_CONNECT;
                p->spistatus &= ~STAT_INTR;
                printf("\r\nCMD_SET_USB_MODE\r\n");
            }
            break;

        case CMD_DISK_MOUNT:
            if (p->wptr == 1){
                p->rbuf[0] = ANSW_RET_SUCCESS;
                p->rptr = 0;
                p->nextintr = ANSW_USB_INT_SUCCESS;
                p->spistatus &= ~STAT_INTR;
                memcpy(p->backrbuf,(void*)"\x0cKraft80 Disk",13);
                printf("\r\nCMD_DISK_MOUNT\r\n");
            }
            break;

        case CMD_GET_STATUS:
            if (p->wptr == 1){
                p->rbuf[0] = p->nextintr;
                p->rptr = 0;
                p->spistatus |= STAT_INTR;
                printf("\r\nCMD_GET_STATUS\r\n");
            }
            break;

        case CMD_RD_USB_DATA0:
            if (p->wptr == 1){
                p->rptr = p->wptr = 0;
                memcpy(p->rbuf,p->backrbuf,SPIBUFSZ);
                printf("\r\nCMD_RD_USB_DATA0\r\n");
            }
            break;

        case CMD_DISK_READ:
            if (p->wptr == 6){
//                p->secnum = p->wbuf[4];
//                p->secnum <<= 8;
//                p->secnum |= p->wbuf[3];
//                p->secnum <<= 8;
//                p->secnum |= p->wbuf[2];
//                p->secnum <<= 8;
//                p->secnum |= p->wbuf[1];
//                p->sec_chunk = 0;
//                read_sec_chunk(p);
//                p->nextintr = ANSW_USB_INT_DISK_READ;
//                p->spistatus &= ~STAT_INTR;
                printf("\r\nCMD_DISK_READ\r\n");
            }
            break;

        case CMD_DISK_RD_GO:
            if (p->wptr == 1){
//                read_sec_chunk(p);
//                if (p->sec_chunk < 9)
//                    p->nextintr = ANSW_USB_INT_DISK_READ;
//                else
//                    p->nextintr = ANSW_USB_INT_SUCCESS;
//                p->spistatus &= ~STAT_INTR;
                printf("\r\nCMD_DISK_RD_GO\r\n");
            }
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////
ch376_t *ch376_init(char *basedir_name){

    ch376_t *p = malloc(sizeof(ch376_t));

    if (basedir_name[0])
        strcpy(p->basedir_name, basedir_name);
    else
        getcwd(p->basedir_name,sizeof(p->basedir_name));
    p->spistatus = STAT_INTR;

    printf("\r\nBaseDir:%s\r\n",p->basedir_name);

    return p;
}

////////////////////////////////////////////////////////////////////////////////
void ch376_out(ch376_t *p, uint8_t port, uint8_t value){

    uint8_t oldctl = p->spictl;

    switch(port){

        case PORTSPICTL:
            p->spictl = value;
            if (!(p->spictl & CTL_CS)){
                p->rptr = p->wptr = 0;
                if ((oldctl ^ value) & CTL_CS)
                    printf("\r\n========== END CMD\r\n");
                //printf(" CS OFF ");
            }
            else{
                if ((oldctl ^ value) & CTL_CS)
                    printf("\r\n========== BEGIN CMD\r\n");
                //printf(" CS ON ");
            }

            if (!(p->spictl & CTL_RESET)){
                if ((oldctl ^ value) & CTL_RESET)
                    printf("\r\n========== RESET OFF\r\n");
            }
            else{
                if ((oldctl ^ value) & CTL_RESET)
                    printf("\r\n========== RESET ON\r\n");
            }
            break;

        case PORTSPIDATA:
            if (p->spictl & CTL_CS){
                p->wbuf[p->wptr++] = value;
                if (p->wptr == SPIBUFSZ)
                    p->wptr = 0;
                //printf(" WR:%02x ",value);
                ch376_exec(p);
            }

            break;
    }
}

////////////////////////////////////////////////////////////////////////////////
uint8_t ch376_in(ch376_t *p, uint8_t port){

    uint8_t data = 0xff;

    switch(port){

        case PORTSPISTATUS:
            return p->spistatus;

        case PORTSPIDATA:
            if (p->spictl & CTL_CS){
                data = p->rbuf[p->rptr];
                p->rptr++;
                if (p->rptr == SPIBUFSZ)
                    p->rptr = 0;

                //printf(" RD:%02x ",data);
            }
    }

    return data;
}

////////////////////////////////////////////////////////////////////////////////
void ch376_end(ch376_t *p){

    free(p);
}
