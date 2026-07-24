#ifndef __POWER_LIMIT_H_
#define __POWER_LIMIT_H_
#include "dev_communication.h"
#include "bsp.h"

#define DEFAULT_CHG_MAX_CURRENT_VAL     (500)       //默认最大充电允许电流（0.1A）
#define DEFAULT_DSG_MAX_CURRENT_VAL     (500)       //默认最大充电允许电流（0.1A）

typedef struct
{
    float   chg_ending_factor[BMS_PACK_MAX_NUM];    //充电末端功率因数
    float   dsg_ending_factor[BMS_PACK_MAX_NUM];    //放电末端功率因数
    uint8_t chg_end_factor[BMS_PACK_MAX_NUM];       //充电结束系数（1 or 0）
    uint8_t dsg_end_factor[BMS_PACK_MAX_NUM];       //放电结束系数（1 or 0）
    
    
    
    int32_t arr_chg_current_limit[BMS_PACK_MAX_NUM];
    int32_t arr_dsg_current_limit_10s[BMS_PACK_MAX_NUM];
    int32_t arr_dsg_current_limit_30s[BMS_PACK_MAX_NUM];
    int32_t arr_dsg_current_limit_60s[BMS_PACK_MAX_NUM];
    
    int32_t arr_chg_limit_final[BMS_PACK_MAX_NUM];
    int32_t arr_dsg_limit_10s_final[BMS_PACK_MAX_NUM];
    int32_t arr_dsg_limit_30s_final[BMS_PACK_MAX_NUM];
    int32_t arr_dsg_limit_60s_final[BMS_PACK_MAX_NUM];
} AllPackCurrentLimit;


extern AllPackCurrentLimit gstAllPackLimit;

//最大电流设置
void MaxCurControl(void);

//最大充电电流限制处理
void MaxChgCurFactorProcess(void);
//最大放电电流限制处理
void MaxDsgCurFactorProcess(void);
//故障充放电因数限制处理
void FaultPowerFactorProcess(void);
//充满放空充放电因数限制处理
void FullOrEmptyFactorProcess(void);

#endif

