/*
 * gpio.h
 *
 *  Created on: 2025年1月6日
 *      Author: Nova007456
 */

#ifndef DEVICE_GPIO_H_
#define DEVICE_GPIO_H_
#include "system.h"

// 风扇转速检测GPIO配置
#define FAN1_CAP_GPIO           (80)
#define FAN2_CAP_GPIO           (81)
#define FAN3_CAP_GPIO           (52)
#define FAN4_CAP_GPIO           (53)
//风扇输出GPIO配置
#define FAN1_OUT_GPIO           (18)
#define FAN2_OUT_GPIO           (19)
#define FAN3_OUT_GPIO           (20)
#define FAN4_OUT_GPIO           (21)
#define RCMU_CHK_GPIO          (28)
void LedGpioToggle(void);
void LedGpioSet(void);
void LedGpioReset(void);
void TestCpuGpioToggle(void);
void TestCpuGpioSet(void);
void TestCpuGpioReset(void);
void AdcGpioSwitch(EN_CPU_INTERRUPT_TIME vlaue);
void FlashCsGpioHigh(void);
void FlashCsGpioLow(void);
void Gpio_Init(void);

void SetRcmuChkState(uint16_t value);
#endif /* DEVICE_GPIO_H_ */
