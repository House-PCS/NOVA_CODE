/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT Nova
  * @file    dev_spi_flash.c
  * @author  Xi'an NovaStar Tech Co.,LTD.()
  * @version V1.0.0
  * @date    2021.08.04
  * @brief   CBB:This file provides the functions of dev_spi_flash.
  ******************************************************************************
  * @attention 如果使用Nowait读接口，必须在DMA中断中把flash的CS拉高
  * @relation

  * @par :
  * <table>
  * <tr><th>Date        <th>Version  <th>Author      <th>Description
  * <tr><td>2021/08/04  <td>1.0.0    <td>Changshuai  <td>CBB Version
  * </table>
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "dev_spiflash.h"
#include "dev_ext_wdg.h"
/* Private macro -------------------------------------------------------------*/
#define LOG_TAG "dev_spiflash"

#define  FLASH_PAGE_SIZE            (256)
#define  FLASH_ERASE_SECTOR_SIZE    (0x1000)
#define  FLASH_ERASE_BLOCK_SIZE     (0x10000)

SpiFlash_HandleType SpiFlash;
/* Private typedef -----------------------------------------------------------*/

typedef struct
{
    uint8_t  c_protect_byte;
    uint8_t  c_unprotect_byte;
} flash_cmd_t;

static int32_t _spi_flash_set_protect_or_unprotect(SpiFlash_HandleType *pSpiFlash);
static int32_t _spi_flash_read_status_reg(SpiFlash_HandleType *pSpiFlash);
static int32_t _spi_flash_write_unprotect(SpiFlash_HandleType *pSpiFlash);
static int32_t _spi_flash_write_protect(SpiFlash_HandleType *pSpiFlash);
static int32_t _spi_flash_chip_deselect(SpiFlash_HandleType *pSpiFlash);
static int32_t _spi_flash_chip_select(SpiFlash_HandleType *pSpiFlash);
static int32_t _spi_flash_rst(SpiFlash_HandleType *pSpiFlash);
static int32_t _spi_flash_un_rst(SpiFlash_HandleType *pSpiFlash);
static int32_t _spi_flash_write_32bit_addr (uint32_t n_addr);
static int32_t _spi_flash_write_24bit_addr (uint32_t n_addr);
/* Private function prototypes -----------------------------------------------*/
/**
  * @brief   chip select of spi flash
  * @param   description info struct of spi flash
  * @return  None
  * @note    None
  *
  */
static int32_t _spi_flash_chip_select(SpiFlash_HandleType *pSpiFlash)
{
    int32_t n_ret = 0;

    GpioBitReset(pSpiFlash->pCs);
    return n_ret;
}
/**
  * @brief   chip de-select of spi flash
  * @param   description info struct of spi flash
  * @return  None
  * @note    None
  *
  */
static int32_t _spi_flash_chip_deselect(SpiFlash_HandleType *pSpiFlash)
{
    int32_t n_ret = 0;

    GpioBitSet(pSpiFlash->pCs);
    return n_ret;
}

/**
  * @brief   chip write protect of spi flash
  * @param   description info struct of spi flash
  * @return  None
  * @note    None
  *
  */
static int32_t _spi_flash_write_protect(SpiFlash_HandleType *pSpiFlash)
{
    int32_t n_ret = 0;

    GpioBitReset(pSpiFlash->pWp);
    return n_ret;
}

/**
  * @brief   chip write unprotect of spi flash
  * @param   description info struct of spi flash
  * @return  None
  * @note    None
  *
  */
static int32_t _spi_flash_write_unprotect(SpiFlash_HandleType *pSpiFlash)
{
    int32_t n_ret = 0;

    GpioBitSet(pSpiFlash->pWp);
    return n_ret;
}

/**
  * @brief   chip write protect of spi flash
  * @param   description info struct of spi flash
  * @return  None
  * @note    None
  *
  */
static int32_t _spi_flash_rst(SpiFlash_HandleType *pSpiFlash)
{
    int32_t n_ret = 0;

//    GpioBitReset(pSpiFlash->pRst);
    return n_ret;
}

/**
  * @brief   chip write unprotect of spi flash
  * @param   description info struct of spi flash
  * @return  None
  * @note    None
  *
  */
static int32_t _spi_flash_un_rst(SpiFlash_HandleType *pSpiFlash)
{
    int32_t n_ret = 0;

//    GpioBitSet(pSpiFlash->pRst);
    return n_ret;
}

static void _spi_write_array(uint32_t len, uint8_t *pData)
{
    DevFeedExtWdg();
    SpiFlashHandle.pTxBuf = pData;
    SpiFlashHandle.tx_size = len;
    SpiWrite8(&SpiFlashHandle);
}

static uint8_t _spi_read_array(uint32_t len, uint8_t *pData)
{
    uint8_t data;

    SpiFlashHandle.pRxBuf  = pData;
    SpiFlashHandle.rx_size = len;

    SpiRead8(&SpiFlashHandle);
    return data;
}

/**
  * @brief   sets the Write Enable Latch bit in the Status Register
  * @param   dev: spi_flash device
  * @return  None
  * @note    None
  *
  */
static int32_t _spi_flash_read_status_reg(SpiFlash_HandleType *pSpiFlash)
{
    int32_t n_ret = 0;
    uint32_t  time_out = 0xFFFF;
    uint8_t c_txdata = 0;
    uint8_t c_rxdata = 0;

    _spi_flash_chip_select(pSpiFlash);
    c_txdata = FLASH_READ_STATUS_REG;
    _spi_write_array(1, &c_txdata);
//    c_txdata = DUMMY_BYTE & 0x01;

    do
    {
        _spi_read_array(1, &c_rxdata);
        time_out--;
    }
    while((c_rxdata & 0x01) && (time_out != 0));

    _spi_flash_chip_deselect(pSpiFlash);

    return n_ret;
}

/**
  * @brief   sets the Write Enable Latch bit in the Status Register
  * @param   dev: spi_flash device
  * @return  None
  * @note    None
  *
  */
static int32_t spi_flash_write_enable(SpiFlash_HandleType *pSpiFlash)
{
    int32_t n_ret = 0;
    uint8_t c_txdata = 0;
    uint8_t c_rxdata = 0;

    _spi_flash_chip_select(pSpiFlash);
    c_txdata = FLASH_WRITE_ENABLE;
    _spi_write_array(1, &c_txdata);
    _spi_flash_chip_deselect(pSpiFlash);
end:
    return n_ret;
}
/**
  * @brief   Page Program instruction(1 to 256 bytes)
  * @param   dev: spi_flash device
  * @param   start_addr: start address
  * @param   length: byte num
  * @param   idata: data buffer pointer
  * @return  None
  * @note    None
  *
  */
static int32_t spi_flash_page_program(uint32_t start_addr, uint8_t *idata, uint16_t length)
{
    int32_t n_ret = 0;
    uint8_t c_txdata = 0;
    uint8_t c_rxdata = 0;

    spi_flash_write_enable(&SpiFlash);
    _spi_flash_chip_select(&SpiFlash);
    if(SpiFlash.n_capacity_MB >= 32)
    {
        c_txdata = FLASH_PAGE_PROGRAM_4BYTE;
        _spi_write_array(1, &c_txdata);
        _spi_flash_write_32bit_addr(start_addr);
    }
    else
    {
        c_txdata = FLASH_PAGE_PROGRAM;
        _spi_write_array(1, &c_txdata);
        _spi_flash_write_24bit_addr(start_addr);
    }
    _spi_write_array(length, idata);
    _spi_flash_chip_deselect(&SpiFlash);
    _spi_flash_read_status_reg(&SpiFlash);
end:
    return n_ret;
}
/**
  * @brief   Protect or UnProtect the Spi Flash.
  * @param   dev: spi_flash device
  * @param   protect_state: !0:protect,0:unprotect
  * @return  None
  * @note    None
  *
  */
static int32_t _spi_flash_set_protect_or_unprotect(SpiFlash_HandleType *pSpiFlash)
{
    int32_t n_ret = 0;
    uint8_t c_txdata = 0;
    uint8_t c_rxdata = 0;

    spi_flash_write_enable(pSpiFlash);
    _spi_flash_chip_select(pSpiFlash);
    c_txdata = FLASH_WRITE_STATUS_REG;
    _spi_write_array(1, &c_txdata);

    if(pSpiFlash->c_protect_state)
    {
        c_txdata = FLASH_PROJECT;
        _spi_write_array(1, &c_txdata);
    }
    else
    {
        c_txdata = FLASH_UNPROJECT | 0x80;
        _spi_write_array(1, &c_txdata);

        _spi_flash_chip_deselect(pSpiFlash);
        _spi_flash_read_status_reg(pSpiFlash);

        spi_flash_write_enable(pSpiFlash);
        _spi_flash_chip_select(pSpiFlash);
        c_txdata = FLASH_WRITE_STATUS_REG2;
        _spi_write_array(1, &c_txdata);
        c_txdata = FLASH_UNPROJECT;
        _spi_write_array(1, &c_txdata);
    }

    _spi_flash_chip_deselect(pSpiFlash);
    _spi_flash_read_status_reg(pSpiFlash);
end:
    return n_ret;
}


/**
  * @brief   SPI hardware get info
  * @param   description info struct of spi flash
  * @return  false or true
  * @note    None
  *
  */
static bool spi_flash_get_info(SpiFlash_HandleType *pSpiFlash)
{
    int32_t n_ret = 0;
    uint16_t c_txdata = 0;
    uint8_t c_rxdata = 0;

    _spi_flash_chip_deselect(pSpiFlash);
    _spi_flash_chip_select(pSpiFlash);

    c_txdata = FLASH_JEDEC_ID;
    _spi_write_array(1, (uint8_t *)&c_txdata);
    _spi_read_array(3, SpiFlash.arr_id);
    _spi_flash_chip_deselect(pSpiFlash);

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
//            LOG_INFO(LOG_TAG,"FLASH ID unknow,default 8MB");
            SpiFlash.n_capacity_MB = 8;
        }
    }
    else
    {
//        LOG_ERROR(LOG_TAG,"FLASH ID=0,default 0MB");
        SpiFlash.n_capacity_MB = 0;
    }
end:
    return n_ret;
}
/**
  * @brief   SPI hardware intialization
  * @param   description info struct of spi flash
  * @return  false or true
  * @note    None
  *
  */
void SpiFlashInit(void)
{
    int32_t n_ret = 0;
    uint32_t i = 0;

    SpiFlash.pCs = &CsSpiFlashHandle;
    SpiFlash.pWp = &WpSpiFlashHandle;
    SpiFlash.pSpi = &SpiFlashHandle;
    SpiFlash.c_protect_state = EN_FLASH_UNPROTECT;

    SpiFlashBspInit();

    _spi_flash_un_rst(&SpiFlash);
    _spi_flash_chip_select(&SpiFlash);
    _spi_flash_write_unprotect(&SpiFlash);
    spi_flash_get_info(&SpiFlash);
    _spi_flash_set_protect_or_unprotect(&SpiFlash);
    _spi_flash_chip_deselect(&SpiFlash);
//    LOG_INFO(LOG_TAG,"Flash_ID:0x%X%X%X Cap:%dMb", SpiFlash.arr_id[2], SpiFlash.arr_id[1], \
    SpiFlash.arr_id[0], SpiFlash.n_capacity_MB);

}


/*----------------------------user code end-----------------------------------*/



/**
  * @brief   Send 24bit Address to Spi
  * @param   dev: spi_flash device
  * @param   add_dummy_byte: special for fast read operation
  * @return  None
  * @note    None
  *
  */
static int32_t _spi_flash_write_24bit_addr (uint32_t n_addr)
{
    int32_t n_ret = 0;
    uint8_t c_txdata = 0;

    c_txdata = n_addr >> 16;
    _spi_write_array(1, &c_txdata);
    c_txdata = n_addr >> 8;
    _spi_write_array(1, &c_txdata);
    c_txdata = n_addr;
    _spi_write_array(1, &c_txdata);
//    if(add_dummy_byte)
//    {
//        c_txdata = DUMMY_BYTE;
//        _spi_write_array(1, &c_txdata);
//    }

    return n_ret;
}

/**
  * @brief   Send 32bit Address to Spi
  * @param   dev: spi_flash device
  * @param   add_dummy_byte: special for fast read operation
  * @return  None
  * @note    None
  *
  */
static int32_t _spi_flash_write_32bit_addr (uint32_t n_addr)
{
    int32_t n_ret = 0;
    uint8_t c_txdata = 0;
    uint8_t c_rxdata = 0;

    c_txdata = n_addr >> 24;
    _spi_write_array(1, &c_txdata);
    c_txdata = n_addr >> 16;
    _spi_write_array(1, &c_txdata);
    c_txdata = n_addr >> 8;
    _spi_write_array(1, &c_txdata);
    c_txdata = n_addr;
    _spi_write_array(1, &c_txdata);
//    if(add_dummy_byte)
//    {
//        c_txdata = DUMMY_BYTE;
//        _spi_write_array(1, &c_txdata);
//    }
    return n_ret;
}


/**
  * @brief   Read Data instruction
  * @param   dev: spi_flash device
  * @param   start_addr: start address
  * @param   odata: data buffer pointer
  * @param   length: byte num
  * @return  None
  * @note    None
  *
  */
int32_t spi_flash_read_bytes(uint32_t start_addr, uint8_t *idata, uint16_t length)
{
    int32_t n_ret = 0;
    uint8_t c_txdata = 0;
    SpiFlash.n_start_addr = start_addr;

    _spi_flash_chip_select(&SpiFlash);
    if(SpiFlash.n_capacity_MB >= 32)
    {
        c_txdata = FLASH_LOW_READ_4BYTE;
        _spi_write_array(1, &c_txdata);
        _spi_flash_write_32bit_addr(start_addr);
    }
    else
    {
        c_txdata = FLASH_LOW_READ;
        _spi_write_array(1, &c_txdata);
        _spi_flash_write_24bit_addr(start_addr);
    }
    _spi_read_array(length, idata);
    _spi_flash_chip_deselect(&SpiFlash);
end:
    return n_ret;
}

/**
  * @brief   Sector Erase instruction(4K-bytes), till erase finished
  * @param   dev: spi_flash device
  * @param   start_addr: start address
  * @return  None
  * @note    None
  *
  */
int32_t spi_flash_auto_erase_sector(uint32_t start_addr, uint16_t length)
{
    int32_t n_ret = 0;
    int32_t n_remain = 0;
    uint8_t c_txdata = 0;

    n_remain = start_addr % FLASH_ERASE_SECTOR_SIZE;
    if((n_remain == 0) || \
            (n_remain + length > FLASH_ERASE_SECTOR_SIZE))
    {
        if(n_remain != 0)
        {
            start_addr += FLASH_ERASE_SECTOR_SIZE;
        }

        spi_flash_write_enable(&SpiFlash);
        _spi_flash_chip_select(&SpiFlash);
        if(SpiFlash.n_capacity_MB >= 32)
        {
            c_txdata = FLASH_SECTOR_ERASE_4BYTE;
            _spi_write_array(1, &c_txdata);
            _spi_flash_write_32bit_addr(start_addr);
        }
        else
        {
            c_txdata = FLASH_SECTOR_ERASE;
            _spi_write_array(1, &c_txdata);
            _spi_flash_write_24bit_addr(start_addr);
        }
        _spi_flash_chip_deselect(&SpiFlash);
        _spi_flash_read_status_reg(&SpiFlash);
    }
    return n_ret;
}

/**
  * @brief   Block Erase instruction(32K-bytes)
  * @param   dev: spi_flash device
  * @param   start_addr: start address
  * @return  None
  * @note    None
  *
  */
int32_t spi_flash_erase_block_32k(uint32_t start_addr)
{
    int32_t n_ret = 0;
    uint8_t c_txdata = 0;

    spi_flash_write_enable(&SpiFlash);
    _spi_flash_chip_select(&SpiFlash);
    c_txdata = FLASH_BLOCK_ERASE_32K;
    _spi_write_array(1, &c_txdata);
    _spi_flash_write_24bit_addr(start_addr);
    _spi_flash_chip_deselect(&SpiFlash);
    _spi_flash_read_status_reg(&SpiFlash);

    return n_ret;
}

/**
  * @brief   Block Erase instruction(64K-bytes)
  * @param   dev: spi_flash device
  * @param   start_addr: start address
  * @return  None
  * @note    None
  *
  */
int32_t spi_flash_erase_block_64k(uint32_t start_addr, uint16_t n_len, uint8_t c_mode)
{
    int32_t n_ret = 0;
    int32_t n_remain = 0;
    uint8_t c_txdata = 0;
    static uint32_t sn_last_erase_addr = 0xFFFFFFFF;

    n_remain = start_addr % (FLASH_ERASE_SECTOR_SIZE);

    if(c_mode == 1)
    {
        /*初次，强制擦除*/
        sn_last_erase_addr = 0xFFFFFFFF;
    }

    if((n_remain == 0) && \
            (((sn_last_erase_addr != 0xFFFFFFFF) && ((start_addr + n_len) > (sn_last_erase_addr + (FLASH_ERASE_SECTOR_SIZE << 4)))) || \
             ((sn_last_erase_addr != 0xFFFFFFFF) && (start_addr < (sn_last_erase_addr))) || \
             (sn_last_erase_addr == 0xFFFFFFFF)))
    {
        sn_last_erase_addr = start_addr;
        _spi_flash_read_status_reg(&SpiFlash);
        spi_flash_write_enable(&SpiFlash);
        _spi_flash_chip_select(&SpiFlash);

//        LOG_DEBUG(LOG_TAG, "Flash erase addr 0x%08x ", 'A', start_addr);

        if(SpiFlash.n_capacity_MB >= 32)
        {
            c_txdata = FLASH_BLOCK_ERASE_64K_4BYTE;
            _spi_write_array(1, &c_txdata);
            _spi_flash_write_32bit_addr(start_addr);
        }
        else
        {
            c_txdata = FLASH_BLOCK_ERASE_64K;
            _spi_write_array(1, &c_txdata);
            _spi_flash_write_24bit_addr(start_addr);
        }
        _spi_flash_chip_deselect(&SpiFlash);
    }
    else if(n_remain != 0)
    {
//        LOG_ERROR(LOG_TAG, "Flash erase addr 0x%08x not 4k", 'A', start_addr);
    }

    return n_ret;
}

/**
  * @brief   Chip Erase instruction
  * @param   dev: spi_flash device
  * @return  None
  * @note    None
  *
  */
int32_t spi_flash_erase_chip(void)
{
    int32_t n_ret = 0;
    uint8_t c_txdata = 0;

    spi_flash_write_enable(&SpiFlash);
    _spi_flash_chip_select(&SpiFlash);
    c_txdata = FLASH_BLOCK_ERASE_64K;
    _spi_write_array(1, &c_txdata);
    _spi_flash_chip_deselect(&SpiFlash);
    _spi_flash_read_status_reg(&SpiFlash);

    return n_ret;
}

/**
  * @brief   Bytes Program without Erase
  * @param   dev: spi_flash device
  * @param   start_addr: start address
  * @param   idata: data buffer pointer
  * @param   length: byte num
  * @return
  * - true
  * - false
  * @note    None
  *
  */
int32_t spi_flash_write_bytes_without_erase(uint32_t start_addr, uint8_t *idata, uint16_t length)
{
    uint8_t num_of_page = 0, num_of_single = 0, addr = 0, count = 0, temp = 0;
    int32_t n_ret = 0;

    num_of_page =  length / FLASH_PAGE_SIZE;
    num_of_single = length % FLASH_PAGE_SIZE;

    while(num_of_page--)
    {
        spi_flash_page_program(start_addr, idata, FLASH_PAGE_SIZE);
        start_addr +=  FLASH_PAGE_SIZE;
        idata += FLASH_PAGE_SIZE;
    }
    if(num_of_single != 0)
    {
        spi_flash_page_program(start_addr, idata, num_of_single);
    }
    return n_ret;
}


int32_t spi_flash_write_bytes_without_erase_noAligned(uint32_t start_addr, uint8_t *idata, uint16_t length)
{
    uint8_t num_of_start = 0, num_of_end = 0;
    uint16_t num_of_mid = 0;
    int32_t n_ret = 0;
    uint8_t mid_page_num;

    num_of_start = FLASH_PAGE_SIZE - start_addr % FLASH_PAGE_SIZE;
    num_of_end = (length - num_of_start) % FLASH_PAGE_SIZE;
    num_of_mid = length - num_of_start - num_of_end;

    mid_page_num = num_of_mid / FLASH_PAGE_SIZE;
    /*第一个页*/
    if(num_of_start > 0)
    {
        spi_flash_page_program(start_addr, idata, num_of_start);
        start_addr +=  num_of_start;
        idata += num_of_start;
    }
    /*中间的页*/
    while(mid_page_num--)
    {
        spi_flash_page_program(start_addr, idata, FLASH_PAGE_SIZE);
        start_addr +=  FLASH_PAGE_SIZE;
        idata += FLASH_PAGE_SIZE;
    }
    /*最后一个页*/
    if(num_of_end > 0)
    {
        spi_flash_page_program(start_addr, idata, num_of_end);
    }

    return n_ret;
}
/**
  * @brief   Bytes Program without Erase
  * @param   dev: spi_flash device
  * @param   start_addr: start address
  * @param   idata: data buffer pointer
  * @param   length: byte num
  * @return
  * - true
  * - false
  * @note    None
  *
  */
int32_t spi_flash_write_bytes_with_erase(uint32_t start_addr, uint8_t *idata, uint16_t length)
{
    int32_t n_ret = 0;

    n_ret = spi_flash_auto_erase_sector(start_addr, length);

    /* 将要写的数据写入flash中 */
    n_ret = spi_flash_write_bytes_without_erase_noAligned(start_addr, idata, length);

    return n_ret;
}


//uint8_t arr_rdata[4096] = {0};
//uint8_t arr_wdata[4096] = {0};

//static uint8_t flash_test(void)
//{
//
//    for(unsigned int i = 0; i < 4096; i ++)
//    {
//        arr_wdata[i] = i % 256;
//    }
//    spi_flash_write_bytes_with_erase(26, arr_wdata, 4096);
//    spi_flash_read_bytes(0, arr_rdata, 4096);
//    for(unsigned int i = 0; i < 4096; i ++)
//    {
//        if(arr_wdata[i] != arr_rdata[i])
//        {
//            LOG_ERROR(LOG_TAG,"spi_test ERROR!!!");
//            return 1;
//        }
//    }
//    LOG_INFO(LOG_TAG,"spi_flash_test OK!!!");
//    return 0;
//
//}

//FINSH_CMD_EXPORT(flash_test,  test 0-4k addr flash erase->write->read);
//MSH_CMD_EXPORT(flash_test,  test 0-4k addr flash erase->write->read);