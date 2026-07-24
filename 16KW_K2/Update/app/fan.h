/*
 * Fan.h
 *
 *  Created on: 2024年8月12日
 *      Author: Nova007456
 */
#include "stdint.h"
#include "F28x_Project.h"

#ifndef MIDDLEWARE_FAN_H_
#define MIDDLEWARE_FAN_H_

// ==================== 配置区 ====================
// 风扇数量配置 - 可随意修改
#define FAN_NUM                 (1)

// 风扇转速检测GPIO配置
#define FAN1_CAP_GPIO           (80)
#define FAN2_CAP_GPIO           (81)
#define FAN3_CAP_GPIO           (52)
#define FAN4_CAP_GPIO           (53)

// 温度阈值配置 (单位: °C)
#define TEMP_THRESHOLD_LEVEL0       (45.0f)
#define TEMP_THRESHOLD_LEVEL1       (55.0f)
#define TEMP_THRESHOLD_LEVEL2       (65.0f)
#define TEMP_THRESHOLD_LEVEL3       (75.0f)
#define TEMP_THRESHOLD_LEVEL4       (85.0f)

// 自检时间配置 (100ms单位)
#define SELF_CHECK_TIME         (36 * FAN_NUM)        // 1.2秒

// 转速故障判断配置
#define SPEED_FAULT_CHECK_TIME  ((80000/4)*0.25f)       // 转速故障检测时间(20KHz中断次数 = 250ms)

// 风扇状态枚举
typedef enum {
    FAN_STATE_INIT = 0,
    FAN_STATE_SELF_TEST,
    FAN_STATE_NORMAL,
    FAN_STATE_FAULT,
    FAN_STATE_RECOVERY
} FAN_STATE;

// 风扇转速状态枚举
typedef enum {
    EN_FAN_SPEED_0_PERCENT = 0,
    EN_FAN_SPEED_20_PERCENT,
    EN_FAN_SPEED_40_PERCENT,
    EN_FAN_SPEED_60_PERCENT,
    EN_FAN_SPEED_80_PERCENT,
    EN_FAN_SPEED_100_PERCENT,
    EN_FAN_SPEED_MAX
} EN_SPEED_STATE;


// 风扇故障类型枚举
typedef enum {
    FAN_IN_FAULT_PULSE = 0,      //脉冲检测
    FAN_IN_FAULT_LEVLE           //电平检测
} FAN_IN_CHK_TYPE;

typedef enum {
    IN_LOW_ACTIVE = 0,
    IN_HIGH_LEVLE
} FAN_ACTIVE_LEVEL_TYPE;    //有效电平状态（正常电平）

// 风扇状态结构体
typedef struct {
    FAN_STATE          current_state;
    EN_SPEED_STATE     speed;
} FanState_t;

// 风扇数值结构体
typedef struct {
    float duty;
    float temperature;
    uint32_t level_cont;   // 电平脉冲计数
    uint32_t total_level_cont;  // 总电平计数
} FanValue_t;


// 风扇配置参数结构体
typedef struct {
    uint16_t self_check_time;               // 自检时间(100ms单位)

    uint32_t speed_check_counter;           // 转速检测计数器
    uint16_t cap_gpio;                      // 转速检测GPIO

    FAN_IN_CHK_TYPE    in_chk_type;
    uint8_t            fault_detected;      // 故障检测标志
    FAN_ACTIVE_LEVEL_TYPE  active_level_type;   //有效电平状态（非故障后电平）
} FanConfig_t;

// 风扇主结构体
typedef struct {
    FanState_t state;
    FanValue_t value;
    FanConfig_t config;
    uint16_t self_check_cont;
} FanStrut_t;

// 函数声明
void FanInit(void);
void FanProcessAll(void);
void FanFaultProcessAll(void);
uint8_t TempDataGet(float temp_array[]);
void FanProcess(uint8_t fanIndex);
bool GetFanFaultState(void);
//获取风扇状态信息
uint8_t FanGetAllFaultState(void);
void FanProcessAll(void);
#endif /* MIDDLEWARE_FAN_H_ */
