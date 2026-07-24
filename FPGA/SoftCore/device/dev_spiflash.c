/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        modbus_analysis.c
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
  * <tr><td>2024-2-4    <td>V1.0.1    <td>xingxiao    <td>modbus_reg LSB Save
  * </table>
  ******************************************************************************
  */

#include "dev_spiflash.h"

#define SPI           ((CMSDK_SPI_TypeDef   *) 0x41002000   )
#define _SpiFlashSelect()            SPI->reg_cs = 0x00000000
#define _SpiFlashUnSelect()          SPI->reg_cs = 0x00000001
#define _SpiFpgaSelect()             SPI->reg_cs = 0x00000000
#define _SpiFpgaUnSelect()           SPI->reg_cs = 0x00000001
#define _SpiFlashProtect()           0
#define _SpiFlashUnProtect()         1
#define  FLASH_PAGE_SIZE            (256)
#define  FLASH_ERASE_SECTOR_SIZE    (0x1000)
#define  FLASH_ERASE_BLOCK_SIZE     (0x10000)

DevSpiFlash_t gDevSpiFlash;

/**
  * @brief   sets the Write Enable Latch bit in the Status Register
  * @param   dev: spi_flash device
  * @return  None
  * @note    None
  *
  */
static void _SpiFlashWriteEnable()//写使能  片选
{
    uint8_t data = FLASH_WRITE_ENABLE;

//    gstSpiFlashHandle.pTxBuf = &data;
//    gstSpiFlashHandle.tx_size = 1;

    _SpiFlashSelect();
//    SpiSend(&gstSpiFlashHandle);
//    SpiSend((uint8_t*)&data, 1);

    SpiSend((uint8_t*)&data, 1); //写
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
    uint8_t     tx_cmd = 0xFF;	
    uint8_t     tx_data = FLASH_READ_STATUS_REG;
//    gstSpiFlashHandle.pTxBuf = &tx_data;
//    gstSpiFlashHandle.tx_size = 1;

//    gstSpiFlashHandle.pRxBuf  = &rx_data;
//    gstSpiFlashHandle.rx_size = 1;

    _SpiFlashSelect();
    SpiSend((uint8_t*)&tx_data, 1);
    do
    {
		SpiSend((uint8_t*)&tx_data, 1);		
		SpiSend((uint8_t*)&tx_cmd, 1);
		SpiRead((uint8_t*)&rx_data, 1);
        time_out--;
    }
    while((rx_data & 0x01) && (time_out != 0));		
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

//    gstSpiFlashHandle.pTxBuf = &tx_data;
//    gstSpiFlashHandle.tx_size = 1;

    tx_data = (addr >> 16) & 0xff;
    SpiSend((uint8_t*)&tx_data, 1);
    tx_data = (addr >> 8) & 0xff;
    SpiSend((uint8_t*)&tx_data, 1);
    tx_data = addr & 0xff;
    SpiSend((uint8_t*)&tx_data, 1);
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

//    gstSpiFlashHandle.pTxBuf = &tx_data;
//    gstSpiFlashHandle.tx_size = 1;

    tx_data = (addr >> 24) & 0xff;
    SpiSend((uint8_t*)&tx_data, 1);
    tx_data = (addr >> 16) & 0xff;
    SpiSend((uint8_t*)&tx_data, 1);
    tx_data = (addr >> 8) & 0xff;
    SpiSend((uint8_t*)&tx_data, 1);
    tx_data = addr & 0xff;
    SpiSend((uint8_t*)&tx_data, 1);
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

//    gstSpiFlashHandle.pTxBuf = &tx_data;
//    gstSpiFlashHandle.tx_size = 1;
    SpiSend((uint8_t*)&tx_data, 1);
}

////state:0表示不保护，1表示保护
//void SpiFlashSetProtectOrUnProtect(uint8_t state)
//{
//    uint8_t     tx_data = FLASH_WRITE_STATUS_REG;
//    uint8_t     rx_data = 0;

////    gstSpiFlashHandle.pTxBuf = &tx_data;
////    gstSpiFlashHandle.tx_size = 1;
////    gstSpiFlashHandle.pRxBuf  = &rx_data;
////    gstSpiFlashHandle.rx_size = 1;

//    _SpiFlashUnProtect();
//    _SpiFlashWriteEnable();
//    _SpiFlashSelect();
//    SpiSend((uint8_t*)&tx_data, 1);

//    if(state)
//    {
//        SpiSend((uint8_t*)&tx_data, 1);
//    }
//    else
//    {
//        tx_data = FLASH_UNPROTECT | 0x80;
//        SpiSend((uint8_t*)&tx_data, 1);
//    }
//    _SpiFlashUnSelect();
//    _SpiFlashReadStatusReg();
//    _SpiFlashProtect();
//}

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
    uint8_t tx_cmd;
    _SpiFlashSelect();
    if(gDevSpiFlash.n_capacity_MB >= 32)
    {
        tx_cmd = FLASH_LOW_READ_4BYTE;
        SpiSend((uint8_t*)&tx_cmd, 1);
        _SpiFlashWrite32bitAddr(start_addr);
    }
    else
    {
        tx_cmd = FLASH_LOW_READ;
        SpiSend((uint8_t*)&tx_cmd, 1);
        _SpiFlashWrite24bitAddr(start_addr);
    }
    tx_cmd = 0xFF;
    SpiSend((uint8_t*)&tx_cmd, len);
    SpiRead(p, len);
    _SpiFlashUnSelect();
}

void SpiFlashReadAllData(uint32_t start_addr, uint8_t *p, uint32_t len)
{
    for(uint16_t i = 0; i < len; i++)
    {
        SpiFlashReadData(start_addr + i , (uint8_t*)&p[i], 1);
    }
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
    uint8_t tx_cmd;
    _SpiFlashSelect();
    if(gDevSpiFlash.n_capacity_MB >= 32)
    {
        tx_cmd = FLASH_HIGH_READ_4BYTE;
        SpiSend((uint8_t*)&tx_cmd, 1);
        _SpiFlashWrite32bitAddr(start_addr);
    }
    else
    {
        tx_cmd = FLASH_HIGH_READ;
        SpiSend((uint8_t*)&tx_cmd, 1);
        _SpiFlashWrite24bitAddr(start_addr);
    }
    _SpiFlashWriteDummyByte();
    SpiSend((uint8_t*)&p, 1);
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
    uint8_t tx_cmd;
    _SpiFlashWriteEnable();
    _SpiFlashSelect();
    if(gDevSpiFlash.n_capacity_MB >= 32)
    {
        tx_cmd = FLASH_PAGE_PROGRAM_4BYTE;
        SpiSend((uint8_t*)&tx_cmd, 1);
        _SpiFlashWrite32bitAddr(start_addr);
    }
    else
    {
        tx_cmd = FLASH_PAGE_PROGRAM;
        SpiSend((uint8_t*)&tx_cmd, 1);
        _SpiFlashWrite24bitAddr(start_addr);
    }
    SpiSend(p, len);
//    tx_cmd = 0x00;
//    SpiSend((uint8_t*)&tx_cmd, 1);
//    SpiSend((uint8_t*)&tx_cmd, 1);
//    SpiSend((uint8_t*)&tx_cmd, 1);
//    SpiSend((uint8_t*)&tx_cmd, 1);
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
    _SpiFlashWriteEnable();
    _SpiFlashSelect();
    if(gDevSpiFlash.n_capacity_MB >= 32)
    {
        tx_cmd = FLASH_SECTOR_ERASE_4BYTE;
        SpiSend((uint8_t*)&tx_cmd, 1);
        _SpiFlashWrite32bitAddr(start_addr);
    }
    else
    {
        tx_cmd = FLASH_SECTOR_ERASE;
        SpiSend((uint8_t*)&tx_cmd, 1);
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
//void SpiFlashAutoEraseSectors(uint32_t start_addr, uint16_t len)
//{
//    uint8_t sector_num;

//    if(start_addr % FLASH_ERASE_SECTOR_SIZE != 0)
//    {
//        start_addr += (FLASH_ERASE_SECTOR_SIZE - start_addr % FLASH_ERASE_SECTOR_SIZE);
//        sector_num = ((len - (FLASH_ERASE_SECTOR_SIZE - start_addr % FLASH_ERASE_SECTOR_SIZE)) + (FLASH_ERASE_SECTOR_SIZE - 1)) / FLASH_ERASE_SECTOR_SIZE;
//    }
//    else
//    {
//        sector_num = (len + (FLASH_ERASE_SECTOR_SIZE - 1)) / FLASH_ERASE_SECTOR_SIZE;
//    }

//    while(sector_num > 0)
//    {
//        SpiFlashEraseSector(start_addr);
//        start_addr += FLASH_ERASE_SECTOR_SIZE;
//        sector_num--;
//    }
//}

void SpiFlashAutoEraseSectors(uint32_t start_addr, uint16_t len)
{
    int32_t n_remain = 0;
    n_remain = start_addr % FLASH_ERASE_SECTOR_SIZE;

    if((n_remain == 0) || \
            (n_remain + len > FLASH_ERASE_SECTOR_SIZE))
    {
        if(n_remain != 0)
        {
            start_addr += FLASH_ERASE_SECTOR_SIZE;
        }
        SpiFlashEraseSector(start_addr);
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
    uint16_t num_of_page = 0, num_of_single = 0;

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
    uint16_t num_of_mid = 0, num_of_start = 0, num_of_end = 0;
    uint16_t mid_page_num;
    uint16_t page_num;
    if(length == 0)
    {
        return ;
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
        SpiFlashPageProgram(start_addr, p_data, num_of_start);
        start_addr +=  num_of_start;
        p_data += num_of_start;
    }
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
    tx_cmd = FLASH_BLOCK_ERASE_32K;
    SpiSend((uint8_t*)&tx_cmd, 1);
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
    if(gDevSpiFlash.n_capacity_MB >= 32)
    {
        tx_cmd = FLASH_BLOCK_ERASE_64K_4BYTE;
        SpiSend((uint8_t*)&tx_cmd, 1);
        _SpiFlashWrite32bitAddr(start_addr);
    }
    else
    {
        tx_cmd = FLASH_BLOCK_ERASE_64K;
        SpiSend((uint8_t*)&tx_cmd, 1);
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
void SpiFlashEraseChip()
{
    uint8_t tx_cmd;

    _SpiFlashWriteEnable();
    _SpiFlashSelect();
    tx_cmd = FLASH_CHIP_ERASE;
    SpiSend((uint8_t*)&tx_cmd, 1);
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
    uint16_t tx_cmd = 0;

    _SpiFlashSelect();
    tx_cmd = FLASH_JEDEC_ID;
    SpiSend((uint8_t*)&tx_cmd, 1);
    tx_cmd = 0x00;
    SpiSend((uint8_t*)&tx_cmd, 3);
    SpiRead((uint8_t*)&p_dev->arr_id, 3);
    _SpiFlashUnSelect();
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

void SpiWriteFpga(uint16_t start_addr, uint8_t *p_data, uint16_t length)
{
    uint8_t tx_data = 0x45;

    _SpiFpgaSelect();
    SpiSend((uint8_t*)&tx_data, 1);
    tx_data = (start_addr >> 8);
    SpiSend((uint8_t*)&tx_data, 1);
    tx_data = (start_addr & 0x00FF);
    SpiSend((uint8_t*)&tx_data, 1);
    SpiSend((uint8_t*)&p_data, length);
    tx_data = 0x00;
    SpiSend((uint8_t*)&tx_data, 1);
    _SpiFpgaUnSelect();
}

void SpiReadFpga(uint16_t start_addr, uint8_t *p_data, uint16_t length)
{
    uint8_t tx_data = 0x43;
    _SpiFpgaSelect();
    SpiSend((uint8_t*)&tx_data, 1);
    tx_data = (start_addr >> 8);
    SpiSend((uint8_t*)&tx_data, 1);
    tx_data = (start_addr & 0x00FF);
    SpiSend((uint8_t*)&tx_data, 1);
    tx_data = 0x00;
    SpiSend((uint8_t*)&tx_data, 1);
    SpiRead((uint8_t*)&p_data, length);
    _SpiFpgaUnSelect();
}


