
#include "flash.h"

static bool _gFlashLocked = true; /* 默认上锁 */
static bool _IsProtectedLocked(uint32_t addr, uint32_t len);

/*len表示字节数，不是32bit数量*/
static void _InternalFlashAutoErasePage(uint32_t addr, uint32_t len)
{
    if(_IsProtectedLocked(addr, len))
    {
        return;
    }
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
    
    FLASH_EraseOnePage(FMC_START_ADDR + (FMC_PAGE_SIZE * page));  //原底层
//	FLASH_EraseOnePage(FMC_START_ADDR + addr);
    
    FLASH_Lock();
}


/*len表示字节数，不是32bit数量*/
void InternalFlashWrite(uint32_t addr, uint32_t *p, uint32_t len)
{
    if(_IsProtectedLocked(addr, len))
    {
        return;
    }
    uint32_t size = (len + 3) / 4;
    FLASH_Unlock();
    _InternalFlashAutoErasePage(addr, len);
//	InternalFlashErasePage(addr);
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
    if(_IsProtectedLocked(addr, len))
    {
        return;
    }
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


/**
* @brief: _is_protected_locked
* @param: addr
* @param: len
* @return: 返回 true 表示区间有交集且当前处于锁状态 -> 拒绝操作
* @note:
*
*/
static bool _IsProtectedLocked(uint32_t addr, uint32_t len)
{
    if(_gFlashLocked == false)               /* 已解锁，直接放行 */
    {
        return false;
    }

    uint32_t protected_start = 0x08040000;
    uint32_t protected_end   = 0x0807FFFF; /* 0x0804 0000 ~ 0x0807 FFFF */

    uint32_t op_end = addr + len - 1;

    /* 无交集 */
    if(op_end < protected_start || addr > protected_end)
    {
        return false;
    }

    return true; /* 有交集且已锁 */
}

void InternalFlashLock(void)
{
    _gFlashLocked = true;
}

void InternalFlashUnlock(void)
{
    _gFlashLocked = false;
}

bool InternalFlashIsLocked(void)
{
    return _gFlashLocked;
}
