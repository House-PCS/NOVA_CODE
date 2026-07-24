/*
 * spi.h
 *
 *  Created on: 2025Äê1ÔÂ6ÈÕ
 *      Author: Nova007456
 */

#ifndef DEVICE_SPI_H_
#define DEVICE_SPI_H_

#include "F28x_Project.h"

#define SPI_BRR        ((200E6 / 4) / 125E5) - 1
#define SPI_FLASH_BRR  ((200E6 / 4) / 125E5) - 1

void Spi_Init(void);
void SpiWriteData(uint16_t len, uint16_t* data);
void SpiReadData(uint16_t len, uint16_t* data);

#endif /* DEVICE_SPI_H_ */
