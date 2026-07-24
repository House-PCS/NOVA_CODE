
#ifndef __TOOLING_PROCESS_H_
#define __TOOLING_PROCESS_H_

#include "bsp.h"



#define EXTER_FLASH_TEST_ADDR      (0x00106000)
#define EXTER_EEPROM_TEST_ADDR     (0x00000100)

void ToolingInit(void);
void ToolingProcess(void);

//ÄÍ¾Ã¶È²âÊÔ
void EnduranceTestFunc(void);
#endif














