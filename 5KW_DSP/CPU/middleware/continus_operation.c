
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        continus_operation.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-12-16
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table  >
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-2-4    <td>V1.0.0    <td>zhangqi    <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "continus_operation.h"
#include "CountTrigger.h"


#define FREQ_RUN 10 //运行频率
#define TRIP_DELAY_TIME 580 //单位秒
// #define TRIP_DELAY_TIME 30 //单位秒
#define RECOVER_DELAY_TIME 60 //单位秒
float32 gHardLimitThreshold = 0.0f;
float32 gSoftLimitThreshold = 0.0f;
static uint16_t _gEnHardLimit = 0;
static uint16_t _gEnSoftLimit = 0;
static uint32_t _gHardLimitCount = 0; 
static uint16_t _gContinusTriggedFlag = 0;
uint16_t gHardLimitTriggerFlag = 0;
static uint16_t _gst = 0;

/**
* @brief: _set_trigger_continus
* @param: 
* @note: 置位持续运行触发标志位
*
*/
static void _set_trigger_continus(void* param)
{
    _gContinusTriggedFlag = 1; 
}
/**
* @brief: _set_trigger_continus
* @param: 
* @note: 复位持续运行触发标志位
*
*/
static void _clear_trigger_continus(void* param)
{
    _gContinusTriggedFlag = 0; 
}
static CountTrigger_t _gstOperation =                                                 \
{                                                                                          \
    .trigged = 0,                                                                          \
    .count = 0,                                                                            \
    .count_interval = 1,                                                                   \
    .hook_param = &_gst,                                                               \
    .trig_hook = _set_trigger_continus,                                                             \
    .trig_threshold = TRIP_DELAY_TIME * FREQ_RUN,                                                           \
};                                                                                         \
static CountTrigger_t _gstOperationRecover =                                                 \
{                                                                                          \
    .trigged = 0,                                                                          \
    .count = 0,                                                                            \
    .count_interval = 1,                                                                   \
    .hook_param = &_gst,                                                               \
    .trig_hook = _clear_trigger_continus,                                                             \
    .trig_threshold = RECOVER_DELAY_TIME * FREQ_RUN,                                                           \
};                                                                                         \
/**
* @brief: ContinusOperationInit
* @param: 
* @note: 初始化
*
*/
void ContinusOperationInit()
{
    CountTriggerInit(&_gstOperation, TRIP_DELAY_TIME * FREQ_RUN);
    _gstOperation.trig_hook = &_set_trigger_continus;
    CountTriggerInit(&_gstOperationRecover, RECOVER_DELAY_TIME * FREQ_RUN);
    _gstOperationRecover.trig_hook = &_clear_trigger_continus;
}
/**
* @brief: ContinusOperationCheck
* @param: check_en ：1为检查过压脱网，0为不使能；vrms:电网电压有效值；vrms_threshold：过压阈值
* @note: 持续过压检查函数
*
*/
void ContinusOperationCheck(float32 vrms, float32 vrms_threshold, uint16_t check_en)
{
    uint16_t en_flag = 0;
    if(( check_en == 0) || (_gContinusTriggedFlag == 1))
    {
        en_flag = 0;
        CountTriggerReset(&_gstOperation);
    }
    else
    {
        if(vrms > vrms_threshold)
        {
            en_flag = 1;
        }
        else
        {
            en_flag = 0;
        }
        CountTriggerRun(&_gstOperation,en_flag);
    }
}
/**
* @brief: ContinusOperationRecover
* @param: check_en ：1为复位重连，0为不使能；vrms:电网电压有效值；vrms_threshold：恢复阈值
* @note: 复位重连
*
*/
void ContinusOperationRecover(float32 vrms, float32 vrms_threshold, uint16_t check_en)
{
    uint16_t en_flag = 0;
    if((check_en == 0) || (_gContinusTriggedFlag == 0))
    {
        en_flag = 0;
        CountTriggerReset(&_gstOperationRecover);
    }
    else
    {
        if(vrms < vrms_threshold)
        {
            en_flag = 1;
        }
        else
        {
            en_flag = 0;
        }
        CountTriggerRun(&_gstOperationRecover,en_flag);
    }
}

uint16_t ReadContinusOperationState(void)
{
    return _gContinusTriggedFlag;
}
uint32_t ReadContinusOperationCount(void)
{
    return _gstOperation.count;
}
uint32_t ReadRecoverCount(void)
{
    return _gstOperationRecover.count;
}
