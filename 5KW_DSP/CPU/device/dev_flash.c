/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_flash.c
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
  * </table>    32M bit flash 0~ 0x3F FFFF
  ******************************************************************************
  */


//
// Included Files
//
#include "dev_flash.h"

static uint16_t _garrDataBuf[256];
static bool _gFlashLocked = true; /* 默认上锁 */

SpiFlash_HandleType SpiFlash;
static bool _SpiFlashGetInfo(SpiFlash_HandleType *pSpiFlash);
static bool _IsProtectedLocked(uint32_t addr, uint32_t len);

/**
* @brief: SpiFlashInit
* @param: void
* @note:
*
*/
void SpiFlashInit(void)
{
    GPIO_SetupPinMux(CS_SPI_FLASH_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(CS_SPI_FLASH_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);

    SpiFifoInit();
    _SpiFlashGetInfo(&SpiFlash);
}

/**
* @brief: _SpiFlashGetInfo
* @param: pSpiFlash
* @return:
* @note:
*
*/
static bool _SpiFlashGetInfo(SpiFlash_HandleType *pSpiFlash)
{
    int32_t n_ret = 0;
    uint16_t c_txdata = 0;

    SF_CS_LOW();

    c_txdata = FLASH_JEDEC_ID;
    SpiWriteData(1, &c_txdata);
    SpiReadData(3, SpiFlash.arr_id);
    SF_CS_HIGH();

    if(SpiFlash.arr_id[0])
    {
        if(0x14 == SpiFlash.arr_id[2])
        {
            SpiFlash.n_capacity_MB = 1;
        }
        else if(0x15 == SpiFlash.arr_id[2])
        {
            SpiFlash.n_capacity_MB = 2;
        }
        else if(0x16 == SpiFlash.arr_id[2])
        {
            SpiFlash.n_capacity_MB = 4;
        }
        else if(0x17 == SpiFlash.arr_id[2])
        {
            SpiFlash.n_capacity_MB = 8;
        }
        else if(0x18 == SpiFlash.arr_id[2])
        {
            SpiFlash.n_capacity_MB = 16;
        }
        else if(0x19 == SpiFlash.arr_id[2])
        {
            SpiFlash.n_capacity_MB = 32;
        }
        else if(0x20 == SpiFlash.arr_id[2] || 0x1A == SpiFlash.arr_id[2] || 0x3A == SpiFlash.arr_id[2])
        {
            SpiFlash.n_capacity_MB = 64;
        }
        else
        {
            SpiFlash.n_capacity_MB = 8;
        }
    }
    else
    {
        SpiFlash.n_capacity_MB = 0;
    }
    return n_ret;
}

/**
* @brief: _SpiFlashWrite32bitAddr
* @param: n_addr
* @return:
* @note:
*
*/
static int32_t _SpiFlashWrite32bitAddr (uint32_t n_addr)
{
    int32_t n_ret = 0;
    uint16_t c_txdata = 0;

    c_txdata = n_addr >> 24;
    SpiWriteData(1, &c_txdata);
    c_txdata = n_addr >> 16;
    SpiWriteData(1, &c_txdata);
    c_txdata = n_addr >> 8;
    SpiWriteData(1, &c_txdata);
    c_txdata = n_addr;
    SpiWriteData(1, &c_txdata);

    return n_ret;
}

/**
* @brief: _SpiFlashWrite24bitAddr
* @param: n_addr
* @return:
* @note:
*
*/
static int32_t _SpiFlashWrite24bitAddr (uint32_t n_addr)
{
    int32_t n_ret = 0;
    uint16_t c_txdata = 0;

    c_txdata = n_addr >> 16;
    SpiWriteData(1, &c_txdata);
    c_txdata = n_addr >> 8;
    SpiWriteData(1, &c_txdata);
    c_txdata = n_addr;
    SpiWriteData(1, &c_txdata);

    return n_ret;
}

/**
* @brief: SpiFlashReadBytes
* @param: start_addr
* @param: idata
* @param: length
* @return:
* @note:
*
*/
int32_t SpiFlashReadBytes(uint32_t start_addr, uint16_t *idata, uint16_t length)
{
    int32_t n_ret = 0;
    uint16_t c_txdata = 0;
    SpiFlash.n_start_addr = start_addr;

    SF_CS_LOW();
    if(SpiFlash.n_capacity_MB >= 32)
    {
        c_txdata = FLASH_LOW_READ_4BYTE;
        SpiWriteData(1, &c_txdata);
        _SpiFlashWrite32bitAddr(start_addr);
    }
    else
    {
        c_txdata = FLASH_LOW_READ;
        SpiWriteData(1, &c_txdata);
        _SpiFlashWrite24bitAddr(start_addr);
    }
    SpiReadData(length, idata);
    SF_CS_HIGH();

    return n_ret;
}

/**
* @brief: _SpiFlashWriteEnable
* @param: pSpiFlash
* @return:
* @note:
*
*/
static int32_t _SpiFlashWriteEnable(SpiFlash_HandleType *pSpiFlash)
{
    int32_t n_ret = 0;
    uint16_t c_txdata = 0;

    SF_CS_LOW();
    c_txdata = FLASH_WRITE_ENABLE;
    SpiWriteData(1, &c_txdata);
    SF_CS_HIGH();

    return n_ret;
}

/**
* @brief: _SpiFlashReadStatusReg
* @param: pSpiFlash
* @return:
* @note:
*
*/
static int32_t _SpiFlashReadStatusReg(SpiFlash_HandleType *pSpiFlash)
{
    int32_t n_ret = 0;
    uint32_t  time_out = 0xFFFF;
    uint16_t c_txdata = 0;
    uint16_t c_rxdata = 0;

    SF_CS_LOW();
    c_txdata = FLASH_READ_STATUS_REG;
    SpiWriteData(1, &c_txdata);
//    c_txdata = DUMMY_BYTE & 0x01;

    do
    {
        SpiReadData(1, &c_rxdata);
        time_out--;
    }
    while((c_rxdata & 0x01) && (time_out != 0));

    SF_CS_HIGH();

    return n_ret;
}

/**
* @brief: SpiFlashAutoEraseSector
* @param: start_addr
* @param: length
* @return:
* @note:
*
*/
int32_t SpiFlashAutoEraseSector(uint32_t start_addr, uint16_t length)
{
    int32_t n_ret = 0;
    int32_t n_remain = 0;
    uint16_t c_txdata = 0;
    
    if(_IsProtectedLocked(start_addr, length))
    {
        return -1;                    /* 保护拒绝 */
    }
            
    n_remain = start_addr % FLASH_ERASE_SECTOR_SIZE;

    if((n_remain == 0) ||\
            (n_remain + length > FLASH_ERASE_SECTOR_SIZE))
    {
        if(n_remain != 0)
        {
            start_addr += FLASH_ERASE_SECTOR_SIZE;
        }

        _SpiFlashWriteEnable(&SpiFlash);
        SF_CS_LOW();
        if(SpiFlash.n_capacity_MB >= 32)
        {
            c_txdata = FLASH_SECTOR_ERASE_4BYTE;
            SpiWriteData(1, &c_txdata);
            _SpiFlashWrite32bitAddr(start_addr);
        }
        else
        {
            c_txdata = FLASH_SECTOR_ERASE;
            SpiWriteData(1, &c_txdata);
            _SpiFlashWrite24bitAddr(start_addr);
        }
        SF_CS_HIGH();
        _SpiFlashReadStatusReg(&SpiFlash);
    }
    return n_ret;
}

/**
* @brief: SpiFlashEraseBlock32k
* @param: start_addr
* @return:
* @note:
*
*/
int32_t SpiFlashEraseBlock32k(uint32_t start_addr)
{
    int32_t n_ret = 0;
    uint16_t c_txdata = 0;

    _SpiFlashWriteEnable(&SpiFlash);
    SF_CS_LOW();
    c_txdata = FLASH_BLOCK_ERASE_32K;
    SpiWriteData(1, &c_txdata);
    _SpiFlashWrite24bitAddr(start_addr);
    SF_CS_HIGH();
    _SpiFlashReadStatusReg(&SpiFlash);

    return n_ret;
}

/**
* @brief: SpiFlashEraseBlock64k
* @param: start_addr
* @return:
* @note:
*
*/
int32_t SpiFlashEraseBlock64k(uint32_t start_addr)
{
    int32_t n_ret = 0;
    uint16_t c_txdata = 0;

    _SpiFlashWriteEnable(&SpiFlash);
    SF_CS_LOW();
    c_txdata = FLASH_BLOCK_ERASE_64K;
    SpiWriteData(1, &c_txdata);
    _SpiFlashWrite24bitAddr(start_addr);
    SF_CS_HIGH();
    _SpiFlashReadStatusReg(&SpiFlash);

    return n_ret;
}

/**
* @brief: SpiFlashEraseChip
* @param: void
* @return:
* @note:
*
*/
int32_t SpiFlashEraseChip(void)
{
    int32_t n_ret = 0;
    uint16_t c_txdata = 0;

    if(SpiFlashIsLocked())
    {
        return -1;
    }

    _SpiFlashWriteEnable(&SpiFlash);
    SF_CS_LOW();
    c_txdata = FLASH_CHIP_ERASE;
    SpiWriteData(1, &c_txdata);
    SF_CS_HIGH();
    _SpiFlashReadStatusReg(&SpiFlash);

    return n_ret;
}

/**
* @brief: SpiFlashPageProgram
* @param: start_addr
* @param: idata
* @param: length
* @return:
* @note:
*
*/
static int32_t SpiFlashPageProgram(uint32_t start_addr, uint16_t *idata, uint16_t length)
{
    int32_t n_ret = 0;
    uint16_t c_txdata = 0;

    if(_IsProtectedLocked(start_addr, length))
    {
        return -1;
    }
            
    _SpiFlashWriteEnable(&SpiFlash);
    SF_CS_LOW();
    if(SpiFlash.n_capacity_MB >= 32)
    {
        c_txdata = FLASH_PAGE_PROGRAM_4BYTE;
        SpiWriteData(1, &c_txdata);
        _SpiFlashWrite32bitAddr(start_addr);
    }
    else
    {
        c_txdata = FLASH_PAGE_PROGRAM;
        SpiWriteData(1, &c_txdata);
        _SpiFlashWrite24bitAddr(start_addr);
    }
    SpiWriteData(length, idata);
    SF_CS_HIGH();
    _SpiFlashReadStatusReg(&SpiFlash);

    return n_ret;
}

/**
* @brief: SpiFflashWriteBytesWithoutErase
* @param: start_addr
* @param: idata
* @param: length
* @return:
* @note:
*
*/
int32_t SpiFflashWriteBytesWithoutErase(uint32_t start_addr, uint16_t *idata, uint16_t length)
{
    uint16_t num_of_page = 0;
    uint16_t num_of_single = 0;
    int32_t n_ret = 0;

    num_of_page =  length / FLASH_PAGE_SIZE;
    num_of_single = length % FLASH_PAGE_SIZE;

    while(num_of_page--)
    {
        SpiFlashPageProgram(start_addr, idata, FLASH_PAGE_SIZE);
        start_addr +=  FLASH_PAGE_SIZE;
        idata += FLASH_PAGE_SIZE;
    }
    if(num_of_single != 0)
    {
        SpiFlashPageProgram(start_addr, idata, num_of_single);
    }
    return n_ret;
}

/**
* @brief: SpiFlashWriteBytesWithoutEraseNoAligned
* @param: start_addr
* @param: idata
* @param: length
* @return:
* @note:
*
*/
int32_t SpiFlashWriteBytesWithoutEraseNoAligned(uint32_t start_addr, uint16_t *idata, uint16_t length)
{
    uint16_t num_of_start = 0, num_of_end = 0;
    uint16_t num_of_mid = 0;
    int32_t n_ret = 0;
    uint8_t mid_page_num;
    uint16_t page_num;

    if(length == 0)
    {
        return n_ret;
    }
    page_num = (start_addr % FLASH_PAGE_SIZE + length + (FLASH_PAGE_SIZE - 1)) / FLASH_PAGE_SIZE;

    if(page_num == 1)
    {
        num_of_start = length;
        num_of_end = 0;
        num_of_mid = 0;
        mid_page_num = 0;
    }
    else
    {
        num_of_start = FLASH_PAGE_SIZE - start_addr % FLASH_PAGE_SIZE;
        num_of_end = (start_addr % FLASH_PAGE_SIZE + length) % FLASH_PAGE_SIZE;
        num_of_mid = length - num_of_start - num_of_end;
        mid_page_num = num_of_mid / FLASH_PAGE_SIZE;
    }

    /*第一个页*/
    if(num_of_start > 0)
    {
        SpiFlashPageProgram(start_addr, idata, num_of_start);
        start_addr +=  num_of_start;
        idata += num_of_start;
    }

    /*中间的页*/
    while(mid_page_num--)
    {
        SpiFlashPageProgram(start_addr, idata, FLASH_PAGE_SIZE);
        start_addr +=  FLASH_PAGE_SIZE;
        idata += FLASH_PAGE_SIZE;
    }
    /*最后一个页*/
    if(num_of_end > 0)
    {
        SpiFlashPageProgram(start_addr, idata, num_of_end);
    }

    return n_ret;
}

/**
* @brief: SpiFlashWriteBytesWithErase
* @param: start_addr
* @param: idata
* @param: length
* @return:
* @note:
*
*/
int32_t SpiFlashWriteBytesWithErase(uint32_t start_addr, uint16_t *idata, uint16_t length)
{
    int32_t n_ret = 0;

    n_ret = SpiFlashAutoEraseSector(start_addr, length);

    /* 将要写的数据写入flash中 */
    n_ret = SpiFlashWriteBytesWithoutEraseNoAligned(start_addr, idata, length);

    return n_ret;
}

/**
* @brief: FlashTest
* @param: void
* @note:
*
*/
bool FlashTest(void)
{
    uint16_t i = 0;

    for(i = 0; i < 256; i ++)
    {
        _garrDataBuf[i] = i;
    }
    SpiFlashWriteBytesWithErase(FLASH_TEST_ADDR, _garrDataBuf, FLASH_PAGE_SIZE);
    memset(_garrDataBuf, 0, sizeof(_garrDataBuf));
    SpiFlashReadBytes(FLASH_TEST_ADDR, _garrDataBuf, FLASH_PAGE_SIZE);

    for(i = 0; i < 256; i ++)
    {
        if(_garrDataBuf[i] != i)
        {
            return false;
        }
    }

    return true;
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

    uint32_t protected_start = 0x00000000;
    uint32_t protected_end   = 0x001FFFFF; /* 0x0000 0000 ~ 0x001 FFFFF */

    uint32_t op_end = addr + len - 1;

    /* 无交集 */
    if(op_end < protected_start || addr > protected_end)
    {
        return false;
    }

    return true; /* 有交集且已锁 */
}

void SpiFlashLock(void)
{
    _gFlashLocked = true;
}

void SpiFlashUnlock(void)
{
    _gFlashLocked = false;
}

bool SpiFlashIsLocked(void)
{
    return _gFlashLocked;
}


//
// End of file
//
