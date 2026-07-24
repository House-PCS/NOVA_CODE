/**
 ******************************************************************************
 * @file    shell_netif.h
 * @author  NovaStar R&D Team
 * @version V1.0.0
 * @date    2017.09.28
 * @brief
 ******************************************************************************
 * @attention
 *
 * History<br>
 * version     date        auther      description<br>
 * V1.0.0      2017.11.13  Huyuxin     Initial Version<br>
 *
 ******************************************************************************
 */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SENSOR_CONTROL_H
#define __SENSOR_CONTROL_H

/* Inculdes ------------------------------------------------------------------*/

#include "debug.h"
#include "bsp.h"

/* Exported typedef -----------------------------------------------------------*/

typedef enum
{
    EN_OCV_V_GET_STEPS_OVC_OPEN,
    EN_OCV_V_GET_STEPS_ADC_GET,
    EN_OCV_V_GET_STEPS_OVC_CLOSE
} EN_OCV_V_GET_STEPS;



void     SensorInit       (void);

uint32_t PackVotageGet    (void);
float    MosTemperatureGet(void);
float    TaTemperatureGet (void);
float    AfeTemperatureGet(void);

float PackNtc1TemperatureGet(void);
float PackNtc2TemperatureGet(void);
float PackNtc3TemperatureGet(void);
float PackNtc4TemperatureGet(void);

void OVCEnOpen(void);
void OVCEnClose(void);
int32_t OVCVoltageGet(void);

#endif /*__SENSOR_CONTROL_H */

