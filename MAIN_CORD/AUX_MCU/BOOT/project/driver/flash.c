
#include "flash.h"



/*len表示字节数，不是32bit数量*/
static void _InternalFlashAutoErasePage(uint32_t addr, uint32_t len)
{
    uint32_t start_page, end_page;
    uint32_t last_addr = addr + len - 1;
    
    start_page = addr / FMC_PAGE_SIZE;
    
    if(addr % FMC_PAGE_SIZE != 0)
    {
        start_page += 1;
    }
    
    end_page = last_addr / FMC_PAGE_SIZE;
    
    for(uint16_t i = start_page; i <= end_page; i++)
    {
        FLASH_EraseOnePage(FMC_START_ADDR + (FMC_PAGE_SIZE * i));
    }
    
    
}

void InternalFlashErasePage(uint32_t addr)
{
    uint32_t page = addr / FMC_PAGE_SIZE;
    FLASH_Unlock();
    
    FLASH_EraseOnePage(FMC_START_ADDR + (FMC_PAGE_SIZE * page));
    
    FLASH_Lock();
}


/*len表示字节数，不是32bit数量*/
void InternalFlashWrite(uint32_t addr, uint32_t *p, uint32_t len)
{
    uint32_t size = (len + 3) / 4;
    FLASH_Unlock();
    _InternalFlashAutoErasePage(addr, len);

    while(size > 0)
    {
        FLASH_ProgramWord(FMC_START_ADDR + addr, p[0]);
        p++;
        addr += 4;
        size -= 1;
    }
    FLASH_Lock();
}

void InternalFlashWriteWithoutErase(uint32_t addr, uint32_t *p, uint32_t len)
{
    uint32_t size = (len + 3) / 4;
    FLASH_Unlock();
    
    while(size > 0)
    {
        FLASH_ProgramWord(FMC_START_ADDR + addr, p[0]);
        p++;
        addr += 4;
        size -= 1;
    }
    FLASH_Lock();
}

void InternalFlashRead(uint32_t addr, uint8_t *p, uint32_t len)
{
    for(uint32_t i = 0; i < len; i++)
    {
        p[i] = *(uint8_t *)(FMC_START_ADDR + addr);
        addr++;
    }
}

/*len表示字节数，不是32bit数量*/
void InternalFlashReadWord(uint32_t addr, uint32_t *p, uint32_t len)
{
    for(uint32_t i = 0; i < len / 4; i++)
    {
        p[i] = *(uint32_t *)(FMC_START_ADDR + addr);
        addr += 4;
    }
}



