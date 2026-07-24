/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        adc.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2022-10-31
  * @brief		 GD32F4xx
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2022-10-31    <td>V1.0.0    <td>           <td>InitialVersion 当前版本仅支持独立模式、规则通道、单次软件触发采样转换
  * <tr><td>2023-5-16     <td>V1.0.0    <td>           <td>InitialVersion 当前版本增加了连续模式、扫描模式以及DMA机制
  * </table>
  ******************************************************************************
  */


#include "adc.h"


static dma_single_data_parameter_struct    _DMA_ADC_InitStructure[ADC_NUM];

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

//    rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV4);
    adc_clock_config(ADC_ADCCK_PCLK2_DIV4);
//    adc_deinit();
//    adc_sync_mode_config(ADC_SYNC_MODE_INDEPENDENT);
}


static void _AdcGpioInit(Adc_HandleType *pHandle)
{
    uint8_t i;
    for(i = 0; i < CHANNEL_NUM_PER_ADC; i++)
    {
        if(pHandle->adc_cfg->channel_config[i].enable == true && ADC_CHL(pHandle->adc_cfg->channel_config[i].chl_id) <= ADC_CHANNEL_15)
        {
            gpio_mode_set(pHandle->adc_cfg->channel_config[i].adc_gpiox, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, pHandle->adc_cfg->channel_config[i].adc_gpio_pinx);
        }
    }

}

static void _DelayMs(uint16_t ms)
{
    soft_delayms(ms);
}


static void _AdcDMACfg(Adc_HandleType *pHandle)
{
    rcu_periph_clock_enable(RCU_DMA0);

    _DMA_ADC_InitStructure[pHandle->adcx].periph_addr           = (uint32_t)(&ADC_RDATA(pHandle->adc_cfg->adc_peri));
    _DMA_ADC_InitStructure[pHandle->adcx].periph_inc            = DMA_PERIPH_INCREASE_DISABLE;
    _DMA_ADC_InitStructure[pHandle->adcx].memory0_addr          = (uint32_t)pHandle->pRData;
    _DMA_ADC_InitStructure[pHandle->adcx].memory_inc            = DMA_MEMORY_INCREASE_ENABLE;
    _DMA_ADC_InitStructure[pHandle->adcx].periph_memory_width   = DMA_PERIPH_WIDTH_16BIT;
    _DMA_ADC_InitStructure[pHandle->adcx].direction             = DMA_PERIPH_TO_MEMORY;
    _DMA_ADC_InitStructure[pHandle->adcx].priority              = DMA_PRIORITY_HIGH; 

    if(pHandle->continuous_num == 0)
    {
        _DMA_ADC_InitStructure[pHandle->adcx].number                = pHandle->chl_valid_number;
        _DMA_ADC_InitStructure[pHandle->adcx].circular_mode         = DMA_CIRCULAR_MODE_ENABLE;
    }
    else
    {
        _DMA_ADC_InitStructure[pHandle->adcx].number                = pHandle->chl_valid_number * pHandle->continuous_num;
        _DMA_ADC_InitStructure[pHandle->adcx].circular_mode         = DMA_CIRCULAR_MODE_DISABLE;
    }
        
    dma_single_data_mode_init(pHandle->adc_cfg->dma_peri, pHandle->adc_cfg->dma_channel, &_DMA_ADC_InitStructure[pHandle->adcx]);
    dma_channel_subperipheral_select(pHandle->adc_cfg->dma_peri, pHandle->adc_cfg->dma_channel, pHandle->adc_cfg->dma_subperi);
    
    if(pHandle->adc_cfg->scan_mode == true)
    {
        pHandle->adc_cfg->en_dma = true;
        if(pHandle->continuous_num != 0)
        {
            pHandle->adc_cfg->en_dma_irq = true;
            
            nvic_irq_enable(pHandle->adc_cfg->nvic_dma_irq, pHandle->adc_cfg->irq_dma_prio, pHandle->adc_cfg->irq_dma_sub_prio);  
            dma_interrupt_enable(pHandle->adc_cfg->dma_peri, pHandle->adc_cfg->dma_channel, DMA_CHXCTL_FTFIE);/*当前驱动版本DMA仅支持完成中断，后续可增加其他中断种类*/
        }
    }
    /* enable DMA channel */
//    dma_channel_enable(pHandle->adc_cfg->dma_peri, pHandle->adc_cfg->dma_channel);  
}




void AdcInit(Adc_HandleType *pHandle)
{
    uint8_t i;

    pHandle->chl_valid_number = 0;

    for(i = 0; i < CHANNEL_NUM_PER_ADC; i++)
    {
        if(pHandle->adc_cfg->channel_config[i].enable == true)
        {
            pHandle->chl_valid_number++;
        }
    }

    AdcClkInit();
    _AdcGpioInit(pHandle);

    adc_sync_mode_config(ADC_SYNC_MODE_INDEPENDENT);
    adc_data_alignment_config(pHandle->adc_cfg->adc_peri, ADC_DATAALIGN_RIGHT);

    if(pHandle->adc_cfg->continuous_mode == true)
    {
        adc_special_function_config(pHandle->adc_cfg->adc_peri, ADC_CONTINUOUS_MODE, ENABLE);
    }
    else
    {
        adc_special_function_config(pHandle->adc_cfg->adc_peri, ADC_CONTINUOUS_MODE, DISABLE);
    }

    if(pHandle->adc_cfg->scan_mode == true)
    {
        uint8_t j = 0;
        adc_special_function_config(pHandle->adc_cfg->adc_peri, ADC_SCAN_MODE, ENABLE);
        adc_channel_length_config(pHandle->adc_cfg->adc_peri, ADC_ROUTINE_CHANNEL, pHandle->chl_valid_number);
        pHandle->adc_cfg->en_dma = true;

        for(i = 0; i < CHANNEL_NUM_PER_ADC; i++)
        {
            if(pHandle->adc_cfg->channel_config[i].enable == true)
            {
                adc_routine_channel_config(pHandle->adc_cfg->adc_peri, j, pHandle->adc_cfg->channel_config[i].chl_id, ADC_SAMPLETIME_56);
                j++;
            }
        }
    }
    else
    {
        adc_special_function_config(pHandle->adc_cfg->adc_peri, ADC_SCAN_MODE, DISABLE);
        adc_channel_length_config(pHandle->adc_cfg->adc_peri, ADC_ROUTINE_CHANNEL, 1);

        for(i = 0; i < CHANNEL_NUM_PER_ADC; i++)
        {
            if(pHandle->adc_cfg->channel_config[i].enable == true)
            {
                break;
            }
        }
        if(i == CHANNEL_NUM_PER_ADC)
        {
            pHandle->current_chl = ADC_CHANNEL_0;
        }
        else
        {
            pHandle->current_chl = pHandle->adc_cfg->channel_config[i].chl_id;
        }
        adc_routine_channel_config(pHandle->adc_cfg->adc_peri, 0, pHandle->current_chl, ADC_SAMPLETIME_56);
    }


     
    /* ADC trigger config */
    adc_external_trigger_source_config(pHandle->adc_cfg->adc_peri, ADC_ROUTINE_CHANNEL, ADC_EXTTRIG_ROUTINE_T0_CH0); 
    adc_external_trigger_config(pHandle->adc_cfg->adc_peri, ADC_ROUTINE_CHANNEL, EXTERNAL_TRIGGER_DISABLE);

    if(pHandle->adc_cfg->en_dma == true)
    {
        if(pHandle->continuous_num == 0)//只有在DMA循环传输的情况下，才打开DDM(每个规则转换结束后自动产生新的DMA请求)
        {
            adc_dma_request_after_last_enable(pHandle->adc_cfg->adc_peri);
        }
        adc_dma_mode_enable(pHandle->adc_cfg->adc_peri);
        _AdcDMACfg(pHandle);
//        dma_channel_enable(pHandle->adc_cfg->dma_peri, pHandle->adc_cfg->dma_channel);  
    }

    adc_enable(pHandle->adc_cfg->adc_peri);
    _DelayMs(1);

    /* ADC calibration and reset calibration */
    adc_calibration_enable(pHandle->adc_cfg->adc_peri);
//    if(pHandle->adc_cfg->en_dma == true)
//    {
//        adc_software_trigger_enable(pHandle->adc_cfg->adc_peri, ADC_ROUTINE_CHANNEL);/*DMA模式(扫描连续运行模式)下一开始就开启转换*/
//    }
}


void AdcStartConv(Adc_HandleType *pHandle)
{
    /* Start ADC calibration */

    if(pHandle->adc_cfg->en_dma == true)
    {
        adc_dma_mode_enable(pHandle->adc_cfg->adc_peri);
        dma_single_data_mode_init(pHandle->adc_cfg->dma_peri, pHandle->adc_cfg->dma_channel, &_DMA_ADC_InitStructure[pHandle->adcx]);
        dma_channel_subperipheral_select(pHandle->adc_cfg->dma_peri, pHandle->adc_cfg->dma_channel, pHandle->adc_cfg->dma_subperi);
        dma_channel_enable(pHandle->adc_cfg->dma_peri, pHandle->adc_cfg->dma_channel);  
    }
    adc_software_trigger_enable(pHandle->adc_cfg->adc_peri, ADC_ROUTINE_CHANNEL);
}

void AdcStopConv(Adc_HandleType *pHandle)
{
//    adc_software_trigger_disable(pHandle->adc_cfg->adc_peri, ADC_ROUTINE_CHANNEL);
//    ADC_Enable(pHandle->adc_cfg->adc_peri, DISABLE);
    dma_channel_disable(pHandle->adc_cfg->dma_peri, pHandle->adc_cfg->dma_channel);  
}





/*单次运行模式或连续运行模式，非DMA模式使用， 扫描模式不可用*/
static bool _AdcValueGet(uint32_t adc_peri, uint8_t channel, uint16_t *p)
{
    uint32_t time_out = 0xFFFF;

    adc_routine_channel_config(adc_peri, 0, channel, ADC_SAMPLETIME_56);
    adc_software_trigger_enable(adc_peri, ADC_ROUTINE_CHANNEL);
    while(!adc_flag_get(adc_peri, ADC_FLAG_EOC)) //检查转换标志
    {
        time_out--;
        if(time_out == 0)
        {
            return false;
        }
    }
    adc_flag_clear(adc_peri, ADC_FLAG_EOC); // 清除结束标志

    *p = adc_routine_data_read(adc_peri);
    return true;
}

/*单次运行模式或连续运行模式，非DMA模式使用， 扫描模式不可用*/
bool AdcVoltageGet(Adc_HandleType *pHandle)
{
    uint16_t value;
    uint8_t i = 0;


    if(pHandle->adc_cfg->scan_mode == true)
    {
        return false;
    }

    if(pHandle->current_chl >= CHANNEL_NUM_PER_ADC)
    {
        return false;
    }
    for(i = 0; i < CHANNEL_NUM_PER_ADC; i++)
    {
        if(pHandle->adc_cfg->channel_config[i].enable == true && pHandle->adc_cfg->channel_config[i].chl_id == pHandle->current_chl)
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

    *pHandle->pRValtage = value * 3.3 / 4096;
    return true;

}



/**
  * @brief  ADC DMA Complete process.
  * @param  Adc_HandleType: ADC需要用户配置的参数
  * @return NULL
  * @note   none
  */
void AdcDMAIrqProcess(Adc_HandleType *pHandle)
{
    /*DMA传输完成*/
    if(dma_interrupt_flag_get(pHandle->adc_cfg->dma_peri, pHandle->adc_cfg->dma_channel, DMA_INT_FLAG_FTF))
    {
        dma_interrupt_flag_clear(pHandle->adc_cfg->dma_peri, pHandle->adc_cfg->dma_channel, DMA_INT_FLAG_FTF);
        pHandle->dma_complete_process();
    }
    /*DMA传输错误*/
    else if(dma_interrupt_flag_get(pHandle->adc_cfg->dma_peri, pHandle->adc_cfg->dma_channel, DMA_INT_FLAG_TAE))
    {
        dma_interrupt_flag_clear(pHandle->adc_cfg->dma_peri, pHandle->adc_cfg->dma_channel, DMA_INT_FLAG_TAE);
    }
}


