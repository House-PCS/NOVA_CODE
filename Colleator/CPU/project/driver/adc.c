
#include "adc.h"

static DMA_InitType    _DMA_ADC_InitStructure[ADC_NUM];



static void _AdcClkInit()
{
#if USE_ADC1
    RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_ADC1, ENABLE);
#endif
#if USE_ADC2
    RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_ADC2, ENABLE);
#endif
    
    /* RCC_ADCHCLK_DIV16*/
    ADC_ConfigClk(ADC_CTRL3_CKMOD_AHB,RCC_ADCHCLK_DIV16);
    RCC_ConfigAdc1mClk(RCC_ADC1MCLK_SRC_HSE, RCC_ADC1MCLK_DIV8);  //selsect HSE as RCC ADC1M CLK Source	
}



static void _AdcGpioInit(Adc_HandleType *pHandle)
{
    uint8_t i;
    GPIO_InitType GPIO_InitStructure;
    for(i = 0; i < CHANNEL_NUM_PER_ADC; i++)
    {
        if(pHandle->adc_cfg->channel_config[i].enable == true && ADC_CHL(pHandle->adc_cfg->channel_config[i].chl_id) <= ADC_CH_18)
        {
            GPIO_InitStructure.Pin       = pHandle->adc_cfg->channel_config[i].adc_gpio_pinx;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
            GPIO_InitPeripheral(pHandle->adc_cfg->channel_config[i].adc_gpiox, &GPIO_InitStructure);
        }
    }
}

static void _DelayMs(uint16_t ms)
{
    soft_delayms(ms);
}

static void _AdcDMACfg(Adc_HandleType *pHandle)
{
    DMA_DeInit(pHandle->adc_cfg->dma_peri_channel);   
    
    _DMA_ADC_InitStructure[pHandle->adcx].MemAddr                  = (uint32_t)pHandle->pRData;
    _DMA_ADC_InitStructure[pHandle->adcx].MemDataSize              = DMA_MemoryDataSize_HalfWord;
    _DMA_ADC_InitStructure[pHandle->adcx].DMA_MemoryInc            = DMA_MEM_INC_ENABLE;
    _DMA_ADC_InitStructure[pHandle->adcx].PeriphAddr               = (uint32_t)&(pHandle->adc_cfg->adc_peri->DAT);//((uint32_t)pUartHandle->uart_cfg->uart_peri + 0x04)
    _DMA_ADC_InitStructure[pHandle->adcx].PeriphDataSize           = DMA_PERIPH_DATA_SIZE_HALFWORD;
    _DMA_ADC_InitStructure[pHandle->adcx].PeriphInc                = DMA_PERIPH_INC_DISABLE;
    _DMA_ADC_InitStructure[pHandle->adcx].Priority                 = DMA_PRIORITY_HIGH;
    _DMA_ADC_InitStructure[pHandle->adcx].Direction                = DMA_DIR_PERIPH_SRC;
    
    _DMA_ADC_InitStructure[pHandle->adcx].Mem2Mem                  = DMA_M2M_DISABLE;
    
    if(pHandle->continuous_num == 0)
    {
        _DMA_ADC_InitStructure[pHandle->adcx].BufSize                  = pHandle->chl_valid_number;
        _DMA_ADC_InitStructure[pHandle->adcx].CircularMode             = DMA_MODE_CIRCULAR;
    }
    else
    {
        _DMA_ADC_InitStructure[pHandle->adcx].BufSize                  = pHandle->chl_valid_number * pHandle->continuous_num;
        _DMA_ADC_InitStructure[pHandle->adcx].CircularMode             = DMA_MODE_NORMAL;
    }
    
    DMA_Init(pHandle->adc_cfg->dma_peri_channel, &_DMA_ADC_InitStructure[pHandle->adcx]);
    if(pHandle->adc_cfg->adc_peri == ADC1)
    {
        DMA_RequestRemap(DMA1_REMAP_ADC1, pHandle->adc_cfg->dma_peri, pHandle->adc_cfg->dma_peri_channel, ENABLE);
    }
    else
    {
        DMA_RequestRemap(DMA1_REMAP_ADC2, pHandle->adc_cfg->dma_peri, pHandle->adc_cfg->dma_peri_channel, ENABLE);
    }
    
    if(pHandle->adc_cfg->scan_mode == true)
    {
        pHandle->adc_cfg->en_dma = true;
        if(pHandle->continuous_num != 0)
        {
            pHandle->adc_cfg->en_dma_irq = true;
        }
    }
    
    if(pHandle->adc_cfg->en_dma == true)
    {
        if(pHandle->adc_cfg->en_dma_irq == true)
        {
            NVIC_InitType NVIC_InitStructure;
            
            NVIC_InitStructure.NVIC_IRQChannel                   = pHandle->adc_cfg->nvic_dma_irq;
            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = pHandle->adc_cfg->irq_dma_prio;
            NVIC_InitStructure.NVIC_IRQChannelSubPriority        = pHandle->adc_cfg->irq_dma_sub_prio;
            NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
            NVIC_Init(&NVIC_InitStructure);
            DMA_ConfigInt(pHandle->adc_cfg->dma_peri_channel, DMA_INT_TXC, ENABLE);/*当前驱动版本DMA仅支持完成中断，后续可增加其他中断种类*/
        }
    }
    
//    /* enable DMA channel */
//    DMA_EnableChannel(pHandle->adc_cfg->dma_peri_channel, ENABLE);  
}


void AdcInit(Adc_HandleType *pHandle)
{
    uint8_t i;
    ADC_InitType     ADC_InitStructure;
    
    pHandle->chl_valid_number = 0;
    
    for(i = 0; i < CHANNEL_NUM_PER_ADC; i++)
    {
        if(pHandle->adc_cfg->channel_config[i].enable == true)
        {
            pHandle->chl_valid_number++;
        }
    }
    if(pHandle->chl_valid_number == 0)
    {
        return;
    }
    
    _AdcClkInit();
    _AdcGpioInit(pHandle);
    
    if(pHandle->adc_cfg->en_dma == true)
    {
        pHandle->adc_cfg->dma_peri_channel = GetDMAPeriChannel(pHandle->adc_cfg->dma_peri, pHandle->adc_cfg->dma_channel);
    }

    /*ADC配置*/
    ADC_InitStructure.WorkMode          = ADC_WORKMODE_INDEPENDENT;
    if(pHandle->adc_cfg->scan_mode == true)
    {
        ADC_InitStructure.MultiChEn      = ENABLE;
        ADC_InitStructure.ChsNumber      = pHandle->chl_valid_number;
    }
    else
    {
        ADC_InitStructure.MultiChEn      = DISABLE;
        ADC_InitStructure.ChsNumber      = 1;
    }
    if(pHandle->adc_cfg->continuous_mode == true)
    {
        ADC_InitStructure.ContinueConvEn = ENABLE;
    }
    else
    {
        ADC_InitStructure.ContinueConvEn = DISABLE;
    }
    ADC_InitStructure.ExtTrigSelect  = ADC_EXT_TRIGCONV_NONE;
    ADC_InitStructure.DatAlign       = ADC_DAT_ALIGN_R;
    
    ADC_Init(pHandle->adc_cfg->adc_peri, &ADC_InitStructure);
    
    if(pHandle->adc_cfg->scan_mode == true)
    {
        uint8_t j = 1;
        for(i = 0; i < CHANNEL_NUM_PER_ADC; i++)
        {
            if(pHandle->adc_cfg->channel_config[i].enable == true)
            {
                ADC_ConfigRegularChannel(pHandle->adc_cfg->adc_peri, pHandle->adc_cfg->channel_config[i].chl_id, j, ADC_SAMP_TIME_55CYCLES5);
                j++;
            }
        }
    }
    else
    {
        for(i = 0; i < CHANNEL_NUM_PER_ADC; i++)
        {
            if(pHandle->adc_cfg->channel_config[i].enable == true)
            {
                break;
            }
        }
        if(i == CHANNEL_NUM_PER_ADC)
        {
            return;
        }
        else
        {
            pHandle->current_chl = pHandle->adc_cfg->channel_config[i].chl_id;
        }
        ADC_ConfigRegularChannel(pHandle->adc_cfg->adc_peri, pHandle->adc_cfg->channel_config[i].chl_id, 1, ADC_SAMP_TIME_55CYCLES5);
    }
    
    if(pHandle->adc_cfg->en_dma == true)
    {
        ADC_EnableDMA(pHandle->adc_cfg->adc_peri, ENABLE);
        _AdcDMACfg(pHandle);
    }
    
    
    
    /* Start ADC Software Conversion */
//    ADC_EnableSoftwareStartConv(pHandle->adc_cfg->adc_peri, ENABLE);
}




void AdcStartConv(Adc_HandleType *pHandle)
{
    /* Start ADC calibration */
    ADC_Enable(pHandle->adc_cfg->adc_peri, ENABLE);
    while(ADC_GetFlagStatusNew(pHandle->adc_cfg->adc_peri,ADC_FLAG_RDY) == RESET);
    ADC_StartCalibration(pHandle->adc_cfg->adc_peri);
    while (ADC_GetCalibrationStatus(pHandle->adc_cfg->adc_peri));
    
    if(pHandle->adc_cfg->en_dma == true)
    {
        DMA_Init(pHandle->adc_cfg->dma_peri_channel, &_DMA_ADC_InitStructure[pHandle->adcx]);
        DMA_EnableChannel(pHandle->adc_cfg->dma_peri_channel, ENABLE);
    }
    ADC_EnableSoftwareStartConv(pHandle->adc_cfg->adc_peri, ENABLE);
}


void AdcStopConv(Adc_HandleType *pHandle)
{
    ADC_EnableSoftwareStartConv(pHandle->adc_cfg->adc_peri, DISABLE);
    ADC_Enable(pHandle->adc_cfg->adc_peri, DISABLE);
}



/*单次运行模式或连续运行模式，非DMA模式使用， 扫描模式不可用*/
static bool _AdcValueGet(ADC_Module* adc_peri, uint8_t channel, uint16_t *p)
{
    uint32_t time_out = 0xFFFF;

    ADC_EnableSoftwareStartConv(adc_peri, ENABLE);
    while(!ADC_GetFlagStatus(adc_peri, ADC_FLAG_ENDC)) //检查转换标志
    {
        time_out--;
        if(time_out == 0)
        {
            return false;
        }
    }
    ADC_ClearFlag(adc_peri,ADC_FLAG_ENDC);
    ADC_ClearFlag(adc_peri,ADC_FLAG_STR);

    *p = ADC_GetDat(adc_peri);
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
    if(DMA_GetIntStatus(DMA1_INT_TXC1 << pHandle->adc_cfg->dma_channel * 4, pHandle->adc_cfg->dma_peri))
    {
        DMA_ClrIntPendingBit(DMA1_INT_TXC1 << pHandle->adc_cfg->dma_channel * 4, pHandle->adc_cfg->dma_peri);
        pHandle->dma_complete_process();
    }
    /*DMA传输错误*/
    else if(DMA_GetIntStatus(DMA1_INT_ERR1 << pHandle->adc_cfg->dma_channel * 4, pHandle->adc_cfg->dma_peri))
    {
        DMA_ClrIntPendingBit(DMA1_INT_ERR1 << pHandle->adc_cfg->dma_channel * 4, pHandle->adc_cfg->dma_peri);
    }
}









