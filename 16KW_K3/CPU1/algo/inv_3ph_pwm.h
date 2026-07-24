/**
 * @file inv_3ph_pwm.h
 * @brief Three-phase T-type three-level split-bus EPWM driver
 * @note EPWMxA = bridge arm, EPWMxB = neutral-point switch; zero state = bridge off + middle on
 */

#ifndef DEVICE_INV_3PH_PWM_H_
#define DEVICE_INV_3PH_PWM_H_

#include "stdint.h"

#define INV_3PH_PWM_DUTY_MAX        (0.97f)
#define INV_3PH_PWM_DUTY_MIN        (-0.97f)
#define INV_3PH_PWM_ZERO_BAND       (0.04f)

void Inv3PhPwm_ErrClear(void);

uint16_t Inv3PhPwm_Update(float duty_a, float duty_b, float duty_c, uint16_t en);
void ThreePhasePwmUpDate(float dutyA,float dutyB,float dutyC,float en_flag);
#endif /* DEVICE_INV_3PH_PWM_H_ */
