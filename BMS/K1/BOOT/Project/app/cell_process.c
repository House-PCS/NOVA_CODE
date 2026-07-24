#include "cell_process.h"
//#include "led_process.h"
//#include "sys_info.h"
//#include "dev_led.h"
//	AFEProcess();

void AFEFunInit(void)
{   
	uint8_t sox_init_flag = 0;
	if(AFEFirstSample() == true)	
	{
		if(!sox_init_flag)
		{
			sox_init_flag = 1;
			if(gSoxDebugParam.sox_debug_mode == 0)
			{
				SoxInit(&gPackStatus); 
			}
			else if(gSoxDebugParam.sox_debug_mode == 1)
			{
				SoxInit(&gPackStatus_debug);         
			}
			else if(gSoxDebugParam.sox_debug_mode == 2)
			{
				SoxInit(&gPackStatus); 
				SoxInit(&gPackStatus_debug);   
			}
                /* 开始AFE采集 */
                 StartAFESample();
//                gFirst_AFE_Coved_Flag = 8;
		}
//		ProtCan_Main();        
//		CanTx_Main();  //时时刻刻从缓冲里边查看是否有数据需要发送        
		FaultAnalysisProcess(); 
//		BMSRunStateProcess();                					
            
		if(gSoxDebugParam.sox_debug_mode == 0)
		{
			SoxProcess(&gPackStatus); 
		}
		else if(gSoxDebugParam.sox_debug_mode == 1)
		{
			SoxProcess(&gPackStatus_debug); 
		}
		else if(gSoxDebugParam.sox_debug_mode == 2)
		{
			SoxProcess(&gPackStatus); 
			SoxProcess(&gPackStatus_debug); 
		}
	} 
}