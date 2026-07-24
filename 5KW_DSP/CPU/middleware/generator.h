/*
 * generator.h
 *
 *  Created on: 2024年11月18日
 *      Author: Nova007456
 */

#ifndef MIDDLEWARE_GENERATOR_H_
#define MIDDLEWARE_GENERATOR_H_

#include "F28x_Project.h"

//定义枚举(发电机状态)
typedef enum
{
    EN_GRID_MODE,       //发电机待机状态（市电模式）
    EN_GRID_TO_GEN,     //市电切发电机状态
    EN_GEN_TO_GRID,     //发电机切市电状态
    EN_GEN_MODE,        //发电机模式状态
} GeneratorState_t;

typedef struct fault_stru_t
{
    uint16_t OverVolt;  //发电机过压
    uint16_t UnderVolt; //发电机欠压
    uint16_t VoltFault; //电压故障
} GeneratorFault_t;

typedef struct genstate_stru_t
{
    uint16_t SysForceGenFlag;           //系统强制发电机模式 置1有效
    uint16_t VoltNormal;                //发电机模式电压频率正常
    uint16_t LoadFault;                //发电机模式负载异常
    uint16_t voltcheck;
    uint16_t LoadCheck;
    float32  VoltTemp;
    uint16_t RlyOpenFinish;             //继电器已断开可以修改底层采样标志位 置1有效
    uint16_t InitFinish;                //初始化完成
    uint16_t AdcGenToGridChangeFinish;  //发电机切市电切换完成
    uint16_t AdcGridToGenChangeFinish;  //市电切发电机切换完成
} GenState_t;

typedef struct gen_stru_t
{
    uint16_t WorkMode;
    GeneratorFault_t Fault;
    GenState_t State;
} GenInfoGet_t;

void InputModeChange(void);
void SetForceGenFlag(uint16_t Value);
void SetGenRlyOpenFinish(uint16_t Value);
uint16_t GetGenToGridFinish(void);
uint16_t Getvoltcheck(void);
uint16_t GetGridToGenFinish(void);
uint16_t GetGenWorkMode(void);
uint16_t GetGenVoltNormal(void);
uint16_t GetGenLoadFault(void);
#endif
