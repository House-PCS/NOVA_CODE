/********************************** (C) COPYRIGHT *****************************
 * File Name          : CH390_Interface.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2024/08/20
 * Description        : CH390 interface
 ******************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 ******************************************************************************/
#include "CH390.h"
#include "CH390_Interface.h"

#define ch390_cs(value)  GPIO_WritePin(66, value);
#define ch390_rst(value) GPIO_WritePin(67, value);

/**
 * @name ch390_get_int_pin
 * @brief Get ch390 interrupt signal
 */
uint16_t ch390_get_int_pin(void)
{
//    return GPIO_ReadPin(25);
    return GPIO_ReadPin(68);
}

/**
 * @name ch390_delay_us
 * @brief Delay a few microseconds
 */
void ch390_delay_us(uint32_t time)
{
    DELAY_US(time);
}

/**
 * @name ch390_hardware_reset
 * @brief Hardware reset CH390 by pull down RSTB pin
 */
void ch390_hardware_reset()
{
    ch390_rst(0);
    ch390_delay_us(10);
    ch390_rst(1);
    ch390_delay_us(10000);
}

static uint8_t ch390_spi_exchange_byte(uint8_t byte)
{
    uint16_t Readtemp;
    while(SpibRegs.SPIFFTX.bit.TXFFST != 0)
    {

    }
    SpibRegs.SPITXBUF = (uint8_t)(byte << 8);

    while(SpibRegs.SPISTS.bit.INT_FLAG == 0)
    {

    }
    Readtemp = SpibRegs.SPIRXBUF;

    return  (Readtemp);
}

#define ch390_spi_dummy_read() ch390_spi_exchange_byte(0)

/***************************************************************************
 * The following are the basic CH390 read/write operation functions.
 */

/**
 * @name ch390_read_reg
 * @brief Read register
 * @param reg - Target register address
 * @return Register value
 */
uint8_t ch390_read_reg(uint8_t reg)
{
    uint8_t spi_data;
    ch390_cs(0); // CS = 0
    ch390_spi_exchange_byte(reg | OPC_REG_R);
    spi_data = ch390_spi_dummy_read();
    ch390_cs(1); // CS = 1
    return spi_data;
}

/**
 * @name ch390_write_reg
 * @brief Write register
 * @param reg - Target register address
 * @param value - Value to be written
 */
void ch390_write_reg(uint8_t reg, uint8_t value)
{
    ch390_cs(0); // CS = 0
    ch390_spi_exchange_byte(reg | OPC_REG_W);
    ch390_spi_exchange_byte(value);
    ch390_cs(1); // CS = 1
    return;
}
/**
 * @name ch390_read_mem
 * @brief Read data from RX SRAM
 * @param data - Data buffer
 * @param length - Length to read
 */
void ch390_read_mem(uint8_t *data, int length)
{
    int i;
    ch390_cs(0);
    ch390_spi_exchange_byte(OPC_MEM_READ);

    for (i = 0; i < length; i++)
    {
        data[i] = ch390_spi_dummy_read();
    }
    ch390_cs(1);
}
/**
 * @name ch390_write_mem
 * @brief Write data to TX SRAM
 * @param data - Data buffer
 * @param length - Length to write
 */
void ch390_write_mem(uint8_t *data, int length)
{
    int i;
    ch390_cs(0);
    ch390_spi_exchange_byte(OPC_MEM_WRITE);
    for (i = 0; i < length; i++)
    {
        ch390_spi_exchange_byte(data[i]);
    }
    ch390_cs(1);
}
