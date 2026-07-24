
//#include "led_process.h"
//#include "sys_info.h"
//#include "dev_led.h"

////////uint8_t         brightness;
//EN_LED_STATUS   led_status = LED_STATUS_OFF;

////void    SetAmbienceLampOff(void)
////{
////    AtmosLampSet(LED_STATUS_OFF,  0);
////} 
//void  SetRunLedFlash(void)
//{
//    AtRunLedLampSet(LED_STATUS_BLINK,  0);    
//}


//void LedAppFun(void)
//{
//    SetRunLedFlash();
//    //AtmosLedProcess();
////	if((gPackStatus.mode == EN_PACK_CURRENT_DISCHARGE_MODE)||(gPackStatus.mode == EN_PACK_CURRENT_IDLE_MODE))
////		{
////			float PowerData1=gPackStatus.sox_info.soc_disp;   // 
////			DSGLedProcess(PowerData1);

////		}
////		if(gPackStatus.mode == EN_PACK_CURRENT_CHARGE_MODE)
////		{
////			float PowerData2=gPackStatus.sox_info.soc_disp;   //
////			CHGLedProcess(PowerData2);
////			
////		}
//}

//void DSGLedProcess(float PowerData1)
//{
////	uint8_t    temp;
////	temp = PowerData1/10;	
////	led_status = LED_STATUS_ON;
////	AtmosLampSet(led_status, brightness);	
////    switch(temp)
////    {
////        case 0://电量0%~9%
////        {
////					  SetLedClour(0xff,0x00,0x00,temp+1);//
////            break;
////        }
////        case 1://电量在10%~19%
////        {
////					  SetLedClour(0xff,0xff,0x00,temp+1);//
////            break;
////        }
////        case 2://电量在20%~29%
////        {   
////					  SetLedClour(0x00,0xff,0x00,temp+1);//
////            break;
////        }
////        case 3://电量在30%~39%
////        {
////					  SetLedClour(0x00,0xff,0x00,temp+1);//
////            break;
////				}
////        case 4://电量在40%~49%
////        {
////					  SetLedClour(0x00,0xff,0x00,temp+1);//
////            break;
////        }
////        case 5://电量在50%~59%
////        {
////					  SetLedClour(0x00,0xff,0x00,temp+1);//
////            break;
////        }	
////        case 6://电量在60%~69%
////        {
////					  SetLedClour(0x00,0xff,0x00,temp+1);//
////            break;
////        }	
////        case 7://电量在70%~79%
////        {
////					  SetLedClour(0x00,0xff,0x00,temp+1);//
////            break;
////        }	
////        case 8://电量在80%~89%
////        {
////					  SetLedClour(0x00,0xff,0x00,temp+1);//
////            break;
////        }
////        case 9://电量在90%~99%
////        {
////					  SetLedClour(0x00,0xff,0x00,temp+1);//
////            break;
////        }
////        case 10://电量在100%
////        {
////					  SetLedClour(0x00,0xff,0x00,temp);//
////            break;
////        }				
////        default:break;
////    }
//	
//}

//void CHGLedProcess(float PowerData2)
//{
////	uint8_t    temp;
////	temp = PowerData2/10;	
////	led_status = LED_STATUS_BLINK;
////	AtmosLampSet(led_status, brightness);
////	SetLedClour(0x00,0xff,0x00,temp+1);//	
//}

////void AtmosLedProcess()
////{

//////    led_status =LED_STATUS_ON;
////      AtmosLampSet(led_status, brightness);
////      SetLedClour(0xff,0x00,0x00,8);//
//////    switch(led_status)
//////    {
//////        case 0://关闭
//////        {
//////            led_status = LED_STATUS_OFF;
//////            AtmosLampSet(led_status, brightness);
//////            break;
//////        }
//////        case 1://长亮
//////        {
//////            led_status = LED_STATUS_ON;
//////            AtmosLampSet(led_status, brightness);
//////            break;
//////        }
//////        case 2://呼吸
//////        {   
//////            led_status = LED_STATUS_BREATHING;
//////            AtmosLampSet(led_status, brightness);
//////            break;
//////        }
//////        case 3://闪烁
//////        {
//////            led_status = LED_STATUS_BLINK;
//////            AtmosLampSet(led_status, brightness);
//////            break;
//////        }
//////        case 4://SOS
//////        {
//////            led_status = LED_STATUS_SOS;
//////            AtmosLampSet(led_status, brightness);
//////            break;
//////        }
//////        default:break;
//////    }
////}
