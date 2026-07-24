
#ifndef __SYS_INFO_H_
#define __SYS_INFO_H_

#include "stdint.h"
#include <string.h>


typedef struct _product_info_stru_t
{
    uint16_t            model_id;
    char                sn[20];
    uint16_t            sw_version;
    uint16_t            hw_version;
} SysInfo_t;


typedef struct _chg_time_interval_stru_t
{
    uint8_t             enable;
    uint32_t            start_time;
    uint32_t            end_time;
    uint8_t             week_enable;
    uint32_t            week_mask;/*bit0:ÐÇÆÚÌì,...bit6:ÐÇÆÚÁù*/
} ChgTimeInterval_t;

extern SysInfo_t gSysInfo;

void DeviceBaseInfoWrite(void);
void DeviceBaseInfoRead(void);

void DeviceCtlWrite(void);
void DeviceCtlRead(void);
void    DeviceWriteCapInfo(uint32_t  time, uint32_t chg_cap, uint32_t dsg_cap, uint8_t soc_data);
uint16_t    DeviceReadCapInfo(uint32_t start_time, uint32_t end_time, uint8_t *p_buf, uint16_t *p_data_size);

void SysInfoInit(void);
void SysRtcAlarmFun(void);

#endif





























