/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        timer_polling.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-8-20
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-09-09    <td>V1.0.0    <td>          <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "timer_polling.h"
#include "bcu_sox.h"
#include "current_detect.h"

static void _TimerUpIrqProcess(void);


/**
  * @brief   定时器轮询初始化
  * @param   None
  * @return  None
  * @note    None
  */
void TimerPollingInit(void)
{
//    TimerPollingHandle.timer_cfg->period = 5 * 1000;//目前轮询周期是10ms
    gTimerPollingHandle.irq_process = _TimerUpIrqProcess;
    TimerPollingBspInit();
} 



/**
* @brief   定时器轮询处理函数，很多对时间要求高的轮询业务放在此处
* @param   None
* @return  None
* @note    
*/
static void _TimerUpIrqProcess(void)
{
    static long cnt = 0;
    uint16_t poll_period = gTimerPollingHandle.timer_cfg->period / 1000;//轮询的周期
    
    if(gPackInfo.calc_period == 0)
    {
        return;
    }
    /*安时积分标志置位*/
    if(cnt % (gPackInfo.calc_period / poll_period) == 0)//300ms进行一次安时积分
    {
        /*电流获取处理*/
        if(gCurrentInfo.filter_big_range_current <= 28000 && gCurrentInfo.filter_big_range_current >= -28000)//电流较小，采用小量程值
        {
            gPackInfo.current = gCurrentInfo.filter_small_range_current;
        }
        else
        {
            gPackInfo.current = gCurrentInfo.filter_big_range_current;
        }  
        gPackInfo.calc_flag = true;
    }
    cnt++;
    
}







