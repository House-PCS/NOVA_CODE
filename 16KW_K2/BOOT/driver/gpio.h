/*
 * gpio.h
 *
 *  Created on: 2025Äê1ÔÂ6ÈÕ
 *      Author: Nova007456
 */

#ifndef DEVICE_GPIO_H_
#define DEVICE_GPIO_H_
#include "system.h"

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


#endif /* DEVICE_GPIO_H_ */
