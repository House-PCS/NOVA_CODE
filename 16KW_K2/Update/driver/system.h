/*
 * system.h
 *
 *  Created on: 2025年1月6日
 *      Author: Nova007456
 */
#ifndef SYSTEM_H_
#define SYSTEM_H_

#define CPU_SYS_CLOCK       (200*1000000)
#define LSPCLK_FREQ         (CPU_SYS_CLOCK/4)
#define PWMSYSCLOCK_FREQ    (100*1000000)

#include "F28x_Project.h"

//定义枚举(CPU中断时序切换状态)
typedef enum
{
    EN_CPU_INTERRUPT_TIME_0,
    EN_CPU_INTERRUPT_TIME_1,
    EN_CPU_INTERRUPT_TIME_2,
    EN_CPU_INTERRUPT_TIME_3,
    EN_CPU_INTERRUPT_TIME_4,
} EN_CPU_INTERRUPT_TIME;

void SysRestart(void);

//#define _LAUNCHXL_F28379D


#endif
