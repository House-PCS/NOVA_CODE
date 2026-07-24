/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_24c08.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-06-19
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-06-19    <td>V1.0.0    <td>           <td>InitialVersion 
  * </table>
  ******************************************************************************
  */


#include "dev_24c32.h"
#include "systick.h"

#define USE_SIM_IIC     0

#define EEPROM_DEVICE_ADDR  0xA0
#define EEPROM_PAGE_SIZE    32


/**
  * @brief   EEPROM初始化
  * @param   none
  * @return  NULL
  */
void DevEepromCtlInit()
{
#if USE_SIM_IIC == 0
    gEepromI2CHandle.slave_addr0 = EEPROM_DEVICE_ADDR;
    gEepromI2CHandle.reg_addr_width = EN_I2C_REG_ADDR_WIDTH_16;
#else
    gEepromSimI2CHandle.slave_addr0 = EEPROM_DEVICE_ADDR;
    gEepromSimI2CHandle.reg_addr_width = EN_SIMI2C_REG_ADDR_WIDTH_16;
#endif
    E2promBspInit();
}

/**
  * @brief   EEPROM页写入
  * @param   pBuf：写Buf指针，addr：写地址，size：写入长度
  * @return  NULL
  */
static void _EepromPageWrite(uint8_t *pBuf, uint16_t addr, uint16_t size)
{
#if USE_SIM_IIC == 0
    gEepromI2CHandle.pTxBuf = pBuf;
    gEepromI2CHandle.reg_addr = addr;
    gEepromI2CHandle.tx_size = size;
    I2CRegWrite(&gEepromI2CHandle);
#else
    gEepromSimI2CHandle.pTxBuf = pBuf;
    gEepromSimI2CHandle.reg_addr = addr;
    gEepromSimI2CHandle.tx_size = size;
    SimI2CRegWrite(&gEepromSimI2CHandle);
#endif
}


/**
  * @brief   EEPROM写入(简化版)
  * @param   pBuf：写Buf指针，addr：写地址，size：写入长度
  * @return  NULL
  */
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
        I2CDeviceWaitStandby(&gEepromI2CHandle);
#else
        SimI2CWaitStandby(&gEepromSimI2CHandle);        
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
                I2CDeviceWaitStandby(&gEepromI2CHandle);
#else
                SimI2CWaitStandby(&gEepromSimI2CHandle);                 
#endif
                addr += EEPROM_PAGE_SIZE;
                pBuf += EEPROM_PAGE_SIZE;
            }
        }
        /*最后一个page*/
        _EepromPageWrite(pBuf, addr, last_offset + 1);
#if (USE_SIM_IIC == 0)
        I2CDeviceWaitStandby(&gEepromI2CHandle);
#else
        SimI2CWaitStandby(&gEepromSimI2CHandle);         
#endif
    }
    else
    {
        _EepromPageWrite(pBuf, addr, size);
#if (USE_SIM_IIC == 0)
        I2CDeviceWaitStandby(&gEepromI2CHandle);
#else
        SimI2CWaitStandby(&gEepromSimI2CHandle);         
#endif
    }

}

/**
  * @brief   EEPROM写入(原版)
  * @param   pBuf：写Buf指针，addr：写地址，size：写入长度
  * @return  NULL
  */
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
            I2CDeviceWaitStandby(&gEepromI2CHandle);
#else
            SimI2CWaitStandby(&gEepromSimI2CHandle);  
#endif
            addr +=  EEPROM_PAGE_SIZE;
            pBuf += EEPROM_PAGE_SIZE;
        }
        if(0 != number_of_single)
        {
            _EepromPageWrite(pBuf, addr, number_of_single);
#if (USE_SIM_IIC == 0)
            I2CDeviceWaitStandby(&gEepromI2CHandle);
#else
            SimI2CWaitStandby(&gEepromSimI2CHandle);  
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
            I2CDeviceWaitStandby(&gEepromI2CHandle);
#else
            SimI2CWaitStandby(&gEepromSimI2CHandle);  
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
                I2CDeviceWaitStandby(&gEepromI2CHandle);
#else
                SimI2CWaitStandby(&gEepromSimI2CHandle);  
#endif
                addr += count;
                pBuf += count;
            }
            /* write page */
            while(number_of_page--)
            {
                _EepromPageWrite(pBuf, addr, EEPROM_PAGE_SIZE);
#if (USE_SIM_IIC == 0)
                I2CDeviceWaitStandby(&gEepromI2CHandle);
#else
                SimI2CWaitStandby(&gEepromSimI2CHandle);  
#endif
                addr +=  EEPROM_PAGE_SIZE;
                pBuf += EEPROM_PAGE_SIZE;
            }
            /* write single */
            if(0 != number_of_single)
            {
                _EepromPageWrite(pBuf, addr, number_of_single);
#if (USE_SIM_IIC == 0)
                I2CDeviceWaitStandby(&gEepromI2CHandle);
#else
                SimI2CWaitStandby(&gEepromSimI2CHandle);  
#endif
            }
        }
    }
}

/**
  * @brief   EEPROM读取
  * @param   pBuf：读Buf指针，addr：读地址，size：读取长度
  * @return  NULL
  */
void EepromRead(uint8_t *pBuf, uint16_t addr, uint16_t size)
{
#if (USE_SIM_IIC == 0)
    gEepromI2CHandle.pRxBuf = pBuf;
    gEepromI2CHandle.reg_addr = addr;
    gEepromI2CHandle.rx_size = size;
    I2CRegRead(&gEepromI2CHandle);
#else
    gEepromSimI2CHandle.pRxBuf = pBuf;
    gEepromSimI2CHandle.reg_addr = addr;
    gEepromSimI2CHandle.rx_size = size;
    SimI2CRegRead(&gEepromSimI2CHandle);
#endif
}








