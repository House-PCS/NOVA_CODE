

/* Includes ------------------------------------------------------------------*/
#include "sensor_control.h"
#include "dev_ntc.h"

#define LOG_TAG     "sensor_control"


/**
 * @brief  : 初始化
 * @para   : None
 * @note   : None
 * @return : None
 */
void SensorInit(void)
{
    if(geBMSType == EN_BMS_A0)
//	if(geBMSType == EN_BMS_EXP)//调试用，后期采用EN_BMS_A0
    {
        ADCBspInit();
        VbatAdcCtrlInit();
    }      
}

/**
 * @brief  : 电池电压获取
 * @para   : None
 * @note   : None
 * @return : 电池包电压
 */
uint32_t PackVotageGet(void)
{
    
    uint8_t c_current_chl = ADC_CHANNEL_11;
    float RValtage = 0;

//    GpioBitSet((&PMON_Handle));
    GpioBitSet((&VbatAdcCtrlHandle));
    Adc_Handle.current_chl = c_current_chl;
    Adc_Handle.pRValtage = &RValtage;
    if(AdcVoltageGet(&Adc_Handle) == false)
    {   /*检测失败，返回0XFFFF*/
        return 0XFFFF; 
    }
//    GpioBitReset((&PMON_Handle));
//    GpioBitReset((&VbatAdcCtrlHandle));
    RValtage = RValtage * 695 / 20;

//    printf("packV: %7.3fV\r\n", RValtage);
//    LOG_INFO(LOG_TAG, "packV: %7.3fV", RValtage);
    
    return (RValtage * 1000);
}

bool REF2_5VGet(void)
{
//    uint8_t c_current_chl = ADC_CHANNEL_15;
//    float RValtage = 0;

//    Adc_Handle.current_chl = c_current_chl;
//    Adc_Handle.pRValtage = &RValtage;
//    if(AdcVoltageGet(&Adc_Handle) == false)
//    {
//        return false;
//    }
////    printf("DC12V_c: %7.3fV\r\n", RValtage);
//    LOG_INFO(LOG_TAG, "REF2_5V: %7.3fV", RValtage);
    
    return true;
}
MSH_CMD_EXPORT(REF2_5VGet, get 2_5V REF);

bool REF1VGet(void)
{
    uint8_t c_current_chl = ADC_CHANNEL_15;
    float RValtage = 0;
    uint16_t voldata = 0;
    uint16_t ref2_5vdata = 0;

//    Adc_Handle.current_chl = c_current_chl;
//    Adc_Handle.pRValtage = &RValtage;
//    Adc_Handle.pRData = &ref2_5vdata;
//    if(AdcVoltageGet(&Adc_Handle) == false)
//    {
//        return false;
//    }
//    LOG_INFO(LOG_TAG, "REF2_5V: %7.3fV", RValtage);
//    c_current_chl = ADC_CHANNEL_8;
//    Adc_Handle.current_chl = c_current_chl;
//    Adc_Handle.pRData = &voldata;
//    if(AdcVoltageGet(&Adc_Handle) == false)
//    {
//        return false;
//    }
//    
//    RValtage = 2.5 / ref2_5vdata * voldata;
//    
////    printf("DC12V_c: %7.3fV\r\n", RValtage);
//    LOG_INFO(LOG_TAG, "REF1V: %7.3fV", RValtage);
    
    return true;
}
MSH_CMD_EXPORT(REF1VGet, get 1V REF);

uint32_t ExpVotageGet(void)
{
    uint8_t c_current_chl = ADC_CHANNEL_8;
    float RValtage = 0;

//    Adc_Handle.current_chl = c_current_chl;
//    Adc_Handle.pRValtage = &RValtage;
//    if(AdcVoltageGet(&Adc_Handle) == false)
//    {
//        return false; 
//    }
    
    RValtage = RValtage * 19;
    
    return (RValtage * 1000);
}

/**
 * @brief  : Mos管温度获取
 * @para   : None
 * @note   : None
 * @return : Mos管温度
 */
float MosTemperatureGet()
{
    return TemperatureGet(ADC_CHANNEL_14);
}

/**
 * @brief  : 环境温度获取
 * @para   : None
 * @note   : None
 * @return : 环境温度
 */
float TaTemperatureGet()
{
    return TemperatureGet(ADC_CHANNEL_13);
}

/**
 * @brief  : afe温度获取
 * @para   : None
 * @note   : None
 * @return : afe温度
 */
float AfeTemperatureGet()
{
    return TemperatureGet(ADC_CHANNEL_12);
}

MSH_CMD_EXPORT(PackVotageGet, get 48v power);

