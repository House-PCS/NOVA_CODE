/*
 * ADCadc.h
 *
 *  Created on: 2025年10月13日
 *      Author: Nova006348
 */

#ifndef DRIVER_ADC_H_
#define DRIVER_ADC_H_

#include "F28x_Project.h"
#include "system.h"

#define ACQPS_SYS_CLKS      (29)
#define ADC_TRIG_SOURCE     (18)            //触发源为EPWM7的SOCB事件

void AdcConfig(void);

#endif /* DRIVER_ADC_H_ */
