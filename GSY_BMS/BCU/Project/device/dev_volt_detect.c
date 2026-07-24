
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_volt_detect.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-5-16
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-6-9      <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "dev_volt_detect.h"

#define DEV_CONT_VOLT_SAMPLE_PERIOD  30

static float _gVoltageValue;
static uint16_t _gContSampleData[CONT_SAMPLE_NUM * EN_DEV_VOLT_DETECT_NUM];
static uint16_t _gContVoltMV[EN_DEV_VOLT_DETECT_NUM][CONT_SAMPLE_NUM];//单位MV

static bool _gContSampleBusyFlag;
static void _DMAAdc2IrqProcess();

/** 
* @brief    电压采样初始化
* @param    none
* @param    none
* @return   none
* @note:
*/
void DevVoltDetectInit()
{
    gAdc2Handle.continuous_num = CONT_SAMPLE_NUM;
    gAdc2Handle.pRData = _gContSampleData;//DMA连续模式
    gAdc2Handle.pRValtage = &_gVoltageValue;//单次模式
    gAdc2Handle.dma_complete_process = _DMAAdc2IrqProcess;
    Adc2BspInit();
}



/** 
* @brief    ADC DMA完成处理中断函数
* @param    none
* @return   none
* @note:
*/
static void _DMAAdc2IrqProcess()
{
    uint16_t i = 0, j = 0;
    AdcStopConv(&gAdc2Handle);
    /*数据转换成电压，mV*/
    for(i = 0; i < EN_DEV_VOLT_DETECT_NUM; i++)
    {
        for(j = 0; j < CONT_SAMPLE_NUM; j++)
        {
            _gContVoltMV[i][j] = gAdc2Handle.pRData[j * EN_DEV_VOLT_DETECT_NUM + i] * 3300 / 4096;
        }
    }
    _gContSampleBusyFlag = false;
}







/** 
* @brief    获取对应通道的电压值(单次模式下)
* @param    idx:需要获取哪一路采样
* @return   电压值，单位mV
* @note:
*/
uint16_t DevGetVoltValue(EN_DEV_VOLT_DETECT_IDX idx)
{
    uint16_t volt_mv;
    switch(idx)
    {
        case EN_DEV_VOLT_DETECT_BP:
        {
            gAdc2Handle.current_chl = ADC_CHANNEL_10;
            break;
        }
        case EN_DEV_VOLT_DETECT_PP:
        {
            gAdc2Handle.current_chl = ADC_CHANNEL_12;
            break;
        }
        case EN_DEV_VOLT_DETECT_CUR1:
        {
            gAdc2Handle.current_chl = ADC_CHANNEL_14;
            break;
        }
        case EN_DEV_VOLT_DETECT_CUR2:
        {
            gAdc2Handle.current_chl = ADC_CHANNEL_15;
            break;
        }
        default:
        {
            return 0;
        }            
    }
#if ADC_USE_CONT_MODE
    volt_mv = gAdc2Handle.pRData[idx] * 3300 / 4096;
#else
    AdcVoltageGet(&gAdc2Handle);
    volt_mv = *gAdc2Handle.pRValtage * 1000;
#endif
    return volt_mv;
}



/** 
* @brief    获取对应通道的电压值数组(连续DMA模式下)
* @param    idx:需要获取哪一路采样
* @return   多次连续采样电压值数组，单位mV
* @note:
*/
uint16_t *DevGetMultiContVoltValue(EN_DEV_VOLT_DETECT_IDX idx)
{
    return _gContVoltMV[idx];
}






/** 
* @brief    ADC DMA连续模式下数据处理大轮询
* @param    none
* @return   none
* @note:
*/
void DevVoltDetectProcess()
{
#if ADC_USE_CONT_MODE
    static uint32_t timecount = 0;
    if((uint32_t)(tim_get_systime() - timecount) > DEV_CONT_VOLT_SAMPLE_PERIOD)
    {
        timecount = tim_get_systime();
        if(_gContSampleBusyFlag == false)
        {
            AdcStartConv(&gAdc2Handle);
            _gContSampleBusyFlag = true;
        }
    }
#endif
}













