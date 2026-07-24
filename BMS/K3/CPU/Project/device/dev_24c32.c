/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_24c32.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-6-1
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2025-6-1    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
/******************************************************************************/
/** 简    介：用于实现二线制串行（I2C）EEPROM AT24CXX系列的操作具有1K、2K、  **/
/**           4K、8K、16K、32K、64K、128K、256K、512K、1M、2M等多种容量，不  **/
/**           同容量其采用的设备地址位、寄存器地址位存在差异每一页所包含，   **/
/**           的字节数也不一样。具体配置如下：                               **/
/**  +----------+-------------+--------------------+---------------+--------+**/
/**  |   型号   |  容量结构   |     设备地址位     |   寄存器地址  |每页字节|**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24C01C  |128x8(1K)    |1|0|1|0|A2|A1|A0|R/W|7位(1个寄存器) |  8字节 |**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24C01D  |128x8(1K)    |1|0|1|0|A2|A1|A0|R/W|7位(1个寄存器) |  8字节 |**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24C02C  |256x8(2K)    |1|0|1|0|A2|A1|A0|R/W|8位(1个寄存器) |  8字节 |**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24C02D  |256x8(2K)    |1|0|1|0|A2|A1|A0|R/W|8位(1个寄存器) |  8字节 |**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24C04C  |512x8(4K)    |1|0|1|0|A2|A1|P0|R/W|9位(1个寄存器) | 16字节 |**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24C04D  |512x8(4K)    |1|0|1|0|A2|A1|P0|R/W|9位(1个寄存器) | 16字节 |**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24C08C  |1024x8(8K)   |1|0|1|0|A2|P1|P0|R/W|10位(1个寄存器)| 16字节 |**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24C08D  |1024x8(8K)   |1|0|1|0|A2|P1|P0|R/W|10位(1个寄存器)| 16字节 |**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24C16C  |2048x8(16K)  |1|0|1|0|P2|P1|P0|R/W|11位(1个寄存器)| 16字节 |**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24C16D  |2048x8(16K)  |1|0|1|0|P2|P1|P0|R/W|11位(1个寄存器)| 16字节 |**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24C32D  |4096x8(32K)  |1|0|1|0|A2|A1|A0|R/W|12位(2个寄存器)| 32字节 |**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24C32E  |4096x8(32K)  |1|0|1|0|A2|A1|A0|R/W|12位(2个寄存器)| 32字节 |**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24C64B  |8192x8(64K)  |1|0|1|0|A2|A1|A0|R/W|13位(2个寄存器)| 32字节 |**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24C64D  |8192x8(64K)  |1|0|1|0|A2|A1|A0|R/W|13位(2个寄存器)| 32字节 |**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24C128C |16384x8(128K)|1|0|1|0|A2|A1|A0|R/W|14位(2个寄存器)| 64字节 |**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24C256C |32768x8(256K)|1|0|1|0|A2|A1|A0|R/W|15位(2个寄存器)| 64字节 |**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24C512C |65536x8(512K)|1|0|1|0|A2|A1|A0|R/W|16位(2个寄存器)| 128字节|**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24CM01  |131072x8(1M) |1|0|1|0|A2|A1|P0|R/W|17位(2个寄存器)| 256字节|**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24CM02  |262144x8(2M) |1|0|1|0|A2|P1|P0|R/W|18位(2个寄存器)| 256字节|**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24CS01  |128x8(1K)    |1|0|1|0|A2|A1|A0|R/W|7位(1个寄存器) |  8字节 |**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24CS02  |256x8(2K)    |1|0|1|0|A2|A1|A0|R/W|8位(1个寄存器) |  8字节 |**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24CS04  |512x8(4K)    |1|0|1|0|A2|A1|P0|R/W|9位(1个寄存器) | 16字节 |**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24CS08  |1024x8(8K)   |1|0|1|0|A2|P1|P0|R/W|10位(1个寄存器)| 16字节 |**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24CS16  |2048x8(16K)  |1|0|1|0|P2|P1|P0|R/W|11位(1个寄存器)| 16字节 |**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24CS32  |4096x8(32K)  |1|0|1|0|A2|A1|A0|R/W|12位(2个寄存器)| 32字节 |**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24CS64  |8192x8(64K)  |1|0|1|0|A2|A1|A0|R/W|13位(2个寄存器)| 32字节 |**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24CSW010|128x8(1K)    |1|0|1|0|A2|A1|A0|R/W|7位(1个寄存器) |  8字节 |**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24CSW020|256x8(2K)    |1|0|1|0|A2|A1|A0|R/W|8位(1个寄存器) |  8字节 |**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24CSW040|512x8(4K)    |1|0|1|0|A2|A1|P0|R/W|9位(1个寄存器) | 16字节 |**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24CSW080|1024x8(8K)   |1|0|1|0|A2|P1|P0|R/W|10位(1个寄存器)| 16字节 |**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24HC02C |256x8(2K)    |1|0|1|0|A2|A1|A0|R/W|8位(1个寄存器) |  8字节 |**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24HC04B |512x8(4K)    |1|0|1|0|A2|A1|P0|R/W|9位(1个寄存器) | 16字节 |**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24MAC402|256x8(2K)    |1|0|1|0|A2|A1|A0|R/W|8位(1个寄存器) | 16字节 |**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**  |AT24MAC602|256x8(2K)    |1|0|1|0|A2|A1|A0|R/W|8位(1个寄存器) | 16字节 |**/
/**  +----------+-------------+-+-+-+-+--+--+--+---+---------------+--------+**/
/**           有些容量的设备地址段有些位用于寄存器地址（P）寻址，设备地址的可**/
/**           设定位数（A）决定了同一总线上所挂载的器件数量：                **/
/**               3位用于设备地址，0位用于内存寻址：可挂载8个；              **/
/**               2位用于设备地址，1位用于内存寻址：可挂载4个；              **/
/**               1位用于设备地址，2位用于内存寻址：可挂载2个；              **/
/**               0位用于设备地址，3位用于内存寻址：可挂载1个；              **/
/**--------------------------------------------------------------------------**/
/******************************************************************************/

#include "dev_24c32.h"
#include "stddef.h"

#define LOG_TAG     "dev_24c32"
#define USE_SIM_IIC     1

#define EEPROM_DEVICE_ADDR  0xA0
#define EEPROM_PAGE_SIZE    32

//I2CEepromHandle.

void DevEepromCtlInit()
{
#if USE_SIM_IIC == 0
    I2CEepromHandle.slave_addr0 = EEPROM_DEVICE_ADDR;
    I2CEepromHandle.reg_addr_width = EN_I2C_REG_ADDR_WIDTH_16;
    I2CEepromBspInit();
#else
    SimI2CEepromHandle.slave_addr0 = EEPROM_DEVICE_ADDR;
    SimI2CEepromHandle.reg_addr_width = EN_SIMI2C_REG_ADDR_WIDTH_16;
    I2CEepromBspInit();

#endif

    if(EepromTest() == false)
    {
        gFaultAnalysisFlag.e2prom_error_flag = 1;
    }
}


static void _EepromPageWrite(uint8_t *pBuf, uint16_t addr, uint16_t size)
{
#if USE_SIM_IIC == 0
    I2CEepromHandle.pTxBuf = pBuf;
    I2CEepromHandle.reg_addr = addr;
    I2CEepromHandle.tx_size = size;
    I2CRegWrite(&I2CEepromHandle);
#else
    SimI2CEepromHandle.pTxBuf = pBuf;
    SimI2CEepromHandle.reg_addr = addr;
    SimI2CEepromHandle.tx_size = size;
    SimI2CRegWrite(&SimI2CEepromHandle);
#endif
}

void EepromWrite2(uint8_t *pBuf, uint16_t addr, uint16_t size)
{
    uint8_t first_offset = 0;
    uint8_t last_offset = 0;
    uint8_t page_num = 0;


    first_offset = addr % EEPROM_PAGE_SIZE;
    last_offset = (size + first_offset - 1) % EEPROM_PAGE_SIZE;

    page_num = (size + first_offset + (EEPROM_PAGE_SIZE - 1)) / EEPROM_PAGE_SIZE;


    if(page_num > 1)
    {
        /*第一个page*/
        _EepromPageWrite(pBuf, addr, EEPROM_PAGE_SIZE - first_offset);
#if (USE_SIM_IIC == 0)
        I2CDeviceWaitStandby(&I2CEepromHandle);
#else
        soft_delayms(10);
#endif
        addr += EEPROM_PAGE_SIZE - first_offset;
        pBuf += EEPROM_PAGE_SIZE - first_offset;

        /*中间page*/
        if(page_num > 2)
        {
            for(uint8_t i = 0; i < page_num - 2; i++)
            {
                _EepromPageWrite(pBuf, addr, EEPROM_PAGE_SIZE);
#if (USE_SIM_IIC == 0)
                I2CDeviceWaitStandby(&I2CEepromHandle);
#else
                soft_delayms(10);
#endif
                addr += EEPROM_PAGE_SIZE;
                pBuf += EEPROM_PAGE_SIZE;
            }
        }
        /*最后一个page*/
        _EepromPageWrite(pBuf, addr, last_offset + 1);
#if (USE_SIM_IIC == 0)
        I2CDeviceWaitStandby(&I2CEepromHandle);
#else
        soft_delayms(10);
#endif
    }
    else
    {
        _EepromPageWrite(pBuf, addr, size);
#if (USE_SIM_IIC == 0)
        I2CDeviceWaitStandby(&I2CEepromHandle);
#else
        soft_delayms(10);
#endif
    }

}


void EepromWrite(uint8_t *pBuf, uint16_t addr, uint16_t size)
{
    uint8_t number_of_page = 0, number_of_single = 0, address = 0, count = 0;

    address = addr % EEPROM_PAGE_SIZE;
    count = EEPROM_PAGE_SIZE - address;
    number_of_page =  size / EEPROM_PAGE_SIZE;
    number_of_single = size % EEPROM_PAGE_SIZE;

    /* if write_address is EEPROM_PAGE_SIZE aligned  */
    if(0 == address)
    {
        while(number_of_page--)
        {
            _EepromPageWrite(pBuf, addr, EEPROM_PAGE_SIZE);
#if (USE_SIM_IIC == 0)
            I2CDeviceWaitStandby(&I2CEepromHandle);
#else
            SimI2CWaitStandby(&SimI2CEepromHandle);
#endif
            addr +=  EEPROM_PAGE_SIZE;
            pBuf += EEPROM_PAGE_SIZE;
        }
        if(0 != number_of_single)
        {
            _EepromPageWrite(pBuf, addr, number_of_single);
#if (USE_SIM_IIC == 0)
            I2CDeviceWaitStandby(&I2CEepromHandle);
#else
            SimI2CWaitStandby(&SimI2CEepromHandle);
#endif
        }
    }
    else
    {
        /* if write_address is not EEPROM_PAGE_SIZE aligned */
        if(size < count)
        {
            _EepromPageWrite(pBuf, addr, size);
#if (USE_SIM_IIC == 0)
            I2CDeviceWaitStandby(&I2CEepromHandle);
#else
            SimI2CWaitStandby(&SimI2CEepromHandle);
#endif
        }
        else
        {
            size -= count;
            number_of_page =  size / EEPROM_PAGE_SIZE;
            number_of_single = size % EEPROM_PAGE_SIZE;

            if(0 != count)
            {
                _EepromPageWrite(pBuf, addr, count);
#if (USE_SIM_IIC == 0)
                I2CDeviceWaitStandby(&I2CEepromHandle);
#else
                SimI2CWaitStandby(&SimI2CEepromHandle);
#endif
                addr += count;
                pBuf += count;
            }
            /* write page */
            while(number_of_page--)
            {
                _EepromPageWrite(pBuf, addr, EEPROM_PAGE_SIZE);
#if (USE_SIM_IIC == 0)
                I2CDeviceWaitStandby(&I2CEepromHandle);
#else
                SimI2CWaitStandby(&SimI2CEepromHandle);
#endif
                addr +=  EEPROM_PAGE_SIZE;
                pBuf += EEPROM_PAGE_SIZE;
            }
            /* write single */
            if(0 != number_of_single)
            {
                _EepromPageWrite(pBuf, addr, number_of_single);
#if (USE_SIM_IIC == 0)
                I2CDeviceWaitStandby(&I2CEepromHandle);
#else
                SimI2CWaitStandby(&SimI2CEepromHandle);
#endif
            }
        }
    }
}


void EepromRead(uint8_t *pBuf, uint16_t addr, uint16_t size)
{
#if (USE_SIM_IIC == 0)
    I2CEepromHandle.pRxBuf = pBuf;
    I2CEepromHandle.reg_addr = addr;
    I2CEepromHandle.rx_size = size;
    I2CRegRead(&I2CEepromHandle);
#else
    SimI2CEepromHandle.pRxBuf = pBuf;
    SimI2CEepromHandle.reg_addr = addr;
    SimI2CEepromHandle.rx_size = size;
    SimI2CRegRead(&SimI2CEepromHandle);
#endif
}

#define BUFFER_SIZE             0x100
#define WRITE_ADDR              0xF00
uint8_t i2c_buffer_write[BUFFER_SIZE];
uint8_t i2c_buffer_read[BUFFER_SIZE];
bool EepromTest()
{
    uint16_t i;
//    DevEepromCtlInit();

    for(i = 0; i < BUFFER_SIZE; i++)
    {
        i2c_buffer_write[i] = i;
    }

    EepromWrite(i2c_buffer_write, WRITE_ADDR, BUFFER_SIZE);
    LOG_INFO(LOG_TAG, "EEPROMWriteTest OK!!!");
    LOG_INFO_HEX(LOG_TAG, i2c_buffer_write, 256);

    EepromRead(i2c_buffer_read, WRITE_ADDR, BUFFER_SIZE);
    LOG_INFO(LOG_TAG, "EEPROMReadTest OK!!!");
    LOG_INFO_HEX(LOG_TAG, i2c_buffer_read, 256);

    for(i = 0; i < BUFFER_SIZE; i++)
    {
        if(i2c_buffer_write[i] != i2c_buffer_read[i])
        {
            LOG_ERROR(LOG_TAG, " EEPROM Test ERROR! ");
            return false;
        }
    }

    LOG_INFO(LOG_TAG, " EEPROM Test Success! ");
    return true;
}

MSH_CMD_EXPORT(EepromTest, -- -);



