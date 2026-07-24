/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        TimeTrigger.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-5-24
  * @brief       软件触发器
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-5-24    <td>V1.0.0    <td>dengjiahui    <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "TimeTrigger.h"
#include "cpu_timers.h"


/**
 * @brief 定时触发器初始化
 * @param p_trigger 触发器指针
 * @param threshold 触发间隔（ms)
 */
void TimeTriggerInit(TimeTrigger_t* p_trigger, uint32_t threshold)
{
    if(p_trigger == 0)
    {
        return;
    }
    p_trigger->time = 0;
    p_trigger->time_interval = 0;
    p_trigger->time_last = 0;
    p_trigger->trig_threshold = threshold;
    p_trigger->trig_hook = 0;
    p_trigger->trigged = 0;
}

void TimeTriggerClear(TimeTrigger_t* p_trigger)
{
    if(p_trigger == 0)
    {
        return;
    }
    p_trigger->time = 0;
    p_trigger->time_interval = 0;
    p_trigger->time_last = 0;
    p_trigger->trigged = 0;
}

/**
 * @brief 定时触发器运行
 * @param p_trigger 触发器指针
 */
void TimeTriggerRun(TimeTrigger_t* p_trigger)
{
    if(p_trigger == 0)
    {
        return;
    }
    p_trigger->time = TimGetSystime();
    if(p_trigger->time_last == 0)
    {
        p_trigger->time_last = p_trigger->time;
        return;
    }

    p_trigger->time_interval = TimDifftime(p_trigger->time_last, p_trigger->time);
    if(p_trigger->time_interval >= p_trigger->trig_threshold)
    {
        p_trigger->trigged = 1;
        if(p_trigger->trig_hook != 0)
        {
            p_trigger->trig_hook(p_trigger->hook_param);
        }
        p_trigger->time_last = p_trigger->time;
    }
}

/**
 * @brief 延时触发器触发动作
 * @param p_param 透传参数
 */
static void _DelayTriggerTrigged(void* p_param)
{
    if(p_param == 0)
    {
        return;
    }
    DelayTrigger_t* p_trigger = p_param;
    p_trigger->output = p_trigger->trig_dir == 1;
    p_trigger->trig_dir = 0;
    if(p_trigger->trig_hook!=0)
    {
        p_trigger->trig_hook(p_trigger->hook_param);
    }
}

/**
 * @brief 延时触发器初始化
 * @param p_trigger 触发器指针
 * @param inverval 延时时间（ms）
 */
void DelayTriggerInit(DelayTrigger_t* p_trigger, uint32_t inverval)
{
    if(p_trigger == 0)
    {
        return;
    }
    TimeTriggerInit((TimeTrigger_t*)p_trigger, inverval);
    p_trigger->time_trigger.trig_hook = _DelayTriggerTrigged;
    p_trigger->input = 0;
    p_trigger->input_last = 0;
    p_trigger->output = 0;
    p_trigger->delay_interval = inverval;
    p_trigger->trig_dir = 0;
    p_trigger->trig_hook = 0;
    ((TimeTrigger_t*)p_trigger)->hook_param=p_trigger;
    p_trigger->hook_param=0;
}

void DelayTriggerClear(DelayTrigger_t* p_trigger)
{
    if(p_trigger == 0)
    {
        return;
    }
    TimeTriggerClear((TimeTrigger_t*)p_trigger);
    p_trigger->input = 0;
    p_trigger->input_last = 0;
    p_trigger->output = 0;
    p_trigger->trig_dir = 0;
}

/**
 * @brief 延时触发器运行
 * @param p_trigger 触发器指针
 * @param input 触发器输入
 */
void DelayTriggerRun(DelayTrigger_t* p_trigger, uint8_t input)
{
    if(p_trigger == 0)
    {
        return;
    }
    p_trigger->input = input;
    if(p_trigger->input != p_trigger->input_last)
    {
        TimeTriggerInit((TimeTrigger_t*)p_trigger, p_trigger->delay_interval);
        p_trigger->time_trigger.trig_hook = _DelayTriggerTrigged;
        p_trigger->trig_dir = p_trigger->input == 1 ? 1 : -1;
    }

    p_trigger->input_last = p_trigger->input;
    if(p_trigger->trig_dir != 0)
    {
        TimeTriggerRun((TimeTrigger_t*)p_trigger);
    }


}

/**
 * @brief 保持恢复触发器触发动作
 * @param p_param 透传参数
 */
static void _HoldTriggerTrigged(void* p_param)
{
    if(p_param == 0)
    {
        return;
    }
    HoldTrigger_t* p_trigger = p_param;
    p_trigger->output = p_trigger->stable_level;
    if(p_trigger->trig_hook!=0)
    {
        p_trigger->trig_hook(p_trigger->hook_param);
    }
}

/**
 * @brief 保持恢复触发器初始化
 * @param p_trigger 触发器指针
 * @param inverval 保持多久后恢复
 * @param stable_level 要保持的输出
 */
void HoldTriggerInit(HoldTrigger_t* p_trigger, uint32_t inverval, uint8_t stable_level)
{
    if(p_trigger == 0)
    {
        return;
    }
    TimeTriggerInit((TimeTrigger_t*)p_trigger, inverval);
    p_trigger->time_trigger.trig_hook = _HoldTriggerTrigged;
    p_trigger->input = 0;
    p_trigger->input_last = 0;
    p_trigger->stable_level = stable_level;
    p_trigger->output = 0;
    p_trigger->hold_interval = inverval;
    p_trigger->trig_hook = 0;
    ((TimeTrigger_t*)p_trigger)->hook_param=p_trigger;
    p_trigger->hook_param=0;
}

void HoldTriggerClear(HoldTrigger_t* p_trigger)
{
    if(p_trigger == 0)
    {
        return;
    }
    TimeTriggerClear((TimeTrigger_t*)p_trigger);
    p_trigger->time_trigger.trig_hook = _HoldTriggerTrigged;
    p_trigger->input = 0;
    p_trigger->input_last = 0;
    p_trigger->output = 0;
}

/**
 * @brief 保持恢复触发器运行
 * @param p_trigger 触发器指针
 * @param input 触发器输入
 */
void HoldTriggerRun(HoldTrigger_t* p_trigger, uint8_t input)
{
    if(p_trigger == 0)
    {
        return;
    }
    p_trigger->input = input;
    if(p_trigger->input != p_trigger->input_last)
    {
        if(p_trigger->input != p_trigger->stable_level)
        {
            TimeTriggerInit((TimeTrigger_t*)p_trigger, p_trigger->hold_interval);
            p_trigger->time_trigger.trig_hook = _HoldTriggerTrigged;
        }
        p_trigger->output = p_trigger->input;
    }
    if(p_trigger->input != p_trigger->stable_level)
    {
        TimeTriggerRun((TimeTrigger_t*)p_trigger);
    }
    else
    {
        p_trigger->output = p_trigger->input;
    }
    p_trigger->input_last = p_trigger->input;
}
