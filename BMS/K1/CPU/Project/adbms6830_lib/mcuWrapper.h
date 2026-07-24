/*******************************************************************************
Copyright (c) 2020 - Analog Devices Inc. All Rights Reserved.
This software is proprietary & confidential to Analog Devices, Inc.
and its licensor.
******************************************************************************
* @file:    mcuWrapper.h
* @brief:   Generic wrapper header file
* @version: $Revision$
* @date:    $Date$
* Developed by: ADIBMS Software team, Bangalore, India
*****************************************************************************/
/** @addtogroup MUC_DRIVER
*  @{
*
*/

/** @addtogroup SPI_DRIVER SPI DRIVER
*  @{
*
*/
#ifndef __ADBMSWRAPPER_H
#define __ADBMSWRAPPER_H
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


/*需要向lib库传入的底层接口，主要为延时函数、SPI读写函数*/
typedef struct _afe_incoming_api_stru_t
{
	void		(*p_delay_us)(uint32_t );
	void		(*p_delay_ms)(uint32_t );
	void		(*p_spi_cs_low)();
	void		(*p_spi_cs_high)();
	void		(*p_spi_write_read)(uint8_t *, uint8_t *, uint16_t );
	void		(*p_spi_read)(uint16_t , uint8_t * );
	void		(*p_spi_write)(uint16_t , uint8_t * );
}AFEInComingApi_t;


void ADBMS6830LibInit(AFEInComingApi_t *p_api);


void Adbms6830DelayMs(uint32_t delay);
void Adbms6830DelayUs(uint32_t delay);
void adBmsCsLow(void);
void adBmsCsHigh(void);
void spiWriteBytes
( 
  uint16_t size,                                /*Option: Number of bytes to be written on the SPI port*/
  uint8_t *tx_Data                              /*Array of bytes to be written on the SPI port*/
);
void spiWriteReadBytes
(
  uint8_t *tx_data,                             /*array of data to be written on SPI port*/
  uint8_t *rx_data,                             /*Input: array that will store the data read by the SPI port*/
  uint16_t size                             /*Option: number of bytes*/
);
void spiReadBytes(uint16_t size, uint8_t *rx_data);

uint32_t getTimCount(void);
void adBmsWakeupIc(uint8_t total_ic);

#endif
/** @}*/
/** @}*/

