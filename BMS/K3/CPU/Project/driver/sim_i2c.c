/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        sim_i2c.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2022-10-31
  * @brief       GD32F30x
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2022-10-31    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "sim_i2c.h"
#define BQHWI2C      1
#define USE_SIM_IIC  1

void SimI2CInit(SimI2C_HandleType *pHandle)
{
    gpio_init(pHandle->simi2c_cfg->scl_gpiox, GPIO_MODE_OUT_OD, GPIO_OSPEED_2MHZ, pHandle->simi2c_cfg->scl_gpio_pinx);
    gpio_init(pHandle->simi2c_cfg->sda_gpiox, GPIO_MODE_OUT_OD, GPIO_OSPEED_2MHZ, pHandle->simi2c_cfg->sda_gpio_pinx);
}

static void _SimI2CSDASet(SimI2C_HandleType *pHandle)
{
    gpio_bit_set(pHandle->simi2c_cfg->sda_gpiox, pHandle->simi2c_cfg->sda_gpio_pinx);
}

static void _SimI2CSDAReset(SimI2C_HandleType *pHandle)
{
    gpio_bit_reset(pHandle->simi2c_cfg->sda_gpiox, pHandle->simi2c_cfg->sda_gpio_pinx);
}


static void _SimI2CSCLSet(SimI2C_HandleType *pHandle)
{
    gpio_bit_set(pHandle->simi2c_cfg->scl_gpiox, pHandle->simi2c_cfg->scl_gpio_pinx);
}

static void _SimI2CSCLReset(SimI2C_HandleType *pHandle)
{
    gpio_bit_reset(pHandle->simi2c_cfg->scl_gpiox, pHandle->simi2c_cfg->scl_gpio_pinx);
}

static uint8_t _SimI2CSDARead(SimI2C_HandleType *pHandle)
{
    uint8_t ret = 0xff;

    ret = gpio_input_bit_get(pHandle->simi2c_cfg->sda_gpiox, pHandle->simi2c_cfg->sda_gpio_pinx);
//    if(ret != 0)
//    {
//        ret = 1;
//    }
    return ret;
}

static void _SimI2CSDADir()
{

}


static void _SimI2CDelay(SimI2C_HandleType *pHandle)
{
    int32_t n_utime = 29;
//    DelayUs(10);
    while(n_utime--);
}


static bool _SimI2CStart(SimI2C_HandleType *pHandle)
{
    bool status = true;
    _SimI2CSDASet(pHandle);
    _SimI2CSCLSet(pHandle);
    _SimI2CDelay(pHandle);

    _SimI2CSDAReset(pHandle);
    _SimI2CDelay(pHandle);
    _SimI2CSCLReset(pHandle);
    _SimI2CDelay(pHandle);
    return status;
}

static bool _SimI2CStop(SimI2C_HandleType *pHandle)
{
    bool status = true;
    _SimI2CSCLReset(pHandle);
    _SimI2CDelay(pHandle);
    _SimI2CSDAReset(pHandle);
    _SimI2CDelay(pHandle);

    _SimI2CSCLSet(pHandle);
    _SimI2CDelay(pHandle);
    _SimI2CSDASet(pHandle);
    _SimI2CDelay(pHandle);
    return status;
}

static bool _SimI2CWaitAck(SimI2C_HandleType *pHandle)
{
    bool status = true;
    _SimI2CSDASet(pHandle);
    _SimI2CDelay(pHandle);
    _SimI2CSCLSet(pHandle);
    _SimI2CDelay(pHandle);
    if(_SimI2CSDARead(pHandle) == 1)
    {
        _SimI2CStop(pHandle);
        status = false;
        return status;
    }
    _SimI2CSCLReset(pHandle);
    _SimI2CDelay(pHandle);
    return status;
}

static void _SimI2CSendAck(SimI2C_HandleType *pHandle)
{
    _SimI2CSDAReset(pHandle);
    _SimI2CDelay(pHandle);
    _SimI2CSCLSet(pHandle);
    _SimI2CDelay(pHandle);
    _SimI2CSCLReset(pHandle);
    _SimI2CDelay(pHandle);
//    SimI2CSDASet(pHandle);
}

static void _SimI2CSendNoAck(SimI2C_HandleType *pHandle)
{
    _SimI2CSDASet(pHandle);
    _SimI2CDelay(pHandle);
    _SimI2CSCLSet(pHandle);
    _SimI2CDelay(pHandle);
    _SimI2CSCLReset(pHandle);
    _SimI2CDelay(pHandle);
//    SimI2CSDASet(pHandle);
}


void SimI2CSendData(SimI2C_HandleType *pHandle, uint8_t value)
{
    uint8_t i;

    for(i = 0; i < 8; i++)
    {
        _SimI2CSCLReset(pHandle);
//        _SimI2CDelay(pHandle);/*4分之1周期*/
        if(value & 0x80)
        {
            _SimI2CSDASet(pHandle);
        }
        else
        {
            _SimI2CSDAReset(pHandle);
        }
        _SimI2CDelay(pHandle);
        _SimI2CSCLSet(pHandle);
        _SimI2CDelay(pHandle);
        _SimI2CDelay(pHandle);
        _SimI2CSCLReset(pHandle);
        value <<= 1;
    }
}


uint8_t SimI2CRevData(SimI2C_HandleType *pHandle)
{
    uint8_t i = 0;
    uint8_t data = 0;

    _SimI2CSDASet(pHandle);
    for(i = 0; i < 8; i++)
    {
        data <<= 1;
        _SimI2CSCLReset(pHandle);
        _SimI2CDelay(pHandle);
        _SimI2CSCLSet(pHandle);
        _SimI2CDelay(pHandle);
        if(_SimI2CSDARead(pHandle) == 1)
        {
            data |= 0x01;
        }
    }
    _SimI2CSCLReset(pHandle);

    return data;
}


/*应用*/


bool SimI2CRegWrite(SimI2C_HandleType *pHandle)
{
    int i = 0;
    uint8_t *p = (uint8_t *)pHandle->pTxBuf;
    uint16_t size = pHandle->tx_size;

    _SimI2CStart(pHandle);
    SimI2CSendData(pHandle, pHandle->slave_addr0);
    if(_SimI2CWaitAck(pHandle) == false)
    {
        _SimI2CStop(pHandle);
        return false;
    }

    if(pHandle->reg_addr_width == EN_SIMI2C_REG_ADDR_WIDTH_16)
    {
        SimI2CSendData(pHandle, (pHandle->reg_addr >> 8));
        if(_SimI2CWaitAck(pHandle) == false)
        {
            _SimI2CStop(pHandle);
            return false;
        }
    }

    SimI2CSendData(pHandle, pHandle->reg_addr);
    if(_SimI2CWaitAck(pHandle) == false)
    {
        _SimI2CStop(pHandle);
        return false;
    }
    for(i = 0; i < size; i++)
    {
        SimI2CSendData(pHandle, *p);
        if(_SimI2CWaitAck(pHandle) == false)
        {
            _SimI2CStop(pHandle);
            return false;
        }
        p++;
    }
    _SimI2CStop(pHandle);
    return true;
}


bool SimI2CRegRead(SimI2C_HandleType *pHandle)
{
    int i = 0;
    uint8_t *p = pHandle->pRxBuf;
    uint16_t size = pHandle->rx_size;

    _SimI2CStart(pHandle);
    SimI2CSendData(pHandle, pHandle->slave_addr0);
    if(_SimI2CWaitAck(pHandle) == false)
    {
        _SimI2CStop(pHandle);
        return false;
    }

    if(pHandle->reg_addr_width == EN_SIMI2C_REG_ADDR_WIDTH_16)
    {
        SimI2CSendData(pHandle, (pHandle->reg_addr >> 8));
        if(_SimI2CWaitAck(pHandle) == false)
        {
            _SimI2CStop(pHandle);
            return false;
        }
    }

    SimI2CSendData(pHandle, pHandle->reg_addr);
    if(_SimI2CWaitAck(pHandle) == false)
    {
        _SimI2CStop(pHandle);
        return false;
    }

    _SimI2CStart(pHandle);
    SimI2CSendData(pHandle, pHandle->slave_addr0 | 0x01);
    if(_SimI2CWaitAck(pHandle) == false)
    {
        _SimI2CStop(pHandle);
        return false;
    }

    for(i = 0; i < size - 1; i++)
    {
        *p = SimI2CRevData(pHandle);
        _SimI2CSendAck(pHandle);
        p++;
    }
    *p = SimI2CRevData(pHandle);
    _SimI2CSendNoAck(pHandle);

    _SimI2CStop(pHandle);
    return true;
}
bool SimI2CWaitStandby(SimI2C_HandleType *pHandle)
{
    uint32_t timeout = 0xFFFF;
    do
    {
        _SimI2CStart(pHandle);
        SimI2CSendData(pHandle, pHandle->slave_addr0);
        timeout--;
    }
    while((timeout != 0) && (_SimI2CSDARead(pHandle) != 0));
    soft_delayms(10);
    return true;
}















