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
#include "dev_atmos_sk68xx.h"
#include "key_backlight_led.h"
#include "atmos_led.h"
#include "bsp.h"

static void _TimerUpIrqProcess(void);
/**
  * @brief   定时器轮询初始化
  * @param   None
  * @return  None
  * @note    None
  */
void TimedPollingInit(void)
{
    TimerPollingHandle.irq_process = _TimerUpIrqProcess;
    TimerPollingBspInit();
}

//extern uint8_t timer_test;
//extern uint8_t flag;
/**
* @brief   定时器轮询处理函数，很多对时间要求高的轮询业务放在此处
* @param   None
* @return  None
* @note    这里只放轮询业务函数，具体业务如果轮询时间更长，则在其轮询内部进行计数再处理，时间是该轮询的倍数关系
*/
static void _TimerUpIrqProcess(void)
{
//    timer_test = 1;
//    if(timer_test == 1)
//    {
//        flag = 1;
//        timer_test = 0;
//    }
//    else 
//    {
//        flag = 0;
//        timer_test = 1;
//    }

//    SetTestIo();
   
//    DevAtmosLedProcess();
//    DevSyAtmosLedProcess();   //新芯片的轮询函数
    KeyLedProcess();
//    SideAtomsPolling();
//    ReSetTestIo();
}




