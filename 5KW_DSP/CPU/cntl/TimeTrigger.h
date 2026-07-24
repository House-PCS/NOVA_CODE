#ifndef TIMETRIGGER_H
#define TIMETRIGGER_H
#include "F2837xD_device.h"

typedef struct time_trigger_stru_t
{
    uint32_t time;              //当前时间
    uint32_t time_last;         //上次时间
    uint32_t time_interval;     //时间间隔
    uint32_t trig_threshold;    //触发阈值
    uint8_t trigged;            //是否已触发
    void (*trig_hook)(void*);   //回调函数
    void* hook_param;           //回调参数
} TimeTrigger_t;

typedef struct delay_trigger_stru_t
{
    TimeTrigger_t time_trigger; //继承自定时触发器
    uint8_t input;              //输入
    uint8_t input_last;         //上次输入
    uint8_t output;             //输出
    uint8_t delay_interval;     //延时间隔
    char trig_dir;              //触发方向
    void (*trig_hook)(void*);   //回调函数
    void* hook_param;           //回调参数
} DelayTrigger_t;

typedef struct hold_trigger_stru_t
{
    TimeTrigger_t time_trigger; //继承自定时触发器
    uint8_t input;              //输入
    uint8_t input_last;         //上次输入
    uint8_t stable_level;       //保持的输出
    uint8_t output;             //输出
    uint8_t hold_interval;      //保持间隔
    void (*trig_hook)(void*);   //回调函数
    void* hook_param;           //回调参数
} HoldTrigger_t;


void TimeTriggerInit(TimeTrigger_t* p_trigger, uint32_t threshold);
void TimeTriggerClear(TimeTrigger_t* p_trigger);
void TimeTriggerRun(TimeTrigger_t* p_trigger);
void DelayTriggerRun(DelayTrigger_t* p_trigger, uint8_t input);
void DelayTriggerInit(DelayTrigger_t* p_trigger, uint32_t inverval);
void DelayTriggerClear(DelayTrigger_t* p_trigger);
void HoldTriggerRun(HoldTrigger_t* p_trigger, uint8_t input);
void HoldTriggerInit(HoldTrigger_t* p_trigger, uint32_t inverval, uint8_t stable_level);
void HoldTriggerClear(HoldTrigger_t* p_trigger);

#endif
