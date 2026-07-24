
#ifndef __SYS_INFO_H_
#define __SYS_INFO_H_

#include "stdint.h"
#include <string.h>
#include "dev_spiflash.h"
#include "flash.h"
#pragma pack(1)
typedef struct _product_info_stru_t
{
    uint16_t            model_id;
    char                sn[20];
    uint32_t            sw_version;
    uint16_t            hw_version;
    uint16_t            ex_version[12];
} SysInfo_t;
#pragma pack()

typedef struct _chg_time_interval_stru_t
{
    uint8_t             enable;
    uint32_t            start_time;
    uint32_t            end_time;
    uint8_t             week_enable;
    uint32_t            week_mask;/*bit0:ÐÇÆÚÌì,...bit6:ÐÇÆÚÁù*/
} ChgTimeInterval_t;

extern SysInfo_t gSysInfo;


void SysInfoInit(void);
void SystemState(void);
void GetFirmwareVersion(void);
void GetExtendedVersion(void);
void GetHardwareVersion(void);
#endif





























