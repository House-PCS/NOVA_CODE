
#ifndef __BOOT_H_
#define __BOOT_H_

#include "dev_spiflash.h"
#include "bsp.h"
#include "String.h"
#include "flash.h"

typedef enum
{
    EN_FW_EXT_STATUS_NON_EXIST,
    EN_FW_EXT_STATUS_EXIST_NEW,
    EN_FW_EXT_STATUS_EXIST_OLD
}EN_FW_EXT_STATUS;


void JudgeIfBootMode();

void ExternalFWForceTrans();



#endif


