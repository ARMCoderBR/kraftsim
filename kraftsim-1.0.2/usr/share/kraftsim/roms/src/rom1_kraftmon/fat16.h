/*
 * fat.h
 *
 *  Created on: 7 de abr. de 2026
 *      Author: milton
 */

#ifndef FAT_H_
#define FAT_H_

#include <stdint.h>

#define DIRENTRYSIZE 32
#define MAXSECSIZE   512
#define MAXHANDLER   3

typedef struct {
    uint8_t dummy[4];
    uint8_t type;
    uint8_t dummy2[3];
    uint32_t startSector;
    uint32_t numberOfSectors;
} primary_partition_t;

typedef struct {
    uint8_t  fileName[8];		//0x00 .. 0x07
    uint8_t  fileExt[3];		//0x08 .. 0x0A
    uint8_t  fileAttrs;			//0x0B
    uint8_t  unused1;			//0x0C
    uint8_t  unused2;			//0x0D
    uint16_t fileCreateTime;		//0x0E .. 0x0F
    uint16_t fileCreateDate;		//0x10 .. 0x11
    uint16_t unused3;			//0x12 .. 0x13
    uint16_t unused4;			//0x14 .. 0x15
    uint16_t fileModTime;		//0x16 .. 0x17
    uint16_t fileModDate;		//0x18 .. 0x19
    uint16_t startCluster;		//0x1A .. 0x1B
    uint32_t fileSize;			//0x1C .. 0x1F
} direntry_t;

#define ATTRS_RO    0x01
#define ATTRS_HID   0x02
#define ATTRS_SYS   0x04
#define ATTRS_VOL   0x08
#define ATTRS_DIR   0x10
#define ATTRS_ARC   0x20
#define ATTRS_DEV   0x40

typedef struct {
    uint16_t currentCluster;		//0x00 .. 0x01
    uint32_t currentLogicalSector;	//0x02 .. 0x05
    uint32_t streamSize;		//0x06 .. 0x09
    uint32_t streamPtr;			//0x0A .. 0x0D
    uint16_t currentByteInSector;	//0x0E .. 0x0F
    uint8_t  currentSectorInCluster;	//0x10
    uint8_t  inUse;			//0x11
    uint8_t  eof;			//0x12
    uint8_t  unused1;                   //0x13
    uint8_t  bufsector[MAXSECSIZE];	//0x14 .. 0x213
} handler_t;

typedef struct {
    uint32_t  partitionLogicalSector;	//0x00 .. 0x03
    uint32_t  totalLogicalSectors;	//0x04 .. 0x07
    uint16_t  logicalSectorsPerFat;	//0x08 .. 0x09
    uint16_t  unused0;                  //0x0A .. 0x0B
    uint32_t  ssaIndex;			//0x0C .. 0x0F
    uint16_t  bytesPerSector;		//0x10 .. 0x11
    uint16_t  reservedSectors;		//0x12 .. 0x13
    uint16_t  rootDirSectors;		//0x14 .. 0x15
    uint16_t  rootDirEntries;		//0x16 .. 0x17
    uint16_t  fsInfoLogicalSector;	//0x18 .. 0x19
    uint16_t  logicalSectorBootAreaCopy;//0x1A .. 0x1B
    uint8_t   sectorsPerCluster;	//0x1C
    uint8_t   numberOfFATs;		//0x1D
    uint16_t fi_startCluster;		//0x1E .. 0x1F
    uint32_t fi_fileSize;		//0x20 .. 0x23
    uint16_t fi_fileCreateTime;		//0x24 .. 0x25
    uint16_t fi_fileCreateDate;		//0x26 .. 0x27
    uint16_t fi_fileModTime;		//0x28 .. 0x29
    uint16_t fi_fileModDate;		//0x2A .. 0x2B
    uint32_t rootDirStartSector;	//0x2C .. 0x2F
    uint8_t  fi_fileAttrs;		//0x30
    uint8_t  unused1;			//0x31
    uint8_t bufdir[DIRENTRYSIZE];	//0x32 .. 0x51
    handler_t handler[MAXHANDLER];	//0x52 .. 0x265
					//0x266 .. 0x479
					//0x47A .. 0x68C
} fatfs_t;

#endif /* FAT_H_ */
