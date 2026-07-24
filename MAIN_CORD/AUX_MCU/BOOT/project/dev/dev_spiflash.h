
#ifndef __DEV_SPIFLASH_H_
#define __DEV_SPIFLASH_H_

#include "bsp.h"


/* spi flash control cmd */
#define  FLASH_LOW_READ                 0x03
#define  FLASH_LOW_READ_4BYTE           0x13

#define  FLASH_HIGH_READ                0x0B
#define  FLASH_HIGH_READ_4BYTE          0x0C

#define  FLASH_SECTOR_ERASE             0x20
#define  FLASH_SECTOR_ERASE_4BYTE       0x21

#define  FLASH_BLOCK_ERASE_32K          0x52
#define  FLASH_CHIP_ERASE               0xC7

#define  FLASH_BLOCK_ERASE_64K          0xD8
#define  FLASH_BLOCK_ERASE_64K_4BYTE    0xDC

#define  FLASH_PAGE_PROGRAM             0x02
#define  FLASH_PAGE_PROGRAM_4BYTE       0x12 

#define  FLASH_READ_STATUS_REG          0x05
#define  FLASH_WRITE_STATUS_REG         0x01
#define  FLASH_WRITE_STATUS_REG2        0x31
#define  FLASH_WRITE_ENABLE             0x06
#define  FLASH_WRITE_DISABLE            0x04
#define  FLASH_POWER_DOWN               0xB9
#define  FLASH_RELEASE_PD               0xAB
#define  FLASH_JEDEC_ID                 0x9F
#define  DUMMY_BYTE                     0xFF

#define  FLASH_PROTECT                  0xFC
#define  FLASH_UNPROTECT                0x00


typedef struct _dev_spiflash_stru_t
{
    uint8_t             arr_id[4];
    uint16_t            n_capacity_MB;  /*Õ×±ÈÌØ*/   
}DevSpiFlash_t;




void SpiFlashReadData(uint32_t start_addr, uint8_t *p, uint32_t len);
void SpiFlashFastReadData(uint32_t start_addr, uint8_t *p, uint32_t len);
void SpiFlashPageProgram(uint32_t start_addr, uint8_t *p, uint32_t len);
void SpiFlashWriteBytesWithoutErase(uint32_t start_addr, uint8_t *p_data, uint16_t length);
void SpiFlashWriteBytesWithoutErasenoAligned(uint32_t start_addr, uint8_t *p_data, uint16_t length);
void SpiFlashWriteBytesWithErase(uint32_t start_addr, uint8_t *p_data, uint16_t length);

void SpiFlashEraseSector(uint32_t start_addr);
void SpiFlashAutoEraseSectors(uint32_t start_addr, uint16_t len);
void SpiFlashEraseBlock32K(uint32_t start_addr);
void SpiFlashEraseBlock64K(uint32_t start_addr);
void SpiFlashEraseChip(void);
void SpiFlashReadJEDECID(DevSpiFlash_t *p_dev);

void DevSpiFlashInit();
#endif










