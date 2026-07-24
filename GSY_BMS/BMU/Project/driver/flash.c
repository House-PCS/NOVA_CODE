
#include "flash.h"



/*len表示字节数，不是32bit数量*/
static void _InternalFlashAutoErasePage(uint32_t addr, uint32_t len)
{
    uint16_t start_page, end_page;
    uint32_t start_addr, end_addr;
    
    start_addr=addr-FMC_START_ADDR;
    end_addr=start_addr+ len - 1;
    start_page = start_addr / FMC_PAGE_SIZE;
    
//    if(start_addr % FMC_PAGE_SIZE != 0)
//    {
//        start_page += 1;
//    }
    
    end_page = end_addr / FMC_PAGE_SIZE;
    
    
    /* clear all pending flags */
    fmc_flag_clear(FMC_FLAG_BANK0_END | FMC_FLAG_BANK0_WPERR | FMC_FLAG_BANK0_PGERR );
    
    for(uint16_t i = start_page; i <= end_page; i++)
    {
        fmc_page_erase(FMC_START_ADDR + (FMC_PAGE_SIZE * i));
        fmc_flag_clear(FMC_FLAG_BANK0_END | FMC_FLAG_BANK0_WPERR | FMC_FLAG_BANK0_PGERR);
    }
    
    
}

void InternalFlashErasePage(uint32_t addr)
{
    uint32_t page = (addr-FMC_START_ADDR) / FMC_PAGE_SIZE;
    fmc_unlock();
    /* clear all pending flags */
    fmc_flag_clear(FMC_FLAG_BANK0_END | FMC_FLAG_BANK0_WPERR | FMC_FLAG_BANK0_PGERR);
    fmc_page_erase(FMC_START_ADDR + (FMC_PAGE_SIZE * page));
    
    fmc_lock();
}


/*len表示字节数，不是32bit数量*/
void InternalFlashWrite(uint32_t addr, uint32_t *p, uint32_t len)
{
    uint32_t size = (len + 3) / 4;
    fmc_unlock();
    _InternalFlashAutoErasePage(addr, len);

    while(size > 0)
    {
        fmc_word_program( addr, p[0]);
        p++;
        addr += 4;
        size -= 1;
    }
    fmc_lock();
}
void InternalFlashWriteWithoutErase(uint32_t addr, uint32_t *p, uint32_t len)
{
    uint32_t size = (len + 3) / 4;
    fmc_unlock();
    fmc_flag_clear(FMC_FLAG_BANK0_END | FMC_FLAG_BANK0_WPERR | FMC_FLAG_BANK0_PGERR);
    
    while(size > 0)
    {
        fmc_word_program(addr, p[0]);
        p++;
        addr += 4;
        size -= 1;
    }
    fmc_lock();
}

void InternalFlashRead(uint32_t addr, uint8_t *p, uint32_t len)
{
    for(uint32_t i = 0; i < len; i++)
    {
        p[i] = *(uint8_t *)( addr);
        addr++;
    }
}

/*len表示字节数，不是32bit数量*/
void InternalFlashReadWord(uint32_t addr, uint32_t *p, uint32_t len)
{
    for(uint32_t i = 0; i < len / 4; i++)
    {
        p[i] = *(uint32_t *)( addr);
        addr += 4;
    }
}



