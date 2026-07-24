/*
 * pwm.h
 *
 *  Created on: 2025Äê1ÔÂ6ÈÕ
 *      Author: Nova007456
 */
#ifndef DEVICE_PWM_H_
#define DEVICE_PWM_H_

#include "system.h"

#define TO_EVEN(x) ((int)(x) - ((int)(x) % 2))

#define INV_PWM_FREQUENCY   (20*1000)
#define INV_DEADBAND_US     (1.0)
#define INV_PWM_PERIOD      ((PWMSYSCLOCK_FREQ)/(INV_PWM_FREQUENCY))       //100*1000000/20*1000
#define INV_DEADBAND        (INV_DEADBAND_US * PWMSYSCLOCK_FREQ * 0.000001)

#define PV_PWM_FREQUENCY   (40*1000ul)
#define PV_DEADBAND_US     (0.5)
#define PV_PWM_PERIOD      (TO_EVEN((PWMSYSCLOCK_FREQ/PV_PWM_FREQUENCY)))
#define PV_DEADBAND        (PV_DEADBAND_US * PWMSYSCLOCK_FREQ * 0.000001)

#define CPU_INTERRUPT_PWM_FREQUENCY   (80*1000ul)
#define CPU_INTERRUPT_DEADBAND_US     (0.5)
#define CPU_INTERRUPT_PWM_PERIOD      ((PWMSYSCLOCK_FREQ)/(CPU_INTERRUPT_PWM_FREQUENCY))
#define CPU_INTERRUPT_DEADBAND        (CPU_INTERRUPT_DEADBAND_US * PWMSYSCLOCK_FREQ * 0.000001)

#define CLA_INTERRUPT_PWM_FREQUENCY   (20*1000)
#define CLA_INTERRUPT_DEADBAND_US     (0.5)
#define CLA_INTERRUPT_PWM_PERIOD      ((PWMSYSCLOCK_FREQ)/(CLA_INTERRUPT_PWM_FREQUENCY))
#define CLA_INTERRUPT_DEADBAND        (CLA_INTERRUPT_DEADBAND_US * PWMSYSCLOCK_FREQ * 0.000001)


#define FAN_PWM_FREQUENCY   (20*1000)
#define FAN_DEADBAND_US     (1.0)
#define FAN_PWM_PERIOD      ((PWMSYSCLOCK_FREQ)/(FAN_PWM_FREQUENCY))       //100*1000000/20*1000
#define FAN_DEADBAND        (FAN_DEADBAND_US * PWMSYSCLOCK_FREQ * 0.000001)

void EPwm_Init(void);
void ClearEpwmTZ(void);
//uint16_t PvEpwmUpdata(float pvduty,uint16_t channel, uint16_t en_pv);
uint16_t PvEpwmUpdata(float * pvdutyaddress, uint16_t en_pv);
uint16_t FanEpwmUpdata(float * dutyaddress, uint16_t en_pv);
#endif /* DEVICE_PWM_H_ */
