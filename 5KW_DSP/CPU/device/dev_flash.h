/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_flash.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-2-4
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-2-4    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */


#ifndef __DEV_FLASH_H__
#define __DEV_FLASH_H__

#include "spi.h"
#include "string.h"
#ifdef __cplusplus
extern "C" {
#endif
#define CS_SPI_FLASH_GPIO    61
/* 片选口线置低选中  */
#define SF_CS_LOW()		GPIO_WritePin(CS_SPI_FLASH_GPIO, 0);
/* 片选口线置高不选中 */
#define SF_CS_HIGH()	GPIO_WritePin(CS_SPI_FLASH_GPIO, 1);
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

#define  FLASH_PROJECT                  0xBC    //0xFC
#define  FLASH_UNPROJECT                0x00    //0x40

#define  FLASH_PAGE_SIZE            (256)
#define  FLASH_ERASE_SECTOR_SIZE    (0x1000)
#define  FLASH_ERASE_BLOCK_SIZE     (0x10000)
#define  FLASH_TEST_ADDR            (0x3FF000)


typedef struct _spi_flash_handle_stru_t
{
    uint16_t             arr_id[4];
    uint16_t            n_capacity_MB;  /*兆比特*/
    uint8_t             c_protect_state;

    uint32_t            n_start_addr;
    uint16_t            n_length;
    uint8_t             *pdata;

}SpiFlash_HandleType;

void SpiFlashInit(void);
int32_t SpiFlashReadBytes(uint32_t start_addr, uint16_t *idata, uint16_t length);
int32_t SpiFlashEraseChip(void);
int32_t SpiFflashWriteBytesWithoutErase(uint32_t start_addr, uint16_t *idata, uint16_t length);
int32_t SpiFlashWriteBytesWithErase(uint32_t start_addr, uint16_t *idata, uint16_t length);
int32_t SpiFlashAutoEraseSector(uint32_t start_addr, uint16_t length);
bool FlashTest(void);
void SpiFlashLock(void);
void SpiFlashUnlock(void);
bool SpiFlashIsLocked(void);

#ifdef __cplusplus
}
#endif                                  /* extern "C" */

#endif

//===========================================================================
// End of file.
//===========================================================================
