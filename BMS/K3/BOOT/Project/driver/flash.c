/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        adc.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2022-10-31
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2022-10-31    <td>V1.0.0    <td>           <td>InitialVersion 当前版本仅支持独立模式、规则通道、单次软件触发采样转换

  * </table>
  ******************************************************************************
  */


#include "flash.h"

#define  FLASH_PAGE_SIZE            (256)


void FlashWrite(uint16_t len, uint8_t  *data, uint32_t addr_start)
{
    uint16_t i;
    uint32_t temp;

    fmc_state_enum  fmc_state = FMC_READY;

    fmc_unlock();

    fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);

    for(i = 0; i < len / 4; i++)
    {
        fmc_state = fmc_word_program(addr_start, *(uint32_t*)data);

        if(fmc_state != FMC_READY)
        {
            return;
        }

        fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR);

        data += 4;
        addr_start += 4;
    }
    if(len % 4)
    {
        temp = *(uint32_t*)data | 0xff000000;
        fmc_state = fmc_word_program(addr_start, temp);
    }

    fmc_lock();
}

void FlashRead(uint16_t len, uint8_t *outdata, uint32_t addr_start)
{
    uint32_t addr;
    uint16_t i;

    addr = addr_start;

    for(i = 0; i < len; i++)
    {
        *outdata = *(uint8_t*) addr;
        addr = addr + 1;
        outdata++;
    }
}

void FlashErase(uint32_t start, uint32_t end)
{
    uint32_t EraseCounter;
    fmc_state_enum fmc_state = FMC_READY;

    /* unlock the flash program/erase controller */
    fmc_unlock();
    /* clear all pending flags */
    fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);

    /* erase the flash pages */
    while(start < end)
    {
        EraseCounter = start / FLASH_PAGE_SIZE;
        fmc_state = fmc_page_erase(EraseCounter * FLASH_PAGE_SIZE);
        if(fmc_state != FMC_READY)
        {
            return;
        }
        fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR);

        start += FLASH_PAGE_SIZE;
    }
    /* lock the main FMC after the erase operation */
    fmc_lock();
}