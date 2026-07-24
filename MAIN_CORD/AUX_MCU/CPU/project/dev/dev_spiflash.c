
#include "dev_spiflash.h"
#include "gpio.h"

#define _SpiFlashSelect()            GpioBitReset(&SpiFlashCsHandle)
#define _SpiFlashUnSelect()          GpioBitSet(&SpiFlashCsHandle)

#define _SpiFlashProtect()           
#define _SpiFlashUnProtect()         

#define  FLASH_PAGE_SIZE            (256)	
#define  FLASH_ERASE_SECTOR_SIZE    (0x1000)	
#define  FLASH_ERASE_BLOCK_SIZE     (0x10000)


DevSpiFlash_t gDevSpiFlash;

//uint8_t JEDECID[4];




/**
  * @brief   sets the Write Enable Latch bit in the Status Register
  * @param   dev: spi_flash device
  * @return  None
  * @note    None
  *
  */
static void _SpiFlashWriteEnable()
{
    uint8_t data = FLASH_WRITE_ENABLE;
    
    SpiFlashHandle.pTxBuf = &data;
    SpiFlashHandle.tx_size = 1;
    
    _SpiFlashSelect();
    SpiWrite8(&SpiFlashHandle);
    _SpiFlashUnSelect();
}


/**
  * @brief   sets the Write Enable Latch bit in the Status Register
  * @param   dev: spi_flash device
  * @return  None
  * @note    None
  *
  */
static void _SpiFlashReadStatusReg()
{
    uint32_t    time_out = 0xFFFF;
    uint8_t     rx_data = 0;
    uint8_t     tx_data = FLASH_READ_STATUS_REG;
    
    SpiFlashHandle.pTxBuf = &tx_data;
    SpiFlashHandle.tx_size = 1;
    
    SpiFlashHandle.pRxBuf  = &rx_data;
    SpiFlashHandle.rx_size = 1;
    
    _SpiFlashSelect();
    SpiWrite8(&SpiFlashHandle);
    
    do
    {
        SpiRead8(&SpiFlashHandle);
        time_out--;
    }while((rx_data & 0x01) && time_out);
    _SpiFlashUnSelect();
}


/**
  * @brief   Send 24bit Address to Spi
  * @param   addr: flash addr
  * @return  None
  * @note    None
  *
  */
static void _SpiFlashWrite24bitAddr(uint32_t addr)
{
    uint8_t     tx_data = 0;
    
    SpiFlashHandle.pTxBuf = &tx_data;
    SpiFlashHandle.tx_size = 1;
    
    tx_data = (addr >> 16) &0xff;
    SpiWrite8(&SpiFlashHandle);
    tx_data = (addr >> 8) & 0xff;
    SpiWrite8(&SpiFlashHandle);
    tx_data = addr & 0xff;
    SpiWrite8(&SpiFlashHandle);
}


/**
  * @brief   Send 24bit Address to Spi
  * @param   addr: flash addr
  * @return  None
  * @note    None
  *
  */
static void _SpiFlashWrite32bitAddr(uint32_t addr)
{
    uint8_t     tx_data = 0;
    
    SpiFlashHandle.pTxBuf = &tx_data;
    SpiFlashHandle.tx_size = 1;
    
    tx_data = (addr >> 24) &0xff;
    SpiWrite8(&SpiFlashHandle);
    tx_data = (addr >> 16) &0xff;
    SpiWrite8(&SpiFlashHandle);
    tx_data = (addr >> 8) & 0xff;
    SpiWrite8(&SpiFlashHandle);
    tx_data = addr & 0xff;
    SpiWrite8(&SpiFlashHandle);
}


/**
  * @brief   send no use data
  * @param   None
  * @return  None
  * @note    None
  *
  */
static void _SpiFlashWriteDummyByte()
{
    uint8_t     tx_data = 0xff;
    
    SpiFlashHandle.pTxBuf = &tx_data;
    SpiFlashHandle.tx_size = 1;
    SpiWrite8(&SpiFlashHandle);
}

//state:0表示不保护，1表示保护
static void _SpiFlashSetProtectOrUnProtect(uint8_t state)
{
    uint8_t     tx_data = FLASH_WRITE_STATUS_REG;
    uint8_t     rx_data = 0;
    
    SpiFlashHandle.pTxBuf = &tx_data;
    SpiFlashHandle.tx_size = 1;
    SpiFlashHandle.pRxBuf  = &rx_data;
    SpiFlashHandle.rx_size = 1;
    
    _SpiFlashUnProtect();
    _SpiFlashWriteEnable();
    _SpiFlashSelect();
    SpiWrite8(&SpiFlashHandle);
    
    if(state)
    {
        SpiWrite8(&SpiFlashHandle);
    } 
    else
    {
        tx_data = FLASH_UNPROTECT | 0x80;
        SpiWrite8(&SpiFlashHandle);
    }
    _SpiFlashUnSelect();
    _SpiFlashReadStatusReg();
    _SpiFlashProtect();
}

/**
  * @brief   Read Data instruction
  * @param   start_addr: start address
  * @param   p: data buffer pointer
  * @param   length: byte num
  * @return  None
  * @note    None
  *
  */
void SpiFlashReadData(uint32_t start_addr, uint8_t *p, uint32_t len)
{
    uint32_t time_out = 0xffff;
    uint8_t tx_cmd;
    
    
//    while((RESET == SPI_RxStreamIsReady(&SpiFlashHandle)) && time_out)
//    {
//        time_out--; 
//    }
//    while((RESET == SPI_TxStreamIsReady(&SpiFlashHandle)) && time_out)
//    {
//        time_out--; 
//    }
    
    _SpiFlashSelect();
    SpiFlashHandle.pTxBuf = &tx_cmd;
    SpiFlashHandle.tx_size = 1;
    
    if(gDevSpiFlash.n_capacity_MB >= 32)
    {
        tx_cmd = FLASH_LOW_READ_4BYTE;
        SpiWrite8(&SpiFlashHandle);
        _SpiFlashWrite32bitAddr(start_addr);
    }
    else
    {
        tx_cmd = FLASH_LOW_READ;
        SpiWrite8(&SpiFlashHandle);
        _SpiFlashWrite24bitAddr(start_addr);
    }
    
    SpiFlashHandle.pRxBuf  = p;
    SpiFlashHandle.rx_size = len;
    
    SpiRead8(&SpiFlashHandle);
    _SpiFlashUnSelect();
}


/**
  * @brief   Fast Read Data instruction
  * @param   start_addr: start address
  * @param   p: data buffer pointer
  * @param   length: byte num
  * @return  None
  * @note    None
  *
  */
void SpiFlashFastReadData(uint32_t start_addr, uint8_t *p, uint32_t len)
{
    uint32_t time_out = 0xffff;
    uint8_t tx_cmd;
    
//    while((RESET == SPI_RxStreamIsReady(&SpiFlashHandle)) && time_out)
//    {
//        time_out--; 
//    }
//    while((RESET == SPI_TxStreamIsReady(&SpiFlashHandle)) && time_out)
//    {
//        time_out--; 
//    }
    
    _SpiFlashSelect();
    SpiFlashHandle.pTxBuf = &tx_cmd;
    SpiFlashHandle.tx_size = 1;
    
    if(gDevSpiFlash.n_capacity_MB >= 32)
    {
        tx_cmd = FLASH_HIGH_READ_4BYTE;
        SpiWrite8(&SpiFlashHandle);
        _SpiFlashWrite32bitAddr(start_addr);
    }
    else
    {
        tx_cmd = FLASH_HIGH_READ;
        SpiWrite8(&SpiFlashHandle);
        _SpiFlashWrite24bitAddr(start_addr);
    }
    _SpiFlashWriteDummyByte();
    
    SpiFlashHandle.pRxBuf  = p;
    SpiFlashHandle.rx_size = len;
    
    SpiRead8(&SpiFlashHandle);
    _SpiFlashUnSelect();
}



/**
  * @brief   Page Program instruction(1 to 256 bytes)
  * @param   start_addr: start address
  * @param   length: byte num
  * @param   p: data buffer pointer
  * @return  None
  * @note    None
  *
  */
void SpiFlashPageProgram(uint32_t start_addr, uint8_t *p, uint32_t len)
{
    uint32_t time_out = 0xffff;
    uint8_t tx_cmd;
    
//    while((RESET == SPI_RxStreamIsReady(&SpiFlashHandle)) && time_out)
//    {
//        time_out--; 
//    }
//    while((RESET == SPI_TxStreamIsReady(&SpiFlashHandle)) && time_out)
//    {
//        time_out--; 
//    }
    
    _SpiFlashWriteEnable();
    _SpiFlashSelect();
    SpiFlashHandle.pTxBuf = &tx_cmd;
    SpiFlashHandle.tx_size = 1;
    
    if(gDevSpiFlash.n_capacity_MB >= 32)
    {
        tx_cmd = FLASH_PAGE_PROGRAM_4BYTE;
        SpiWrite8(&SpiFlashHandle);
        _SpiFlashWrite32bitAddr(start_addr);
    }
    else
    {
        tx_cmd = FLASH_PAGE_PROGRAM;
        SpiWrite8(&SpiFlashHandle);
        _SpiFlashWrite24bitAddr(start_addr);
    }
    
    SpiFlashHandle.pTxBuf = p;
    SpiFlashHandle.tx_size = len;
    SpiWrite8(&SpiFlashHandle);
    _SpiFlashUnSelect();
    _SpiFlashReadStatusReg();
}


/**
  * @brief   Sector Erase instruction
  * @param   start_addr: start address
  * @return  None
  * @note    None
  *
  */
void SpiFlashEraseSector(uint32_t start_addr)
{
    uint8_t tx_cmd;
    _SpiFlashUnProtect();
    _SpiFlashWriteEnable();
    _SpiFlashSelect();
    
    SpiFlashHandle.pTxBuf = &tx_cmd;
    SpiFlashHandle.tx_size = 1;
    
    if(gDevSpiFlash.n_capacity_MB >= 32)
    {
        tx_cmd = FLASH_SECTOR_ERASE_4BYTE;
        SpiWrite8(&SpiFlashHandle);
        _SpiFlashWrite32bitAddr(start_addr);
    }
    else
    {
        tx_cmd = FLASH_SECTOR_ERASE;
        SpiWrite8(&SpiFlashHandle);
        _SpiFlashWrite24bitAddr(start_addr);
    }
    
    _SpiFlashUnSelect();
    _SpiFlashReadStatusReg();
    
}


/**
  * @brief   Sector Erase instruction Auto
  * @param   start_addr: start address
  * @return  None
  * @note    None
  *
  */
void SpiFlashAutoEraseSectors(uint32_t start_addr, uint16_t len)
{
    uint32_t erase_addr = start_addr;
    uint8_t sector_num;
    
    if(start_addr % FLASH_ERASE_SECTOR_SIZE != 0)
    {
        if(len + start_addr % FLASH_ERASE_SECTOR_SIZE <= FLASH_ERASE_SECTOR_SIZE)
        {
            sector_num = 0;
        }
        else
        {
            sector_num = ((len - (FLASH_ERASE_SECTOR_SIZE - start_addr % FLASH_ERASE_SECTOR_SIZE)) + (FLASH_ERASE_SECTOR_SIZE - 1)) / FLASH_ERASE_SECTOR_SIZE;
        }
        start_addr += (FLASH_ERASE_SECTOR_SIZE - start_addr % FLASH_ERASE_SECTOR_SIZE);
    }
    else
    {
        sector_num = (len + (FLASH_ERASE_SECTOR_SIZE - 1)) / FLASH_ERASE_SECTOR_SIZE;
    }
    
    while(sector_num > 0)
    {
        SpiFlashEraseSector(start_addr);
        start_addr += FLASH_ERASE_SECTOR_SIZE;
        sector_num--;
    }
}


/**
  * @brief   Bytes Program without Erase
  * @param   start_addr: start address
  * @param   p_data: data buffer pointer
  * @param   length: byte num
  * @return  NONE
  */
void SpiFlashWriteBytesWithoutErase(uint32_t start_addr, uint8_t *p_data, uint16_t length)
{
    uint16_t num_of_page = 0, num_of_single = 0, addr = 0, count = 0, temp = 0;
    
    num_of_page =  length / FLASH_PAGE_SIZE;
    num_of_single = length % FLASH_PAGE_SIZE;
    
    while(num_of_page--)
    {
        SpiFlashPageProgram(start_addr, p_data, FLASH_PAGE_SIZE);
        start_addr +=  FLASH_PAGE_SIZE;
        p_data += FLASH_PAGE_SIZE;
    }
    if(num_of_single != 0)
    {
        SpiFlashPageProgram(start_addr, p_data, num_of_single);
    }
}

/**
  * @brief   Bytes Program without Erase, no need to consider if the addr is aligned with the sector
  * @param   start_addr: start address
  * @param   p_data: data buffer pointer
  * @param   length: byte num
  * @return  NONE
  */
void SpiFlashWriteBytesWithoutErasenoAligned(uint32_t start_addr, uint8_t *p_data, uint16_t length)
{
    uint16_t num_of_mid = 0, num_of_start = 0, num_of_end = 0, addr = 0, count = 0, temp = 0;
    uint16_t mid_page_num;
    
    uint16_t page_num;
    
    if(length == 0)
    {
        return;
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
    SpiFlashPageProgram(start_addr, p_data, num_of_start);
    start_addr +=  num_of_start;
    p_data += num_of_start;
    /*中间的页*/
    while(mid_page_num--)
    {
        SpiFlashPageProgram(start_addr, p_data, FLASH_PAGE_SIZE);
        start_addr +=  FLASH_PAGE_SIZE;
        p_data += FLASH_PAGE_SIZE;
    }
    /*最后一个页*/
    if(num_of_end > 0)
    {
        SpiFlashPageProgram(start_addr, p_data, num_of_end);
    }
}


/**
  * @brief   Bytes Program with Erase
  * @param   start_addr: start address
  * @param   p_data: data buffer pointer
  * @param   length: byte num
  * @return  NONE
  */
void SpiFlashWriteBytesWithErase(uint32_t start_addr, uint8_t *p_data, uint16_t length)
{
    int32_t n_ret = 0;
    
    SpiFlashAutoEraseSectors(start_addr, length);  

    /* 将要写的数据写入flash中 */
    SpiFlashWriteBytesWithoutErasenoAligned(start_addr, p_data, length);     
}









/**
  * @brief   Block Erase instruction(32K-bytes)
  * @param   start_addr: start address
  * @return  None
  * @note    None
  */
void SpiFlashEraseBlock32K(uint32_t start_addr)
{
    uint8_t tx_cmd;
    
    _SpiFlashWriteEnable();
    _SpiFlashSelect();
    
    SpiFlashHandle.pTxBuf = &tx_cmd;
    SpiFlashHandle.tx_size = 1;
    tx_cmd = FLASH_BLOCK_ERASE_32K;
    SpiWrite8(&SpiFlashHandle);
    _SpiFlashWrite24bitAddr(start_addr);
    _SpiFlashUnSelect();
    _SpiFlashReadStatusReg();
}

/**
  * @brief   Block Erase instruction(64K-bytes)
  * @param   start_addr: start address
  * @return  None
  * @note    None
  */
void SpiFlashEraseBlock64K(uint32_t start_addr)
{
    uint8_t tx_cmd;
    
    _SpiFlashWriteEnable();
    _SpiFlashSelect();
    
    SpiFlashHandle.pTxBuf = &tx_cmd;
    SpiFlashHandle.tx_size = 1;
    
    if(gDevSpiFlash.n_capacity_MB >= 32)
    {
        tx_cmd = FLASH_BLOCK_ERASE_64K_4BYTE;
        SpiWrite8(&SpiFlashHandle);
        _SpiFlashWrite32bitAddr(start_addr);
    }
    else
    {
        tx_cmd = FLASH_BLOCK_ERASE_64K;
        SpiWrite8(&SpiFlashHandle);
        _SpiFlashWrite24bitAddr(start_addr);
    }
    _SpiFlashUnSelect();
    _SpiFlashReadStatusReg();
}


/**
  * @brief   Chip Erase instruction
  * @return  None
  * @note    None
  */
void SpiFlashEraseChip(void)
{
    uint8_t tx_cmd;
    
    _SpiFlashWriteEnable();
    _SpiFlashSelect();
    
    SpiFlashHandle.pTxBuf = &tx_cmd;
    SpiFlashHandle.tx_size = 1;
    tx_cmd = FLASH_CHIP_ERASE;
    SpiWrite8(&SpiFlashHandle);
    _SpiFlashUnSelect();
    _SpiFlashReadStatusReg();
}


/**
  * @brief   SPI hardware get info
  * @param   description info struct of spi flash
  * @return  NONE
  * @note    None
  *
  */
void SpiFlashReadJEDECID(DevSpiFlash_t *p_dev)
{
    uint32_t i;
    uint8_t tx_cmd;
    
    _SpiFlashSelect();
    SpiFlashHandle.pTxBuf = &tx_cmd;
    SpiFlashHandle.tx_size = 1;
    tx_cmd = FLASH_JEDEC_ID;    
    SpiWrite8(&SpiFlashHandle);
    
    SpiFlashHandle.pRxBuf  = p_dev->arr_id;
    SpiFlashHandle.rx_size = 3;
    SpiRead8(&SpiFlashHandle);
    
    if(p_dev->arr_id[0])
    {
        if(0x14 == p_dev->arr_id[2])
        {
            p_dev->n_capacity_MB = 1;
        }
        else if(0x15 == p_dev->arr_id[2])
        {
            p_dev->n_capacity_MB = 2;
        }
        else if(0x16 == p_dev->arr_id[2])
        {
            p_dev->n_capacity_MB = 4;
        }
        else if(0x17 == p_dev->arr_id[2])
        {
            p_dev->n_capacity_MB = 8;
        }
        else if(0x18 == p_dev->arr_id[2])
        {
            p_dev->n_capacity_MB = 16;
        }
        else if(0x19 == p_dev->arr_id[2])
        {
            p_dev->n_capacity_MB = 32;
        }
        else if(0x20 == p_dev->arr_id[2] || 0x1A == p_dev->arr_id[2] || 0x3A == p_dev->arr_id[2])
        {
            p_dev->n_capacity_MB = 64;
        }
        else
        {
            //LOG_INFO(LOG_TAG,"FLASH ID unknow,default 8MB");
            p_dev->n_capacity_MB = 8;
        }
    }
    else
    {
        p_dev->n_capacity_MB = 0;
    }
}

void DevSpiFlashInit()
{
    SpiFlashBspInit();
    
    SpiFlashReadJEDECID(&gDevSpiFlash);
    _SpiFlashSetProtectOrUnProtect(0);
}

