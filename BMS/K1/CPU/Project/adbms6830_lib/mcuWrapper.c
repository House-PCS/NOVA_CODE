/*******************************************************************************
Copyright (c) 2020 - Analog Devices Inc. All Rights Reserved.
This software is proprietary & confidential to Analog Devices, Inc.
and its licensor.
******************************************************************************
* @file:    mcuWrapper.c
* @brief:   BMS SPI driver functions
* @version: $Revision$
* @date:    $Date$
* Developed by: ADIBMS Software team, Bangalore, India
*****************************************************************************/
/*! \addtogroup MCU DRIVER
*  @{
*/

/*! @addtogroup Mcu Driver
*  @{
*/

#include "mcuWrapper.h"


static AFEInComingApi_t _gAFEInComingApi;
/**
 *******************************************************************************
 * Function: ADBMS6830_lib初始化
 * @brief 向该库传入通信接口
 *
 * @details 
 *
 * Parameters:
 * @param [in]  AFEInComingApi_t：通信接口API组合
 *
 * @return None
 *
 *******************************************************************************
*/
void ADBMS6830LibInit(AFEInComingApi_t *p_api)
{
	_gAFEInComingApi.p_delay_us = p_api->p_delay_us;
	_gAFEInComingApi.p_delay_ms = p_api->p_delay_ms;
	_gAFEInComingApi.p_spi_cs_low = p_api->p_spi_cs_low;
	_gAFEInComingApi.p_spi_cs_high = p_api->p_spi_cs_high;
	_gAFEInComingApi.p_spi_write_read = p_api->p_spi_write_read;
	_gAFEInComingApi.p_spi_read = p_api->p_spi_read;
	_gAFEInComingApi.p_spi_write = p_api->p_spi_write;
}




/**
 *******************************************************************************
 * Function: Delay_ms
 * @brief Delay mili second
 *
 * @details This function insert delay in ms.
 *     
 * Parameters:
 * @param [in]  delay   Delay_ms
 *
 * @return None
 *
 *******************************************************************************
*/
void Adbms6830DelayMs(uint32_t delay)
{
    _gAFEInComingApi.p_delay_ms(delay);
}


/**
 *******************************************************************************
 * Function: _DelayUs
 * @brief Delay u second
 *
 * @details This function insert delay in us.
 *
 * Parameters:
 * @param [in]  delay   Delay_us
 *
 * @return None
 *
 *******************************************************************************
*/
void Adbms6830DelayUs(uint32_t delay)
{
    _gAFEInComingApi.p_delay_us(delay);
}

/**
 *******************************************************************************
 * Function: adBmsCsLow
 * @brief Select chip select low
 *
 * @details This function does spi chip select low.
 *
 * @return None
 *
 *******************************************************************************
*/
void adBmsCsLow()
{
    _gAFEInComingApi.p_spi_cs_low();
}

/**
 *******************************************************************************
 * Function: adBmsCsHigh
 * @brief Select chip select High
 *
 * @details This function does spi chip select high.
 *
 * @return None
 *
 *******************************************************************************
*/
void adBmsCsHigh()
{
    _gAFEInComingApi.p_spi_cs_high();
}

/**
 *******************************************************************************
 * Function: spiWriteBytes
 * @brief Writes an array of bytes out of the SPI port.
 *
 * @details This function wakeup bms ic in IsoSpi mode send dumy byte data in spi line..
 *
 * @param [in]  size            Numberof bytes to be send on the SPI line
 *
 * @param [in]  *tx_Data    Tx data pointer 
 *
 * @return None
 *
 *******************************************************************************
*/
void spiWriteBytes
( 
uint16_t size,                     /*Option: Number of bytes to be written on the SPI port*/
uint8_t *tx_Data                       /*Array of bytes to be written on the SPI port*/
)
{
    _gAFEInComingApi.p_spi_write(size, tx_Data);
}

/**
 *******************************************************************************
 * Function: spiWriteReadBytes
 * @brief Writes and read a set number of bytes using the SPI port.
 *
 * @details This function writes and read a set number of bytes using the SPI port.
 *
 * @param [in]  *tx_data    Tx data pointer
 *
 * @param [in]  *rx_data    Rx data pointer 
 *
 * @param [in]  size            Data size 
 *
 * @return None
 *
 *******************************************************************************
*/
void spiWriteReadBytes
(
uint8_t *tx_data,                   /*array of data to be written on SPI port*/
uint8_t *rx_data,                   /*Input: array that will store the data read by the SPI port*/
uint16_t size                           /*Option: number of bytes*/
)
{
    _gAFEInComingApi.p_spi_write_read(tx_data, rx_data, size);
}

/**
 *******************************************************************************
 * Function: spiReadBytes
 * @brief Read number of bytes using the SPI port.
 *
 * @details This function Read a set number of bytes using the SPI port.
 *
 * @param [in]  size            Data size 
 *
 * @param [in]  *rx_data    Rx data pointer
 * 
 * @return None
 *
 *******************************************************************************
*/
void spiReadBytes(uint16_t size, uint8_t *rx_data)
{   
    _gAFEInComingApi.p_spi_read(size, rx_data);
}





/**
 *******************************************************************************
 * Function: adBmsWakeupIc
 * @brief Wakeup bms ic using chip select
 *
 * @details This function wakeup thr bms ic using chip select.
 *
 * @param [in]  total_ic    Total_ic
 *
 * @return None
 *
 *******************************************************************************
*/
void adBmsWakeupIc(uint8_t total_ic)
{
    for (uint8_t ic = 0; ic < total_ic; ic++)
    {
        adBmsCsLow();
        Adbms6830DelayUs(20);
        adBmsCsHigh();
        Adbms6830DelayUs(20); 
    }
    Adbms6830DelayUs(500);
}

/** @}*/
/** @}*/