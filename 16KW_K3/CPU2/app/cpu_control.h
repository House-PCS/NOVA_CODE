/*
 * cpu_control.h
 *
 *  Created on: Nov 22, 2024
 *      Author: Nova007456
 */
#ifndef APP_CPU_CONTROL_H_
#define APP_CPU_CONTROL_H_

#include <stdint.h>


void CpuIntFunInit(void);
void CpuIntServiceFun(void);
uint32_t GetCpuInterruptRunCont(void);
uint32_t GetCpuInterruptQIdleCnt(void);

void CpuControl_Ph3CalcInit(void);
void CpuControl_Ph3StsInit(void);
void CpuControl_Ph3DataUpdate(void);
void CpuControl_Ph3FlagUpdate(void);

#endif /* APP_CPU_CONTROL_H_ */
