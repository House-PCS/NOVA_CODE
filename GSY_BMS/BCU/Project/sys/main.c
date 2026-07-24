
#include "gd32f4xx.h"

#include <stdio.h>
#include "main.h"

#include "mcu_bsp.h"
#include "dev_spiflash.h"

#include "dev_sd2506.h"
#include "dev_volt_detect.h"
#include "dev_ioctl.h"
#include "dev_sgm58031.h"
#include "dev_ht7017.h"
#include "dev_uart_comm.h"
#include "dev_led.h"

#include "bcu_sox.h"
#include "current_limit.h"
#include "alarm.h"
#include "high_volt_detect.h"
#include "bms_data_manage.h"
#include "current_detect.h"
#include "temp_detect.h"
#include "ins_res_detect.h"

#include "can_tx_rx_process.h"
#include "modbus_slave_process.h"

#include "data_storage_manage.h"
#include "timer_polling.h"
#include "bmu_ctl.h"
#include "rs485_test.h"
#include "function_test.h"

#include "sys_run.h"

#define USER_FLASH_START_ADDRESS            0x08000000


/*!
    \brief    main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
	__set_PRIMASK(1);   // 关闭总中断
//    nvic_vector_table_set(NVIC_VECTTAB_FLASH, USER_FLASH_START_ADDRESS);
    nvic_vector_table_set(USER_FLASH_START_ADDRESS, 0x0);
    nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);
    __set_PRIMASK(0);   // 打开总中断  

    systick_config();
    McuBspInit();
        
    DevEepromCtlInit();
    DevRtcInit();
    DevVoltDetectInit();
    DevIOCtlInit();
    DevSpiFlashInit();
    DevSgm58031Init();
    DevHT7017Init();
    DevUartCommInit();
	DevLedInit();
    CanProtoInit();
    
    BCUSoxInit();
    TimerPollingInit();
    DataStorageInfoInit();
    
//    EventLogInit(EX_FLASH_EVENT_LOG_START_ADDR,EX_FLASH_EVENT_LOG_SIZE,(FLASH_READ_WRITE_FUN)SpiFlashReadData,(FLASH_READ_WRITE_FUN)SpiFlashWriteBytesWithErase);

	while(1) 
    {
		//FWDGT_Feed();
		LedCtlBlink(LED_SRC_DEBUG, 1000);	
        DevRtcProcess();
        DevVoltDetectProcess();
        DevHT7017Process();
        DevUartCommDirCtlProcess();
        DevLedProcess();
        
        HighVoltDetectProcess();
        CurrentDetectProcess();
        TempDetectProcess();
        
        BMSCalcDataProcess();
        AlarmProcess();
        InsResDetectProcess();       
        SOXProcess();
        CanProtoProcess();
        
		RS485RxProcess();//工装测试485
        ScheStorageDataProcess();
        BMUCtlProcess();
		
		CurrentLimitProcess();
		ModbusSlaveRxProcess();
		SysRunProcess();
		FunctionTestProcess();
	}
}
//#if (PBUF_POOL_BUFSIZE <= MEM_ALIGNMENT)
//#error "PBUF_POOL_BUFSIZE must be greater than MEM_ALIGNMENT or the offset may take the full first pbuf"
//#endif
