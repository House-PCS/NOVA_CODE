/*
 * sci.h
 *
 *  Created on: 2025Äê1ÔÂ6ÈÕ
 *      Author: Nova007456
 */
#include "F28x_Project.h"
#include "system.h"

#ifndef DEVICE_SCI_H_
#define DEVICE_SCI_H_

#define PRINTF_ENABLE   (0)

#define SCID_FREQ        115200//1250000
#define SCID_PRD         ((LSPCLK_FREQ/(SCID_FREQ*8))-1)

#define SCIA_FREQ        (uint32_t)115200
#define SCIA_PRD         ((LSPCLK_FREQ/(SCIA_FREQ*8))-1)

#define SCIB_FREQ        (uint32_t)115200
#define SCIB_PRD         ((LSPCLK_FREQ/(SCIB_FREQ*8))-1)

#define SCIC_FREQ        (uint32_t)9600 //memter
#define SCIC_PRD         ((LSPCLK_FREQ/(SCIC_FREQ*8))-1)



void Sci_Init(void);
void ScidSendByte(uint16_t data);
void SciaSend(uint8_t* data, uint16_t len);
void ScibSend(uint8_t* data, uint16_t len);
void ScicSend(uint8_t* data, uint16_t len);
void ScicFifoInit();

#endif /* DEVICE_SPI_H_ */


