
#ifndef __SPI_H_
#define __SPI_H_
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "CMSDK_CM0.h"
#include "core_cm0.h"
#include "dev_spiflash.h"

typedef struct
{
    __O   uint32_t  reg_spi_mode;          /*!< Offset: 0x000 Data Register    (R/W) */
    __O   uint32_t  reg_freq_coe;         /*!< Offset: 0x004 Status Register  (R/W) */
    __O   uint32_t  reg_cs;          /*!< Offset: 0x008 Control Register (R/W) */
    __O   uint32_t  reg_txd;          /*!< Offset: 0x008 Control Register (R/W) */    

    __I   uint32_t  reg_rxd;       /*!< Offset: 0x010 Baudrate Divider Register (R/W) */
    __I   uint32_t  write_tx_ready;       /*!< Offset: 0x010 Baudrate Divider Register (R/W) */    

} CMSDK_SPI_TypeDef;

void SpiSendByte(uint8_t dat);
void SpiInit(void);
void SpiSend(uint8_t *data, uint16_t len);
uint8_t SPIDataGet(void);
void SpiRead(uint8_t *data, uint16_t len);

#endif


