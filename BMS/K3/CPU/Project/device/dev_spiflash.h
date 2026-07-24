/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT Nova
  * @file    dev_spi_flash.h
  * @author  Xi'an NovaStar Tech Co.,LTD.()
  * @version V1.0.0
  * @date    2021.08.04
  * @brief   CBB:this file provides the functions of dev_spi_flash.
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date        <th>Version  <th>Author      <th>Description
  * <tr><td>2021/08/04  <td>1.0.0    <td>Changshuai  <td>CBB Version
  * </table>
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DEV_SPIFLASH_H_
#define __DEV_SPIFLASH_H_

/* Inculdes ------------------------------------------------------------------*/
#include "bsp.h"
#include "systick.h"
#include "fault_analysis.h"

#ifdef __cplusplus
extern "C" {
#endif
/* Exported macro -----------------------------------------------------------*/
#define  NULL                    0

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
/* 状态寄存器1数据 */      

/* Exported types ------------------------------------------------------------*/
typedef enum
{
    EN_FLASH_UNPROTECT = 0x00,
    EN_FLASH_PROTECT,
} en_flash_protect_t;



typedef enum
{
#if USE_SPI_FLASH0
    SPI_FLASH0_PORT,
#endif
#if USE_SPI_FLASH1
    SPI_FLASH1_PORT,
#endif
#if USE_SPI_FLASH2
    SPI_FLASH2_PORT,
#endif
#if USE_SPI_FLASH3  
    SPI_FLASH3_PORT,
#endif
#if USE_SPI_FLASH4
    SPI_FLASH4_PORT,
#endif
#if USE_SPI_FLASH5
    SPI_FLASH5_PORT,
#endif
#if USE_SPI_FLASH6
    SPI_FLASH6_PORT,
#endif
#if USE_SPI_FLASH7  
    SPI_FLASH7_PORT,
#endif
    SPI_FLASH_NUM
}EN_SPI_FLASHx_PORT;

typedef struct _spi_flash_handle_stru_t
{
    EN_SPI_FLASHx_PORT  spi_flashx;
    
    Gpio_HandleType     *pCs;
    Gpio_HandleType     *pWp;
    Gpio_HandleType     *pRst;
    Spi_HandleType      *pSpi;
    uint8_t             arr_id[4];
    uint16_t            n_capacity_MB;  /*兆比特*/    
    uint8_t             c_protect_state;
    
    uint32_t            n_start_addr;
    uint16_t            n_length;
    uint8_t             *pdata;
    
}SpiFlash_HandleType;
/* Exported constants --------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
void SpiFlashInit                       (void);
int32_t spi_flash_read_bytes(uint32_t start_addr, uint8_t *idata, uint16_t length);
int32_t spi_flash_auto_erase_sector(uint32_t start_addr, uint16_t length);
int32_t spi_flash_write_bytes_without_erase(uint32_t start_addr, uint8_t *idata, uint16_t length);
int32_t spi_flash_write_bytes_with_erase(uint32_t start_addr, uint8_t *idata, uint16_t length);
int32_t spi_flash_erase_block_64k(uint32_t start_addr);
#ifdef __cplusplus
}
#endif
#endif /* __SPI_FLASH */

