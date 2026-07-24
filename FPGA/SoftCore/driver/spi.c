/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        spi.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-07-20
  * @brief       N32G4FR
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-07-20    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "spi.h"
#define SPI           ((CMSDK_SPI_TypeDef   *) 0x41002000   )

void SpiSendByte(uint8_t dat)
{
    SPI->reg_txd = dat;
    while ((SPI->write_tx_ready - 1));
}

void SpiInit(void)
{
    SPI->reg_cs = 0x01;
    SPI->reg_freq_coe = 1;
    SPI->reg_spi_mode = 3;
    SpiFlashReadJEDECID(&gDevSpiFlash);
}


void SpiSend(uint8_t *data, uint16_t len)
{
    uint16_t i = 0;

    for(i = 0; i < len; i++)
    {       
        SpiSendByte(data[i]);
    }
}

uint8_t SPIDataGet(void)
{
    return SPI->reg_rxd;
}


void SpiRead(uint8_t *data, uint16_t len)
{
    uint16_t i = 0;

    for(i = 0; i < len; i++)
    {
        data[i] = SPIDataGet();
    }
}
