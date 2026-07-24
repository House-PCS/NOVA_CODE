
#ifndef __SOX_H_
#define __SOX_H_

#include "bsp.h"

#define CHARGE_MAX_VOLTAGE              (3600)          /*充电的最大电压  3600mV */
#define DISCHARGE_MIN_VOLTAGE           (2700)          /*放电的最小电压为2700 mV*/

#define PACK_RATED_CAPACITY             (100000)        /*电池单体容量为100000mAh*/
#define PACK_CURRENT_MIN_THRESHOLD      (300)           /*电池最小电流阈值为ADC采样的误差 单位mA*/
#define PACK_ACT_MAX_SOC_THRESHOLD      (100)           /*SOC的最大阈值*/
#define PACK_ACT_MIN_SOC_THRESHOLD      (5)             /*SOC的最小阈值*/

typedef enum
{
    EN_PACK_CURRENT_IDLE_MODE,
    EN_PACK_CURRENT_DISCHARGE_MODE,
    EN_PACK_CURRENT_CHARGE_MODE,
    EN_PACK_CURRENT_NONE
} EN_PACK_CURRENT_MODE;

/* 限流模式 */
typedef enum
{
    EN_CC_MODE_IDLE,                    //空闲
    EN_CC_MODE_OPEN_CC,                 //强制开启充电限流
    EN_CC_MODE_OPEN_CHG_MOS,            //开启充电MOS
    EN_CC_MODE_CLOSE_CHG_MOS_AND_CC,    //强制关闭充电MOS与限流
    EN_CC_MODE_OPEN_DSG_MOS,            //开启放电MOS
    EN_CC_MODE_CLOSE_DSG_MOS,           //强制关闭放电MOS
} EN_CC_MODE;

/* 限流模式 */
typedef enum
{
    EN_CC_STATUS_OPEN,                //开启
    EN_CC_STATUS_CLOSE,               //关闭
    EN_CC_STATUS_MOS_CLOSE,           //MOS断开
} EN_CC_STATUS;

#pragma pack(1)
typedef struct _soc_stru_t
{
    float       soc_max;    /* 最大允许放电深度 */
    float       soc_min;    /* 最小允许放电深度 */
    float       soc_real;   /* 真实soc */
    float       soc_actl;   /* 计算续航soc */
    float       soc_disp;   /* 仪表显示soc */
    float       soh;        /*初始值100,按常温额定容量计算，不考虑温度*/
} SOX_t;
#pragma pack()

#pragma pack(1)
typedef struct _pack_status_stru_t
{
    uint16_t    pack_info_start_flag;                   /* 保存起始标志 */
    int16_t     pack_min_temperature;                   /*单位:0.1摄氏度*/
    int16_t     pack_max_temperature;                   /*单位:0.1摄氏度*/
    int16_t     pack_average_temperature;
    
    int32_t     pack_current_real;                      /* 真实电流（校准前） *//*单位:mA, 放电>0, 充电<0*/
    int32_t     pack_current_calibration;               /* 计算电流（校准后） *//*单位:mA, 放电>0, 充电<0*/
    float       chg_pack_current_calibration_factor;    /* 充电电流校准系数 */
    float       dsg_pack_current_calibration_factor;    /* 放电电流校准系数 */
    
    uint16_t    cell_min_voltage;                       /* 电池电芯最小电压 */
    uint16_t    cell_max_voltage;                       /* 电池电芯最大电压 */
    uint16_t    cell_average_voltage;                   /* 电池电芯平均电压 */
    uint32_t    cell_total_voltage;                     /* 电池电芯总压 */

    uint16_t    full_charge_voltage;                    /* 满充电压 */
    bool        full_charge_flag;                       /* 满充标志 */
    uint16_t    empty_charge_voltage;                   /* 空放电压 */
    bool        empty_charge_flag;                      /* 空放电压 */

    uint32_t    cr;                                     /*当前充放电可用容量, cr = ce * soh, ce为电池包额定容量，受温度和电流倍率影响(查表) mAh*/
    uint32_t    cur_max_dsg_current_10s;                /* 10s放电电流 */
    uint32_t    cur_max_dsg_current_30s;                /* 30s放电电流 */
    uint32_t    cur_max_dsg_current_60s;                /* 60s放电电流 */
    uint32_t    cur_max_chg_current;                    /* 充电电流 */
    uint8_t     cur_max_chg_factor;                     /* 最大充电电流系数(只有第一个电池包会使用) */
    // uint8_t      ocv_calibration_flag;/*需要校准时置1*/
    uint32_t    ocv_time_count;                         /*掉电需要保存,每当EN_PACK_CURRENT_IDLE_MODE模式时，赋值RTC的值，当RTC的值-ocv_time_count > 阈值(如2小时), ocv_calibration_flag置1*/

    float       chg_accumulate_capa_ah;                 /*单位:Ah*/
    float       dsg_accumulate_capa_ah;                 /*单位:Ah*/

    float       chg_accumulate_capa_wh;                 /*单位:Wh*/
    float       dsg_accumulate_capa_wh;                 /*单位:Wh*/

    SOX_t       sox_info;                               /* SOX信息 */
    EN_PACK_CURRENT_MODE mode;                          /* 当前系统充放电状态 */
    EN_CC_MODE      cc_mode;                            /* 限流模式 */
    EN_CC_STATUS    cc_status;                          /* 当前限流状态（开还是关） */
    uint16_t    pack_cycles;                            /* 电芯循环次数 */
    uint16_t    current_power;                          /* 当前功率 */
    float       current_power_percentage;               /* 当前功率比例系数（0~1，默认1） */
    uint32_t    recharge_times;                         /* 预计充满时长/剩余可用时长 */
    uint16_t    soh_rpt;                                /*初始值100,按常温额定容量计算，不考虑温度 0.1%*/
    uint16_t    ocv_process_flag;                       /* OCV校准标志 */
    uint16_t    pack_info_end_flag;                     /* 保存结尾标志 */
} PackStatus_t;
#pragma pack()


extern PackStatus_t gPackStatus;
extern PackStatus_t gPackStatus_debug;
extern PackStatus_t gPackStatus_up;
extern uint8_t gFlagClear;

void OCVCalibraiton(PackStatus_t *pPackStatus);
//void FullChargeCalibration(void);

void CurrentCapaCalc(PackStatus_t *pPackStatus);
void SOCAmpereHourIntegral(uint32_t ms, PackStatus_t *pPackStatus);
//void FullChargeRatedVoltageCalc(void);
void PackCurrentModeJudge(PackStatus_t *pPackStatus);

void SoxInit(PackStatus_t *pPackStatus);
void SoxProcess(PackStatus_t *pPackStatus);

void SoxInfoSave(PackStatus_t *p_pack_info);
void SoxInfoLoad(PackStatus_t *p_pack_info);
void SoxInfoSaveInUpgrade(PackStatus_t *p_pack_info);
void SoxInfoLoadInUpgrade(PackStatus_t *p_pack_info);

void FlagClear(void);

float InterpolationOperation(int32_t x1, int32_t x2, int32_t y1, int32_t y2, int32_t x);
float InterpolationOperationf(float x1, float x2, float y1, float y2, float x);
#endif




















