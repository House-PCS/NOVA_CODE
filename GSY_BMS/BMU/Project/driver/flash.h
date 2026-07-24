
#ifndef __FLASH_H_
#define __FLASH_H_

#include "gd32f30x_fmc.h"

#define FMC_PAGE_SIZE           ((uint16_t)2048)
#define FMC_START_ADDR          ((uint32_t)0x08000000U)       //
//#define FMC_START_ADDR          ((uint32_t)0x00000000U)         //


void InternalFlashWrite(uint32_t addr, uint32_t *p, uint32_t len);
void InternalFlashWriteWithoutErase(uint32_t addr, uint32_t *p, uint32_t len);
void InternalFlashRead(uint32_t addr, uint8_t *p, uint32_t len);
void InternalFlashErasePage(uint32_t addr);
void InternalFlashReadWord(uint32_t addr, uint32_t *p, uint32_t len);

#endif







