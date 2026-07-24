/*
 * gpio.h
 *
 *  Created on: 2025年1月6日
 *      Author: Nova007456
 */

#ifndef DEVICE_GPIO_H_
#define DEVICE_GPIO_H_
#include "system.h"


#define LED_GPIO               (99)
#define FLASH_CS_GPIO          (61)


#define RY_LOAD_GPIO           (90)
#define RY_INV_GPIO            (87)
#define RY1_Grid_GPIO          (88)
#define RY2_Grid_GPIO          (89)
#define RY_GEN_GPIO            (92)
#define RY_N_PE_GPIO           (91)
#define RY_EXC_GPIO            (93)

#define RY_GEN_START_GPIO      (94)

#define RCMU_CHK_GPIO          (28)

#define ARC_STATE_GPIO         (54)


#define BOOT0_STATE_GPIO      (50)
#define BOOT1_STATE_GPIO      (51)
#define BOOT2_STATE_GPIO      (52)
#define BOOT3_STATE_GPIO      (53)

#define CPU_TEST_GPIO      (93)       /*该GPIO需更换*/
#define CLA_TEST_GPIO      (33)

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


void LedGpioToggle(void);
void SetLedGpioState(uint8_t value);

void SetFlashCsGpioState(uint8_t value);

void AdcGpioSwitch(EN_CPU_INTERRUPT_TIME vlaue);
void SetCpuTestGpioState(uint8_t value);

void SetLoadRlyState(uint16_t value);
void SetInvMainRlyState(uint16_t value);
void SetGridRlyState(uint16_t value);
void SetGenRlyState(uint16_t value);
void SetPERlyState(uint16_t value);
void Gpio_Init(void);
void SetEXCRlyState(uint16_t value);

uint16_t ArcIOStateRead(void);
void SetRcmuChkState(uint16_t value);
void SetGenStarRlyState(uint16_t value);
uint16_t PoBootStateRead(void);
#endif /* DEVICE_GPIO_H_ */
