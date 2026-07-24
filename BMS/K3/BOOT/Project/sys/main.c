/********************************************************************************
  * 文 件 名: main.c
  * 版 本 号: 初版
  * 修改作者: LC
  * 修改日期: 2022年04月12日
  * 功能介绍:
  ******************************************************************************
  * 注意事项:
*********************************************************************************/
#include "main.h"
#include "firmware_iap.h"

#define LOG_TAG     "main"
#define OFFSET_VECTORTABLE      ((uint32_t)0x08000000)
/************************************************
函数名称 ： main
功    能 ： 主函数
参    数 ： 无
返 回 值 ： 无
作    者 ： LC
*************************************************/

int main(void)
{
    __set_PRIMASK(1);   // 关闭总中断
    nvic_vector_table_set(NVIC_VECTTAB_FLASH, OFFSET_VECTORTABLE);
    nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);  // 优先级分组
    __set_PRIMASK(0);   // 打开总中断
    systick_config();   // 滴答定时器初始化
    soft_delayms(10);     
    BspInit();
    BMSBoardPowerOn();    
    DevFeedExtWdg();    
    soft_delayms(10);       
    CommunicationInit();
    SpiFlashInit();
    if(gFwdgtRstFlag == 0)
    {
        //不是由看门狗触发的重启，允许更新固件跳转
        enterbootmodejudge();
    }
    soft_delayms(10);
    while(1)
    {
        ComDataProcess();
        LedProcess();
        DebugProcess();
        DevFeedExtWdg();
    }
}

