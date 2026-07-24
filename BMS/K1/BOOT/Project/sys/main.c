/********************************************************************************
  * 文 件 名: main.c
  * 版 本 号: 初版
  * 修改作者: LC
  * 修改日期: 2022年04月12日
  * 功能介绍:          
  ******************************************************************************
  * 注意事项:

20231102 加入了和MPPT modbus 485通讯
20231102 modebus 读写驱动已经完成
20231108 增加升级MPPT code 基本完成 firmware_updata.c  firmware_updata.h
*********************************************************************************/
#include "main.h"
//#include "EventRecorder.h"
#define DEBUG                   (0)
#define LOG_TAG                 ("main")
#define OFFSET_VECTORTABLE      ((uint32_t)0x08000000)

//#define VERINFO_ADDR_BASE       (0x0800FF00) //存放FLASH的地址

//const char Software_Ver[]  __attribute__((at(VERINFO_ADDR_BASE + 0x00))) = "Software: 1.0.0";
//const char Compiler_Date[] __attribute__((at(VERINFO_ADDR_BASE + 0x40))) = "Date: "__DATE__;
//const char Compiler_Time[] __attribute__((at(VERINFO_ADDR_BASE + 0x60))) = "Time: "__TIME__;
/************************************************
函数名称 ： main
功    能 ： 主函数
参    数 ： 无
返 回 值 ： 无
作    者 ： LC
*************************************************/
int main(void) 
{
    nvic_vector_table_set(NVIC_VECTTAB_FLASH, OFFSET_VECTORTABLE);
    nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);  // 优先级分组	    
    __set_PRIMASK(0);   // 打开总中断  
    systick_config();   // 滴答定时器初始化 
    /* enable clock */
    BspInit();
    PowerHoldOn();
    DebugInit();  
    /* 采集BoardId */
    BoardIdBspInit();   // ok 
	LedInit();
    SpiFlashInit();     // ok
//    SysInfoInit();      // ok
    cm_backtrace_init_u16("GD32F303", 123, 456);    
    ModbusRegInit();    // ok  
	FwdgtInit();        // ok  
    KeyInit();          // ok  
    
    DevModbusRTUInit(); 
 //       FwdgtOpenCmd();
    
	if(FwdgtFlagProcess() == 1)
	{
		enterbootmodejudge();
	}
    LOG_INFO(LOG_TAG,"boot start\n\r");
    while(1) 
    { 
		MCULedProcess();			
        KeyDetectProcess();
        DebugProcess(); 
        GetKeyValueProcess();
		UpCommonPackProcess();
        DownCommonPackProcess();
        DevModbusDirCtlProcess(EN_MODBUS_HOST_SLAVE_TYPE_HOST);
        DevModbusDirCtlProcess(EN_MODBUS_HOST_SLAVE_TYPE_SLAVE);
        SysResetProcess();
      
//        FeedDog();      
    }
}
