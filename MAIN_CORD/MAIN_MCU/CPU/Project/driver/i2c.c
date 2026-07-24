/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        i2c.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2022-10-31
  * @brief       GD32F4xx
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

#include "i2c.h"




#define I2C_FLAG_TIMEOUT ((uint32_t)0x100000)

static void _I2CGpioInit(I2C_HandleType *pHandle)
{
    gpio_af_set(pHandle->i2c_cfg->scl_gpiox, pHandle->i2c_cfg->scl_alt_num, pHandle->i2c_cfg->scl_gpio_pinx);
    gpio_af_set(pHandle->i2c_cfg->sda_gpiox, pHandle->i2c_cfg->sda_alt_num, pHandle->i2c_cfg->sda_gpio_pinx);

    gpio_mode_set(pHandle->i2c_cfg->scl_gpiox, GPIO_MODE_AF, GPIO_PUPD_PULLUP, pHandle->i2c_cfg->scl_gpio_pinx);
    gpio_output_options_set(pHandle->i2c_cfg->scl_gpiox, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, pHandle->i2c_cfg->scl_gpio_pinx);
    gpio_mode_set(pHandle->i2c_cfg->sda_gpiox, GPIO_MODE_AF, GPIO_PUPD_PULLUP, pHandle->i2c_cfg->sda_gpio_pinx);
    gpio_output_options_set(pHandle->i2c_cfg->sda_gpiox, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, pHandle->i2c_cfg->sda_gpio_pinx);
}




void I2CInit(I2C_HandleType *pHandle)
{
    _I2CGpioInit(pHandle);
    rcu_periph_clock_enable(pHandle->i2c_cfg->rcu_i2c);
    /* I2C clock configure */
    i2c_clock_config(pHandle->i2c_cfg->i2c_peri, pHandle->i2c_cfg->speed, I2C_DTCY_2);
    /* I2C address configure */
    i2c_mode_addr_config(pHandle->i2c_cfg->i2c_peri, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, pHandle->slave_addr0);
    /* enable I2C */
    i2c_enable(pHandle->i2c_cfg->i2c_peri);
    /* enable acknowledge */
    i2c_ack_config(pHandle->i2c_cfg->i2c_peri, I2C_ACK_ENABLE);
}






bool I2CRegWrite(I2C_HandleType *pHandle)
{
    uint32_t timeout = I2C_FLAG_TIMEOUT;
    /* wait until I2C bus is idle */
    while(i2c_flag_get(pHandle->i2c_cfg->i2c_peri, I2C_FLAG_I2CBSY))
    {
        if(timeout-- == 0)
        {
            i2c_stop_on_bus(pHandle->i2c_cfg->i2c_peri);
            return false;
        }
    }

    /* send a start condition to I2C bus */
    i2c_start_on_bus(pHandle->i2c_cfg->i2c_peri);
    /* wait until SBSEND bit is set */
    timeout = I2C_FLAG_TIMEOUT;
    while(!i2c_flag_get(pHandle->i2c_cfg->i2c_peri, I2C_FLAG_SBSEND))
    {
        if(timeout-- == 0)
        {
            i2c_stop_on_bus(pHandle->i2c_cfg->i2c_peri);
            return false;
        }
    }

    /* send slave address to I2C bus */
    i2c_master_addressing(pHandle->i2c_cfg->i2c_peri, pHandle->slave_addr0, I2C_TRANSMITTER);
    /* wait until ADDSEND bit is set */
    timeout = I2C_FLAG_TIMEOUT;
    while(!i2c_flag_get(pHandle->i2c_cfg->i2c_peri, I2C_FLAG_ADDSEND))
    {
        if(timeout-- == 0)
        {
            i2c_stop_on_bus(pHandle->i2c_cfg->i2c_peri);
            return false;
        }
    }

    /* clear the ADDSEND bit */
    i2c_flag_clear(pHandle->i2c_cfg->i2c_peri, I2C_FLAG_ADDSEND);
    /* wait until the transmit data buffer is empty */
    timeout = I2C_FLAG_TIMEOUT;
    while(SET != i2c_flag_get(pHandle->i2c_cfg->i2c_peri, I2C_FLAG_TBE))
    {
        if(timeout-- == 0)
        {
            i2c_stop_on_bus(pHandle->i2c_cfg->i2c_peri);
            return false;
        }
    }

    if(pHandle->reg_addr_width == EN_I2C_REG_ADDR_WIDTH_16)
    {
        /* send reg address high bits */
        i2c_data_transmit(pHandle->i2c_cfg->i2c_peri, (pHandle->reg_addr >> 8));
        /* wait until BTC bit is set */
        timeout = I2C_FLAG_TIMEOUT;
        while(!i2c_flag_get(pHandle->i2c_cfg->i2c_peri, I2C_FLAG_BTC))
        {
            if(timeout-- == 0)
            {
                i2c_stop_on_bus(pHandle->i2c_cfg->i2c_peri);
                return false;
            }
        }
    }

    /* send reg address  */
    i2c_data_transmit(pHandle->i2c_cfg->i2c_peri, pHandle->reg_addr);
    /* wait until BTC bit is set */
    timeout = I2C_FLAG_TIMEOUT;
    while(!i2c_flag_get(pHandle->i2c_cfg->i2c_peri, I2C_FLAG_BTC))
    {
        if(timeout-- == 0)
        {
            i2c_stop_on_bus(pHandle->i2c_cfg->i2c_peri);
            return false;
        }
    }

    /* send the bytes to be written */
    for(uint32_t i = 0; i < pHandle->tx_size; i++)
    {
        i2c_data_transmit(pHandle->i2c_cfg->i2c_peri, ((uint8_t *)(pHandle->pTxBuf))[i]);
        /* wait until BTC bit is set */
        timeout = I2C_FLAG_TIMEOUT;
        while(!i2c_flag_get(pHandle->i2c_cfg->i2c_peri, I2C_FLAG_BTC))
        {
            if(timeout-- == 0)
            {
                i2c_stop_on_bus(pHandle->i2c_cfg->i2c_peri);
                return false;
            }
        }
    }

    /* send a stop condition to I2C bus */
    i2c_stop_on_bus(pHandle->i2c_cfg->i2c_peri);
    /* wait until the stop condition is finished */
    timeout = I2C_FLAG_TIMEOUT;
    while(I2C_CTL0(pHandle->i2c_cfg->i2c_peri) & I2C_CTL0_STOP)
    {
        if(timeout-- == 0)
        {
            i2c_stop_on_bus(pHandle->i2c_cfg->i2c_peri);
            return false;
        }
    }

    return true;
}




bool I2CRegRead(I2C_HandleType *pHandle)
{
    uint32_t timeout = I2C_FLAG_TIMEOUT;
    uint32_t rd_timeout = I2C_FLAG_TIMEOUT;
    uint32_t number_of_byte = pHandle->rx_size;
    uint8_t *p_buffer = (uint8_t *)pHandle->pRxBuf;

    /* wait until I2C bus is idle */
    while(i2c_flag_get(pHandle->i2c_cfg->i2c_peri, I2C_FLAG_I2CBSY))
    {
        if(timeout-- == 0)
        {
            i2c_stop_on_bus(pHandle->i2c_cfg->i2c_peri);
            return false;
        }
    }

    if(2 == number_of_byte)
    {
        i2c_ackpos_config(pHandle->i2c_cfg->i2c_peri, I2C_ACKPOS_NEXT);
    }

    /* send a start condition to I2C bus */
    i2c_start_on_bus(pHandle->i2c_cfg->i2c_peri);
    /* wait until SBSEND bit is set */
    timeout = I2C_FLAG_TIMEOUT;
    while(!i2c_flag_get(pHandle->i2c_cfg->i2c_peri, I2C_FLAG_SBSEND))
    {
        if(timeout-- == 0)
        {
            i2c_stop_on_bus(pHandle->i2c_cfg->i2c_peri);
            i2c_ack_config(pHandle->i2c_cfg->i2c_peri, I2C_ACK_ENABLE);
            i2c_ackpos_config(pHandle->i2c_cfg->i2c_peri, I2C_ACKPOS_CURRENT);
            return false;
        }
    }

    /* send slave address to I2C bus */
    i2c_master_addressing(pHandle->i2c_cfg->i2c_peri, pHandle->slave_addr0, I2C_TRANSMITTER);
    /* wait until ADDSEND bit is set */
    timeout = I2C_FLAG_TIMEOUT;
    while(!i2c_flag_get(pHandle->i2c_cfg->i2c_peri, I2C_FLAG_ADDSEND))
    {
        if(timeout-- == 0)
        {
            i2c_stop_on_bus(pHandle->i2c_cfg->i2c_peri);
            i2c_ack_config(pHandle->i2c_cfg->i2c_peri, I2C_ACK_ENABLE);
            i2c_ackpos_config(pHandle->i2c_cfg->i2c_peri, I2C_ACKPOS_CURRENT);
            return false;
        }
    }

    /* clear the ADDSEND bit */
    i2c_flag_clear(pHandle->i2c_cfg->i2c_peri, I2C_FLAG_ADDSEND);
    /* wait until the transmit data buffer is empty */
    timeout = I2C_FLAG_TIMEOUT;
    while(SET != i2c_flag_get(pHandle->i2c_cfg->i2c_peri, I2C_FLAG_TBE))
    {
        if(timeout-- == 0)
        {
            i2c_stop_on_bus(pHandle->i2c_cfg->i2c_peri);
            i2c_ack_config(pHandle->i2c_cfg->i2c_peri, I2C_ACK_ENABLE);
            i2c_ackpos_config(pHandle->i2c_cfg->i2c_peri, I2C_ACKPOS_CURRENT);
            return false;
        }
    }

    /* enable I2C*/
    i2c_enable(pHandle->i2c_cfg->i2c_peri);

    if(pHandle->reg_addr_width == EN_I2C_REG_ADDR_WIDTH_16)
    {
        /* send the reg address to write to */
        i2c_data_transmit(pHandle->i2c_cfg->i2c_peri, (pHandle->reg_addr >> 8));
        /* wait until BTC bit is set */
        timeout = I2C_FLAG_TIMEOUT;
        while(!i2c_flag_get(pHandle->i2c_cfg->i2c_peri, I2C_FLAG_BTC))
        {
            if(timeout-- == 0)
            {
                i2c_stop_on_bus(pHandle->i2c_cfg->i2c_peri);
                i2c_ack_config(pHandle->i2c_cfg->i2c_peri, I2C_ACK_ENABLE);
                i2c_ackpos_config(pHandle->i2c_cfg->i2c_peri, I2C_ACKPOS_CURRENT);
                return false;
            }
        }
    }

    /* send the reg address to write to */
    i2c_data_transmit(pHandle->i2c_cfg->i2c_peri, pHandle->reg_addr);
    /* wait until BTC bit is set */
    timeout = I2C_FLAG_TIMEOUT;
    while(!i2c_flag_get(pHandle->i2c_cfg->i2c_peri, I2C_FLAG_BTC))
    {
        if(timeout-- == 0)
        {
            i2c_stop_on_bus(pHandle->i2c_cfg->i2c_peri);
            i2c_ack_config(pHandle->i2c_cfg->i2c_peri, I2C_ACK_ENABLE);
            i2c_ackpos_config(pHandle->i2c_cfg->i2c_peri, I2C_ACKPOS_CURRENT);
            return false;
        }
    }

    /* send a start condition to I2C bus */
    i2c_start_on_bus(pHandle->i2c_cfg->i2c_peri);
    /* wait until SBSEND bit is set */
    timeout = I2C_FLAG_TIMEOUT;
    while(!i2c_flag_get(pHandle->i2c_cfg->i2c_peri, I2C_FLAG_SBSEND))
    {
        if(timeout-- == 0)
        {
            i2c_stop_on_bus(pHandle->i2c_cfg->i2c_peri);
            i2c_ack_config(pHandle->i2c_cfg->i2c_peri, I2C_ACK_ENABLE);
            i2c_ackpos_config(pHandle->i2c_cfg->i2c_peri, I2C_ACKPOS_CURRENT);
            return false;
        }
    }

    /* send slave address to I2C bus */
    i2c_master_addressing(pHandle->i2c_cfg->i2c_peri, pHandle->slave_addr0, I2C_RECEIVER);

    if(number_of_byte < 3)
    {
        /* disable acknowledge */
        i2c_ack_config(pHandle->i2c_cfg->i2c_peri, I2C_ACK_DISABLE);
    }

    /* wait until ADDSEND bit is set */
    timeout = I2C_FLAG_TIMEOUT;
    while(!i2c_flag_get(pHandle->i2c_cfg->i2c_peri, I2C_FLAG_ADDSEND))
    {
        if(timeout-- == 0)
        {
            i2c_stop_on_bus(pHandle->i2c_cfg->i2c_peri);
            i2c_ack_config(pHandle->i2c_cfg->i2c_peri, I2C_ACK_ENABLE);
            i2c_ackpos_config(pHandle->i2c_cfg->i2c_peri, I2C_ACKPOS_CURRENT);
            return false;
        }
    }

    /* clear the ADDSEND bit */
    i2c_flag_clear(pHandle->i2c_cfg->i2c_peri, I2C_FLAG_ADDSEND);

    if(1 == number_of_byte)
    {
        /* send a stop condition to I2C bus */
        i2c_stop_on_bus(pHandle->i2c_cfg->i2c_peri);
    }

    while(number_of_byte)
    {
        if(3 == number_of_byte)
        {
            /* wait until BTC bit is set */
            timeout = I2C_FLAG_TIMEOUT;
            while(!i2c_flag_get(pHandle->i2c_cfg->i2c_peri, I2C_FLAG_BTC))
            {
                if(timeout-- == 0)
                {
                    i2c_stop_on_bus(pHandle->i2c_cfg->i2c_peri);
                    i2c_ack_config(pHandle->i2c_cfg->i2c_peri, I2C_ACK_ENABLE);
                    i2c_ackpos_config(pHandle->i2c_cfg->i2c_peri, I2C_ACKPOS_CURRENT);
                    return false;
                }
            }
            /* disable acknowledge */
            i2c_ack_config(pHandle->i2c_cfg->i2c_peri, I2C_ACK_DISABLE);
        }
        if(2 == number_of_byte)
        {
            /* wait until BTC bit is set */
            timeout = I2C_FLAG_TIMEOUT;
            while(!i2c_flag_get(pHandle->i2c_cfg->i2c_peri, I2C_FLAG_BTC))
            {
                if(timeout-- == 0)
                {
                    i2c_stop_on_bus(pHandle->i2c_cfg->i2c_peri);
                    i2c_ack_config(pHandle->i2c_cfg->i2c_peri, I2C_ACK_ENABLE);
                    i2c_ackpos_config(pHandle->i2c_cfg->i2c_peri, I2C_ACKPOS_CURRENT);
                    return false;
                }
            }
            /* send a stop condition to I2C bus */
            i2c_stop_on_bus(pHandle->i2c_cfg->i2c_peri);
        }
        /* wait until the RBNE bit is set and clear it */
        if(i2c_flag_get(pHandle->i2c_cfg->i2c_peri, I2C_FLAG_RBNE))
        {
            /* read a byte from the EEPROM */
            *p_buffer = i2c_data_receive(pHandle->i2c_cfg->i2c_peri);
            /* point to the next location where the byte read will be saved */
            p_buffer++;
            /* decrement the read bytes counter */
            number_of_byte--;
            rd_timeout = I2C_FLAG_TIMEOUT;
        }
        else
        {
            if(rd_timeout-- == 0)
            {
                i2c_stop_on_bus(pHandle->i2c_cfg->i2c_peri);
                i2c_ack_config(pHandle->i2c_cfg->i2c_peri, I2C_ACK_ENABLE);
                i2c_ackpos_config(pHandle->i2c_cfg->i2c_peri, I2C_ACKPOS_CURRENT);
                return false;
            }
        }
    }


    /* wait until the stop condition is finished */
    timeout = I2C_FLAG_TIMEOUT;
    while(I2C_CTL0(pHandle->i2c_cfg->i2c_peri) & I2C_CTL0_STOP)
    {
        if(timeout-- == 0)
        {
            i2c_stop_on_bus(pHandle->i2c_cfg->i2c_peri);
            i2c_ack_config(pHandle->i2c_cfg->i2c_peri, I2C_ACK_ENABLE);
            i2c_ackpos_config(pHandle->i2c_cfg->i2c_peri, I2C_ACKPOS_CURRENT);
            return false;
        }
    }

    /* enable acknowledge */
    i2c_ack_config(pHandle->i2c_cfg->i2c_peri, I2C_ACK_ENABLE);
    i2c_ackpos_config(pHandle->i2c_cfg->i2c_peri, I2C_ACKPOS_CURRENT);

    return true;
}


void I2CDeviceWaitStandby(I2C_HandleType *pHandle)
{
    __IO uint32_t val = 0;

    while(1)
    {
        /* wait until I2C bus is idle */
        while(i2c_flag_get(pHandle->i2c_cfg->i2c_peri, I2C_FLAG_I2CBSY));

        /* send a start condition to I2C bus */
        i2c_start_on_bus(pHandle->i2c_cfg->i2c_peri);

        /* wait until SBSEND bit is set */
        while(!i2c_flag_get(pHandle->i2c_cfg->i2c_peri, I2C_FLAG_SBSEND));

        /* send slave address to I2C bus */
        i2c_master_addressing(pHandle->i2c_cfg->i2c_peri, pHandle->slave_addr0, I2C_TRANSMITTER);

        /* keep looping till the Address is acknowledged or the AE flag is set (address not acknowledged at time) */
        do
        {
            /* get the current value of the I2C_STAT0 register */
            val = I2C_STAT0(pHandle->i2c_cfg->i2c_peri);

        }
        while(0 == (val & (I2C_STAT0_ADDSEND | I2C_STAT0_AERR)));

        /* check if the ADDSEND flag has been set */
        if(val & I2C_STAT0_ADDSEND)
        {

            /* clear ADDSEND flag */
            i2c_flag_clear(pHandle->i2c_cfg->i2c_peri, I2C_FLAG_ADDSEND);

            /* send a stop condition to I2C bus */
            i2c_stop_on_bus(pHandle->i2c_cfg->i2c_peri);

            /* exit the function */
            return ;
        }
        else
        {
            /* clear the bit of AERR */
            i2c_flag_clear(pHandle->i2c_cfg->i2c_peri, I2C_FLAG_AERR);
        }

        /* send a stop condition to I2C bus */
        i2c_stop_on_bus(pHandle->i2c_cfg->i2c_peri);
        /* wait until the stop condition is finished */
        while(I2C_CTL0(pHandle->i2c_cfg->i2c_peri)&I2C_CTL0_STOP);
    }
}










