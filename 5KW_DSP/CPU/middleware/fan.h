/*
 * Fan.h
 *
 *  Created on: 2024年8月12日
 *      Author: Nova007456
 */
#include "stdint.h"

#ifndef MIDDLEWARE_FAN_H_
#define MIDDLEWARE_FAN_H_

#define PWMSYSCLOCK_FREQ            (100*1000000)
#define FAN_PWM_SWITCHING_FREQUENCY ((float)275*100)
#define FAN_PWM_PERIOD              (PWMSYSCLOCK_FREQ)/(FAN_PWM_SWITCHING_FREQUENCY)
#define FAN_PWM_MAX                 (FAN_PWM_PERIOD / 2)
#define FAN_PWM_MIN                 (0)
#define Value_LIMIT(x,max,min)      ((x>max)?max:((x<min)?min:x))
#define Value_TWO_MAX(x,y)          ((x>y)?x:y)
#define Value_THREE_MAX(x,y,z)      ((x>y)?((x>z)?x:z):((y>z)?y:z))

//定义枚举(风扇转速)
typedef enum
{
    EN_FAN_SPEED_0_PERCENT,
    EN_FAN_SPEED_40_PERCENT,
    EN_FAN_SPEED_60_PERCENT,
    EN_FAN_SPEED_80_PERCENT,
    EN_FAN_SPEED_100_PERCENT,
}Speed_State_t;

typedef struct Value_stru_t
{
    float    Duty;
    uint16_t Duty_CMPA;
    uint16_t DutyMax;
    uint16_t DutyMin;
    uint16_t GPIO_High_Cnt;
    uint16_t GPIO_High;
    uint16_t GPIO_Low_Cnt;
    uint16_t GPIO_Low;
    float High_Time;
    float High_TimeTemp;
}Fan_Value_t;

typedef struct State_stru_t
{
    Speed_State_t Speed;
    uint16_t Self_Check_Finish;

}Fan_State_t;

typedef struct Fan_stru_t
{
    Fan_Value_t Value;
    Fan_State_t State;
    float TemperatureMax;
}Fan_Strut_t;

float FanState_GetFault();
float FanValue_GetDuty();
void FanInit(void);
void FanCntl(void);
void FanFault(void);

#endif /* MIDDLEWARE_FAN_H_ */
