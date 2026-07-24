/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        adc.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2022-10-31
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2022-10-31    <td>V1.0.0    <td>           <td>InitialVersion 当前版本仅支持独立模式、规则通道、单次软件触发采样转换

  * </table>
  ******************************************************************************
  */


#include "adc.h"


void AdcClkInit()
{
#if USE_ADC0
    rcu_periph_clock_enable(RCU_ADC0);
#endif
#if USE_ADC1
    rcu_periph_clock_enable(RCU_ADC1);
#endif
#if USE_ADC2
    rcu_periph_clock_enable(RCU_ADC2);
#endif

    rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV4);

//    adc_deinit();
//    adc_sync_mode_config(ADC_SYNC_MODE_INDEPENDENT);
}


static void _AdcGpioInit(Adc_HandleType *pHandle)
{
    uint8_t i;
    for(i = 0; i < CHANNEL_NUM_PER_ADC; i++)
    {
        if(pHandle->adc_cfg->channel_config[i].enable == true && ADC_CHL(pHandle->adc_cfg->channel_config[i].chl_num) <= ADC_CHANNEL_15)
        {
            gpio_init(pHandle->adc_cfg->channel_config[i].adc_gpiox, GPIO_MODE_AIN, GPIO_OSPEED_MAX, pHandle->adc_cfg->channel_config[i].adc_gpio_pinx);
        }
    }

}

static void _DelayMs(uint16_t ms)
{
    soft_delayms(ms);
}


void AdcInit(Adc_HandleType *pHandle)
{
    uint8_t i;
    _AdcGpioInit(pHandle);
    
    adc_mode_config(ADC_MODE_FREE);
    adc_data_alignment_config(pHandle->adc_cfg->adc_peri, ADC_DATAALIGN_RIGHT);
    adc_special_function_config(pHandle->adc_cfg->adc_peri, ADC_CONTINUOUS_MODE, DISABLE);
    adc_special_function_config(pHandle->adc_cfg->adc_peri, ADC_SCAN_MODE, DISABLE);
    adc_channel_length_config(pHandle->adc_cfg->adc_peri, ADC_REGULAR_CHANNEL, 1);
    /* ADC trigger config */
    adc_external_trigger_source_config(pHandle->adc_cfg->adc_peri, ADC_REGULAR_CHANNEL, ADC0_1_2_EXTTRIG_REGULAR_NONE); 
    adc_external_trigger_config(pHandle->adc_cfg->adc_peri, ADC_REGULAR_CHANNEL, ENABLE);

    adc_enable(pHandle->adc_cfg->adc_peri);
    _DelayMs(1);

    /* ADC calibration and reset calibration */
    adc_calibration_enable(pHandle->adc_cfg->adc_peri);
    adc_regular_data_read(pHandle->adc_cfg->adc_peri);
}



static bool _AdcValueGet(uint32_t adc_peri, uint8_t channel, uint16_t *p)
{
    uint32_t time_out = 0xFFFF;

    adc_regular_channel_config(adc_peri, 0, channel, ADC_SAMPLETIME_55POINT5);
    adc_software_trigger_enable(adc_peri, ADC_REGULAR_CHANNEL);
    while(!adc_flag_get(adc_peri, ADC_FLAG_EOC)) //检查转换标志
    {
        time_out--;
        if(time_out == 0)
        {
            return false;
        }
    }
    adc_flag_clear(adc_peri, ADC_FLAG_EOC); // 清除结束标志

    *p = adc_regular_data_read(adc_peri);
    return true;
}


bool AdcVoltageGet(Adc_HandleType *pHandle)
{
    uint16_t value;
    uint8_t i = 0;

    if(pHandle->current_chl >= CHANNEL_NUM_PER_ADC)
    {
        return false;
    }
    for(i = 0; i < CHANNEL_NUM_PER_ADC; i++)
    {
        if(pHandle->adc_cfg->channel_config[i].enable == true && pHandle->adc_cfg->channel_config[i].chl_num == pHandle->current_chl)
        {
            break;
        }
    }

    if(i == CHANNEL_NUM_PER_ADC)
    {
        return false;
    }

    if(_AdcValueGet(pHandle->adc_cfg->adc_peri, ADC_CHL(pHandle->current_chl), &value) == false)
    {
        return false;
    }

    *pHandle->pRData = value;
    *pHandle->pRValtage = value * 3.3 / 4096;
    return true;

}




