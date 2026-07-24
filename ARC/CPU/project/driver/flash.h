
#ifndef __FLASH_H_
#define __FLASH_H_

#include "n32g4fr_flash.h"

#define FMC_PAGE_SIZE           ((uint16_t)0x800U)
#define FMC_START_ADDR          ((uint32_t)0x08000000U)


void InternalFlashWrite(uint32_t addr, uint32_t *p, uint32_t len);
void InternalFlashWriteWithoutErase(uint32_t addr, uint32_t *p, uint32_t len);
void InternalFlashRead(uint32_t addr, uint8_t *p, uint32_t len);
void InternalFlashErasePage(uint32_t addr);
void InternalFlashReadWord(uint32_t addr, uint32_t *p, uint32_t len);

void InternalFlashLock(void);
void InternalFlashUnlock(void);
bool InternalFlashIsLocked(void);
#endif







