/********************************************************************************
  * 文 件 名: main.c
  * 版 本 号: 初版
  * 修改作者: LC
  * 修改日期: 2022年04月12日
  * 功能介绍:          
  ******************************************************************************
  * 注意事项:
*********************************************************************************/
//#include "main.h"
//#include "EventRecorder.h"
//#include "protocol_can.h"
//#include "prot_can_asp.h"


#include "dev_spiflash.h"
#include "dev_pwm_detect_cfg.h"
#include "function_test.h"
#include "can_tx_rx_process.h"
#include "bmu_data_storage_manage.h"
#include "bmu_data_manage.h"
#include "dev_led.h"


#define LOG_TAG     "main"

#define OFFSET_VECTORTABLE      ((uint32_t)0x08008000)
//#define OFFSET_VECTORTABLE      ((uint32_t)0x08000000)
/************************************************
函数名称 ： main
功    能 ： 主函数
参    数 ： 无
返 回 值 ： 无
作    者 ： LC
*************************************************/
#define MEM_PRIV_SIZE   (1024*2)
uint8_t MemBuff[MEM_PRIV_SIZE] = {0};

#define max 1024

uint8_t NameID;
int main(void) 
{    
    __set_PRIMASK(1);   // 关闭总中断
    nvic_vector_table_set(NVIC_VECTTAB_FLASH, OFFSET_VECTORTABLE);
    nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);  // 优先级分组	
    
    systick_config();   // 滴答定时器初始化    
    /* enable clock */
    rcu_periph_clock_enable(RCU_PMU);
    BaseBspInit();
    DevIOCtlInit();
    DevSpiFlashInit();
    DevPwmInOutInit();
    CanProtoInit();
    DevLedInit();
    DevAdbms6830Init();
    
    __set_PRIMASK(0);   // 打开总中断  
    DataStorageInfoInit();

	delay_1ms(200);
    while(1) 
    {
        LedCtlBlink(LED_SRC_DEBUG, 1000);
        DevLedProcess();
        CanProtoProcess();
        AFECtlProcess();
        FunctionTestProcess();
        BMSCalcDataProcess();
    }
}
