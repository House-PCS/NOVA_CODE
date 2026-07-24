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
#ifndef __AFE_CONTROL_H	
#define __AFE_CONTROL_H	

/* Inculdes ------------------------------------------------------------------*/

#include "adbms6830.h"
#include "ntc.h"
#include "debug.h"
//#include "sox.h"


/* Exported typedef -----------------------------------------------------------*/
typedef enum
{
	EN_AFE_IDLE = 0,    /*空闲*/
	EN_AFE_BUSY_V,  	/*电压转换中*/
    EN_AFE_BUSY_C,  	/*电流转换中*/
    EN_AFE_BUSY_T0,  	/*温度转换中*/
    EN_AFE_BUSY_T1,  	/*温度转换中*/
    EN_AFE_BUSY_T2,  	/*温度转换中*/
    EN_AFE_BUSY_T3,  	/*温度转换中*/
    EN_AFE_BUSY_T_AFE,  /*温度转换中*/
	EN_AFE_GET_V, 		/*采集电压*/
    EN_AFE_GET_C, 		/*采集电流*/
	EN_AFE_GET_T0, 		/*采集温度*/
    EN_AFE_GET_T1, 		/*采集温度*/
    EN_AFE_GET_T2, 		/*采集温度*/
    EN_AFE_GET_T3, 		/*采集温度*/
    EN_AFE_GET_T_AFE, 	/*采集温度*/
    EN_AFE_SET_B,
    EN_AFE_CLEAR_B,
    
}en_afe_state;

typedef struct _Cell_handle_stru_t
{
    uint32_t    total_cell_voltage;
    uint16_t    cell_voltage[20];
    uint16_t    cell_max_voltage;
    uint16_t    cell_min_voltage;
    uint16_t    cell_ave_voltage;
    int32_t     cell_current;
    int16_t     cell_temperature[20];
    int16_t     cell_max_temperature;       /* 0.1c */
    int16_t     cell_min_temperature;
    int16_t     cell_ave_temperature;
    int16_t     afe_net_temperature;
    int16_t     dmos_temperature;
    int16_t     cmos_temperature;
    int16_t     emos_temperature;
    uint8_t     current_status;     /* 0-idle 1:dsg 2:chg */
}Cell_Info;

void AFEInit(void);
bool VotageCurrentGet(void);
bool SetPassiveBalance(uint8_t cell_serial);
bool ClearPassiveBalance(uint8_t cell_serial);
bool SetBalanceCurrent(PWM_DUTY e_duty);
void AFEProcess(void);   
bool VotageCurrentGet(void);
bool CurrentGet(void);

bool TemperatureGet_T0(void);
bool TemperatureGet_T1(void);
bool TemperatureGet_T2(void);
bool TemperatureGet_T3(void);
bool TemperatureGet_AFE(void);
en_afe_state AFEStatusGet(void);
bool AFEFirstSample(void);

extern Cell_Info gCell_Info;
extern en_afe_state en_afe;
extern uint8_t gFirst_AFE_Coved_Flag;
#endif /*__MID_SHELL_H */ 
                
