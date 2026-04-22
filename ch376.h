/*
 * ch376.h
 *
 *  Created on: 21 de abr. de 2026
 *      Author: milton
 */

#ifndef CH376_H_
#define CH376_H_

#include <stdio.h>
#include <stdint.h>

#define SPIBUFSZ 65
#define SECBLKSIZE 64

typedef struct {

    uint8_t spictl;
    uint8_t spistatus;
    uint8_t wptr;
    uint8_t rptr;
    uint8_t nextintr;
    uint8_t sec_chunk;
    uint32_t secnum;
    uint8_t wbuf[SPIBUFSZ];
    uint8_t rbuf[SPIBUFSZ];
    uint8_t backrbuf[SPIBUFSZ];
    FILE *fvol;
} ch376_t;

ch376_t *ch376_init(void);
void ch376_out(ch376_t *p, uint8_t port, uint8_t value);
uint8_t ch376_in(ch376_t *p, uint8_t port);
void ch376_end(ch376_t *p);

#define CMD_GET_IC_VER            0x01    //Result: 1 byte in data register, version number & 0x3F
#define CMD_CHECK_EXIST           0x06    //Test that the interface exists and works.
#define CMD_SET_USB_MODE          0x15    //Switch between different USB modes.
#define CMD_GET_STATUS            0x22    //Get interrupt status after an interrupt was triggered.
#define CMD_RD_USB_DATA0          0x27    //Read data from interrupt port, or USB receive buffer.
#define CMD_DISK_MOUNT            0x31    //Mount detected USB drive.
#define CMD_DISK_READ             0x54    //Reads Logical Sector(s).
#define CMD_DISK_RD_GO            0x55    //Requests the next 64-byte chunk for the requested Sector(s).

/////

#define ANSW_USB_INT_SUCCESS      0x14    //Operation successful, no further data
#define ANSW_USB_INT_CONNECT      0x15    //New USB device connected
#define ANSW_USB_INT_DISCONNECT   0x16    //USB device unplugged!
#define ANSW_USB_INT_USB_READY    0x18    //Device is ready
#define ANSW_USB_INT_DISK_READ    0x1d    //Disk read operation
#define ANSW_USB_INT_DISK_WRITE   0x1e    //Disk write operation
#define ANSW_RET_SUCCESS          0x51    //Operation successful
#define ANSW_RET_ABORT            0x5f    //Operation failure
#define ANSW_ERR_DISK_DISCON      0x82    //Disk disconnected

#endif /* CH376_H_ */
