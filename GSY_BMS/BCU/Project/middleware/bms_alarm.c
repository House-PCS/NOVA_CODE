/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        uart.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2022-10-31
  * @brief       
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2022-10-31    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
  
  


#include "bms_alarm.h"
#include "systick.h"

AlarmInfoType AlarmInfo={0};
const AlarmInfoType* const IR_AlarmInfo=&AlarmInfo;

/*******************************************************************************
  * @FunctionName: 
  * @Author:       
  * @DateTime:     2023年3月7日T16:09:58+0800
  * @Purpose:      
  * @param:        fun  Time模块对外开放的获取系统时间的函数名         
  * @return:
*******************************************************************************/
void BmsAlarmInit(){
//	uint8_t i=0;
//	for(i=0;i<ALARM_NUM;i++){
//		AlarmInfo[i].AlarmNum=0;
//	}
//	
//	SysTimeFunc=fun;
}
//void *temp_p;
//uint8_t temp_j;
/** 
* @brief  报警模块主函数
* @param  none
* @return  none
* @note:
*
*/
void BmsAlarmMain(void )
{
	uint8_t i=0,j=0;
    
	for(i=0;i<AlarmInfo.AlarmTypeNum;i++){
		for(j=0;j<AlarmInfo.AlarmPacket[i].AlarmNum; j++){
//             temp_p = AlarmInfo.AlarmPacket[i].AlarmParts[j].AlarmParseFunc;
//            temp_j = AlarmInfo.AlarmPacket[i].AlarmParts[j].AlarmCode;
			if(AlarmInfo.AlarmPacket[i].AlarmParts[j].AlarmParseFunc == 0)
            {	
				continue;		//空指针保护
			}
            if(tim_get_systime() <= AlarmInfo.AlarmPacket[i].AlarmParts[j].EnabledTime)
            {
                continue;
            }
			if(AlarmInfo.AlarmPacket[i].AlarmParts[j].AlarmParseFunc() ){
				if(AlarmInfo.AlarmPacket[i].AlarmParts[j].lastAlarmState==0)
                {
					AlarmInfo.AlarmPacket[i].AlarmParts[j].ValidTimer = tim_get_systime();
					AlarmInfo.AlarmPacket[i].AlarmParts[j].lastAlarmState=1;
				}else{
					if(tim_get_systime() - AlarmInfo.AlarmPacket[i].AlarmParts[j].ValidTimer >= AlarmInfo.AlarmPacket[i].AlarmParts[j].EnabledTime){
						AlarmInfo.AlarmPacket[i].alarmCode[AlarmInfo.AlarmPacket[i].AlarmParts[j].AlarmCode>>3] |= (uint8_t)(1<<((AlarmInfo.AlarmPacket[i].AlarmParts[j].AlarmCode)%8));
					}
				}
			}
			else {
				if(AlarmInfo.AlarmPacket[i].AlarmParts[j].lastAlarmState){
					AlarmInfo.AlarmPacket[i].AlarmParts[j].InvalidTimer=tim_get_systime();
					AlarmInfo.AlarmPacket[i].AlarmParts[j].lastAlarmState=0;
				}else{
					if(tim_get_systime() - AlarmInfo.AlarmPacket[i].AlarmParts[j].InvalidTimer >= AlarmInfo.AlarmPacket[i].AlarmParts[j].DisenabledTime){
						AlarmInfo.AlarmPacket[i].alarmCode[AlarmInfo.AlarmPacket[i].AlarmParts[j].AlarmCode>>3] &= ~((uint8_t)(1<<((AlarmInfo.AlarmPacket[i].AlarmParts[j].AlarmCode)%8)));
					}
				}
			}
			
		}//for AlarmInfo[i].AlarmNum
	}//for AlarmInfo.AlarmTypeNum
	
}
