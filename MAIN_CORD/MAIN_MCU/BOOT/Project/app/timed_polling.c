/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        timed_polling.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-1-25
  * @brief       定时器轮询，需要较为准确的定时轮询业务放在定时器中断中，防止主
                 循环部分业务占用时间过长导致其他业务无法定时执行
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-1-25     <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "timed_polling.h" 
#include "dev_mbi502x_led.h"
#include "dev_atmos_sk68xx.h"
#include "atmos_led.h"
#include "dev_key.h"
#include "key_backlight_led.h"

static void _TimerUpIrqProcess(void);

/**
  * @brief   定时器轮询初始化
  * @param   None
  * @return  None
  * @note    None
  */
void TimedPollingInit(void)
{
//    TimerPollingHandle.timer_cfg->period = 5 * 1000;//目前轮询周期是10ms
    TimerPollingHandle.irq_process = _TimerUpIrqProcess;
    TimerPollingBspInit();
}    
void TimePollingDeint(void)
{
    TimerPollingBspDeInit();
}

/**
* @brief   定时器轮询处理函数，很多对时间要求高的轮询业务放在此处
* @param   None
* @return  None
* @note    这里只放轮询业务函数，具体业务如果轮询时间更长，则在其轮询内部进行计数再处理，时间是该轮询的倍数关系
*/
extern void AtmosLedFuncUpdateTest(void);
static void _TimerUpIrqProcess(void)
{
//    KeyLedProcess();
	DevKeyDetectProcess();
//	DevAtmosLedProcess();
//	AtmosLedFuncUpdateTest();
	//AtmosLedPackUpdateTest();
//	AtmosLedFuncUpdate();
//	TouchKeyLedFuncUpdate();
//    DevMBI502xProcess();
}











  
