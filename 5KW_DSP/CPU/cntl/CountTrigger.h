//
// Created by Nova004693 on 24-6-20.
//

#ifndef RTTHREAD_COUNTTRIGGER_H
#define RTTHREAD_COUNTTRIGGER_H

#include "F2837xD_device.h"

typedef struct count_trigger_stru_t {
    int32_t count;              //当前时间
    int32_t count_interval;              //当前时间
    uint32_t trig_threshold;    //触发阈值
    uint8_t trigged;            //是否已触发
    void (*trig_hook)(void *);   //回调函数
    void *hook_param;           //回调参数
} CountTrigger_t;

void CountTriggerInit(CountTrigger_t *p_trigger, uint32_t threshold);
void CountTriggerRun(CountTrigger_t *p_trigger, uint8_t value);
void CountTriggerReset(CountTrigger_t* p_trigger);

#endif //RTTHREAD_COUNTTRIGGER_H
