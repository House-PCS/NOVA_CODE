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
void SensorInit(void);
bool PackVotageGet(void);
bool Dc12VCurrentGet(void);
bool MosTemperature1Get(void);
bool MosTemperature2Get(void);
                
#endif /*__SENSOR_CONTROL_H */ 
                
