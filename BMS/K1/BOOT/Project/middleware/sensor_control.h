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
void     SensorInit       (void);

uint32_t PackVotageGet    (void);
float    MosTemperatureGet(void);
float    TaTemperatureGet (void);
float    AfeTemperatureGet(void);

bool     REF2_5VGet       (void);
bool     REF1VGet         (void);

uint32_t ExpVotageGet     (void);
#endif /*__SENSOR_CONTROL_H */ 
                
