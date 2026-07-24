/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        flash.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-06-19
  * @brief		 GD32F4xx
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-06-19    <td>V1.0.0    <td>           <td>InitialVersion 支持自动擦除写入(多字节写，无需关心擦除操作)，
																		  手动擦除和写入，Word、HalfWord、Byte写入模式，Word、Byte读取模式
  * </table>
  ******************************************************************************
  */
#ifndef __FLASH_H_
#define __FLASH_H_

#include "drv_opt.h"

#define FMC_START_ADDR          ((uint32_t)0x08000000U)


typedef struct _fmc_sector_info_stru_t
{
    uint32_t sector_name;                                         /*!< the name of the sector */
    uint32_t sector_num;                                          /*!< the number of the sector */
    uint32_t sector_size;                                         /*!< the size of the sector */
    uint32_t sector_start_addr;                                   /*!< the start address of the sector */
    uint32_t sector_end_addr;                                     /*!< the end address of the sector */
}FMCSectorInfo_t;


/* sector size */
#define FMC_SIZE_16KB              ((uint32_t)0x00004000U)        /*!< size of 16KB*/
#define FMC_SIZE_64KB              ((uint32_t)0x00010000U)        /*!< size of 64KB*/
#define FMC_SIZE_128KB             ((uint32_t)0x00020000U)        /*!< size of 128KB*/
#define FMC_SIZE_256KB             ((uint32_t)0x00040000U)        /*!< size of 256KB*/

/* FMC BANK address */
#define FMC_START_ADDRESS          FLASH_BASE                               /*!< FMC start address */
#define FMC_BANK0_START_ADDRESS    FMC_START_ADDRESS                        /*!< FMC BANK0 start address */
#define FMC_BANK1_START_ADDRESS    ((uint32_t)0x08100000U)                  /*!< FMC BANK1 start address */
#define FMC_SIZE                   (*(uint16_t *)0x1FFF7A22U)               /*!< FMC SIZE */
#define FMC_END_ADDRESS            (FLASH_BASE + (FMC_SIZE * 1024) - 1)     /*!< FMC end address */
#define FMC_MAX_END_ADDRESS        ((uint32_t)0x08300000U)                  /*!< FMC maximum end address */

/* FMC error message */
#define FMC_WRONG_SECTOR_NAME      ((uint32_t)0xFFFFFFFFU)        /*!< wrong sector name*/
#define FMC_WRONG_SECTOR_NUM       ((uint32_t)0xFFFFFFFFU)        /*!< wrong sector number*/
#define FMC_INVALID_SIZE           ((uint32_t)0xFFFFFFFFU)        /*!< invalid sector size*/
#define FMC_INVALID_ADDR           ((uint32_t)0xFFFFFFFFU)        /*!< invalid sector address*/


void InternalFlashWordWrite(uint32_t addr, uint32_t *p, uint32_t len);
void InternalFlashWordWriteWithoutErase(uint32_t addr, uint32_t *p, uint32_t len);
void InternalFlashHalfWordWrite(uint32_t addr, uint16_t *p, uint32_t len);
void InternalFlashHalfWordWriteWithoutErase(uint32_t addr, uint16_t *p, uint32_t len);
void InternalFlashByteWrite(uint32_t addr, uint8_t *p, uint32_t len);
void InternalFlashByteWriteWithoutErase(uint32_t addr, uint8_t *p, uint32_t len);

void InternalFlashByteRead(uint32_t addr, uint8_t *p, uint32_t len);
void InternalFlashWordRead(uint32_t addr, uint32_t *p, uint32_t len);
void InternalFlashErasePage(uint32_t addr);

#endif







