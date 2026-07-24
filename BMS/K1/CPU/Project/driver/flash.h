#ifndef __FLASH_H_
#define __FLASH_H_

#include <stdint.h>
#include "gd32f30x_fmc.h"


void FlashWrite(uint16_t len,uint8_t  *data,uint32_t addr_start);
void FlashRead(uint16_t len,uint8_t *outdata,uint32_t addr_start);
void FlashErase(uint32_t start, uint32_t end);

#endif







