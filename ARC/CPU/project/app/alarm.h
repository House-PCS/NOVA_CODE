#ifndef __ALARM_H_
#define __ALARM_H_

#include "stdbool.h"
#include "stdio.h"
#include "drv_opt.h"
#include "dma.h"

void Alarm(uint8_t value);


typedef enum
{
    EN_ALARM_TYPE_CHANNEL1    = 1,
	EN_ALARM_TYPE_CHANNEL12   = 3,
    EN_ALARM_TYPE_CHANNEL123  = 7,
	EN_ALARM_TYPE_CHANNEL1234 = 15
}EN_ALARM_TYPE;



#define RES_RETRY_INTERVAL_5MIN_MS    (5U * 60U * 1000U)   
#define RES_RETRY_INTERVAL_2H_MS      (2U * 60U * 60U * 1000U) 
#define RES_RETRY_COUNT_TO_2H         24U                  

uint8_t Res_Detect_Alarm(void);           
void Arc_Alarm(void);                      
void Res_Detect_Init(void);                
void Res_Retry_Start(void);                
uint8_t Res_Retry_IsActive(void);          
uint8_t Res_Retry_IsBusy(void);            
void Res_Retry_OnDetectComplete(uint8_t channal_num); 
void Res_Retry_Process(void);              
//void ArcHandleClearProcess();

#endif