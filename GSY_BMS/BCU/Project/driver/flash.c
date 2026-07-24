/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        flash.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-06-19
  * @brief	     GD32F4xx
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
#include "flash.h"


/*!
    \brief      get the sector number, size and range of the given address
    \param[in]  address: The flash address
    \param[out] none
    \retval     FMCSectorInfo_t: The information of a sector
*/
static FMCSectorInfo_t _FMCSectorInfoGet(uint32_t addr)
{
    FMCSectorInfo_t sector_info;
    uint32_t temp = 0x00000000U;
    addr += FMC_START_ADDR;
    
    if((FMC_START_ADDRESS <= addr)&&(FMC_END_ADDRESS >= addr)) 
    {
        if((FMC_BANK1_START_ADDRESS > addr)) 
        {
            /* bank0 area */
            temp = (addr - FMC_BANK0_START_ADDRESS) / FMC_SIZE_16KB;
            if(4U > temp) 
            {
                sector_info.sector_name = (uint32_t)temp;
                sector_info.sector_num = CTL_SN(temp);
                sector_info.sector_size = FMC_SIZE_16KB;
                sector_info.sector_start_addr = FMC_BANK0_START_ADDRESS + (FMC_SIZE_16KB * temp);
                sector_info.sector_end_addr = sector_info.sector_start_addr + FMC_SIZE_16KB - 1;
            } 
            else if(8U > temp) 
            {
                sector_info.sector_name = 0x00000004U;
                sector_info.sector_num = CTL_SN(4);
                sector_info.sector_size = FMC_SIZE_64KB;
                sector_info.sector_start_addr = 0x08010000U;
                sector_info.sector_end_addr = 0x0801FFFFU;
            } 
            else 
            {
                temp = (addr - FMC_BANK0_START_ADDRESS) / FMC_SIZE_128KB;
                sector_info.sector_name = (uint32_t)(temp + 4);
                sector_info.sector_num = CTL_SN(temp + 4);
                sector_info.sector_size = FMC_SIZE_128KB;
                sector_info.sector_start_addr = FMC_BANK0_START_ADDRESS + (FMC_SIZE_128KB * temp);
                sector_info.sector_end_addr = sector_info.sector_start_addr + FMC_SIZE_128KB - 1;
            }
        }
        else
        {
            /* bank1 area */
            temp = (addr - FMC_BANK1_START_ADDRESS) / FMC_SIZE_16KB;
            if(4U > temp) 
            {
                sector_info.sector_name = (uint32_t)(temp + 12);
                sector_info.sector_num = CTL_SN(temp + 16);
                sector_info.sector_size = FMC_SIZE_16KB;
                sector_info.sector_start_addr = FMC_BANK0_START_ADDRESS + (FMC_SIZE_16KB * temp);
                sector_info.sector_end_addr = sector_info.sector_start_addr + FMC_SIZE_16KB - 1;
            } 
            else if(8U > temp) 
            {
                sector_info.sector_name = 0x00000010;
                sector_info.sector_num = CTL_SN(20);
                sector_info.sector_size = FMC_SIZE_64KB;
                sector_info.sector_start_addr = 0x08110000U;
                sector_info.sector_end_addr = 0x0811FFFFU;
            } 
            else if(64U > temp)
            {
                temp = (addr - FMC_BANK1_START_ADDRESS) / FMC_SIZE_128KB;
                sector_info.sector_name = (uint32_t)(temp + 16);
                sector_info.sector_num = CTL_SN(temp + 20);
                sector_info.sector_size = FMC_SIZE_128KB;
                sector_info.sector_start_addr = FMC_BANK1_START_ADDRESS + (FMC_SIZE_128KB * temp);
                sector_info.sector_end_addr = sector_info.sector_start_addr + FMC_SIZE_128KB - 1;
            } 
            else 
            {
                temp = (addr - FMC_BANK1_START_ADDRESS) / FMC_SIZE_256KB;
                sector_info.sector_name = (uint32_t)(temp + 20);
                sector_info.sector_num = CTL_SN(temp + 8);
                sector_info.sector_size = FMC_SIZE_256KB;
                sector_info.sector_start_addr = FMC_BANK1_START_ADDRESS + (FMC_SIZE_256KB * temp);
                sector_info.sector_end_addr = sector_info.sector_start_addr + FMC_SIZE_256KB - 1;
            }
        }
    }
    else 
    {
        /* invalid address */
        sector_info.sector_name = FMC_WRONG_SECTOR_NAME;
        sector_info.sector_num = FMC_WRONG_SECTOR_NUM;
        sector_info.sector_size = FMC_INVALID_SIZE;
        sector_info.sector_start_addr = FMC_INVALID_ADDR;
        sector_info.sector_end_addr = FMC_INVALID_ADDR;
    }
    return sector_info;
}



/*!
    \brief      get the sector number by a given sector name
    \param[in]  address: a given sector name
    \param[out] none
    \retval     uint32_t: sector number
*/
static uint32_t _SectorNameToNumber(uint32_t sector_name)
{
    if(11 >= sector_name)
    {
        return CTL_SN(sector_name);
    }
    else if(23 >= sector_name)
    {
        return CTL_SN(sector_name + 4);
    }
    else if(27 >= sector_name)
    {
        return CTL_SN(sector_name - 12);
    }
    else
    {
        while(1);
    }
}

/*len表示字节数，不是32bit数量*/
static void _InternalFlashAutoErasePage(uint32_t addr, uint32_t len)
{
    FMCSectorInfo_t sector_info;
    uint32_t i;
    uint32_t start_sector, end_sector;
    uint32_t last_addr = addr + len - 1;
    
    sector_info = _FMCSectorInfoGet(addr);
    start_sector = sector_info.sector_name;
    
    if(addr % sector_info.sector_size != 0)//不是从sector开始的地址，当前sector不擦除
    {
        start_sector++;
    }
    
    sector_info = _FMCSectorInfoGet(last_addr);
    end_sector = sector_info.sector_name;
    
    /* clear pending flags */
    fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_OPERR | FMC_FLAG_WPERR | FMC_FLAG_PGMERR | FMC_FLAG_PGSERR);
    for(i = start_sector; i <= end_sector; i++)
    {
        fmc_sector_erase(_SectorNameToNumber(i));
        fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_OPERR | FMC_FLAG_WPERR | FMC_FLAG_PGMERR | FMC_FLAG_PGSERR);
    }
}


void InternalFlashErasePage(uint32_t addr)
{
    FMCSectorInfo_t sector_info;
    
    sector_info = _FMCSectorInfoGet(addr);
    fmc_unlock();
    /* clear all pending flags */
    fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_OPERR | FMC_FLAG_WPERR | FMC_FLAG_PGMERR | FMC_FLAG_PGSERR);
    fmc_sector_erase(sector_info.sector_num);
    
    fmc_lock();
}


/*len表示字节数，不是32bit数量*/
void InternalFlashWordWrite(uint32_t addr, uint32_t *p, uint32_t len)
{
    uint32_t size = (len + 3) / 4;
    fmc_unlock();
    _InternalFlashAutoErasePage(addr, len);

    while(size > 0)
    {
        fmc_word_program(FMC_START_ADDR + addr, p[0]);
        p++;
        addr += 4;
        size -= 1;
    }
    fmc_lock();
}

void InternalFlashWordWriteWithoutErase(uint32_t addr, uint32_t *p, uint32_t len)
{
    uint32_t size = (len + 3) / 4;
    fmc_unlock();
    fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_OPERR | FMC_FLAG_WPERR | FMC_FLAG_PGMERR | FMC_FLAG_PGSERR);
    
    while(size > 0)
    {
        fmc_word_program(FMC_START_ADDR + addr, p[0]);
        p++;
        addr += 4;
        size -= 1;
    }
    fmc_lock();
}

/*len表示字节数，不是16bit数量*/
void InternalFlashHalfWordWrite(uint32_t addr, uint16_t *p, uint32_t len)
{
    uint32_t size = (len + 1) / 2;
    fmc_unlock();
    _InternalFlashAutoErasePage(addr, len);
    
    while(size > 0)
    {
        fmc_halfword_program(FMC_START_ADDR + addr, p[0]);
        p++;
        addr += 2;
        size -= 1;
    }
    fmc_lock();
}

void InternalFlashHalfWordWriteWithoutErase(uint32_t addr, uint16_t *p, uint32_t len)
{
    uint32_t size = (len + 1) / 2;
    fmc_unlock();
    fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_OPERR | FMC_FLAG_WPERR | FMC_FLAG_PGMERR | FMC_FLAG_PGSERR);
    
    while(size > 0)
    {
        fmc_halfword_program(FMC_START_ADDR + addr, p[0]);
        p++;
        addr += 2;
        size -= 1;
    }
    fmc_lock();
}



/*len表示字节数，不是16bit数量*/
void InternalFlashByteWrite(uint32_t addr, uint8_t *p, uint32_t len)
{
    uint32_t size = len;
    fmc_unlock();
    _InternalFlashAutoErasePage(addr, len);
    
    while(size > 0)
    {
        fmc_byte_program(FMC_START_ADDR + addr, p[0]);
        p++;
        addr += 1;
        size -= 1;
    }
    fmc_lock();
}

void InternalFlashByteWriteWithoutErase(uint32_t addr, uint8_t *p, uint32_t len)
{
    uint32_t size = len;
    fmc_unlock();
    fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_OPERR | FMC_FLAG_WPERR | FMC_FLAG_PGMERR | FMC_FLAG_PGSERR);
    
    while(size > 0)
    {
        fmc_byte_program(FMC_START_ADDR + addr, p[0]);
        p++;
        addr += 1;
        size -= 1;
    }
    fmc_lock();
}





void InternalFlashByteRead(uint32_t addr, uint8_t *p, uint32_t len)
{
    for(uint32_t i = 0; i < len; i++)
    {
        p[i] = *(uint8_t *)(FMC_START_ADDR + addr);
        addr++;
    }
}

/*len表示字节数，不是32bit数量*/
void InternalFlashWordRead(uint32_t addr, uint32_t *p, uint32_t len)
{
    for(uint32_t i = 0; i < len / 4; i++)
    {
        p[i] = *(uint32_t *)(FMC_START_ADDR + addr);
        addr += 4;
    }
}



