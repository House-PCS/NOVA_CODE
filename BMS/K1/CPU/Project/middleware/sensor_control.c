

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
//  if(geBMSType == EN_BMS_EXP)//调试用，后期采用EN_BMS_A0
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
    soft_delayms(1);
    PackAdcHandle.current_chl = c_current_chl;
    PackAdcHandle.pRValtage = &RValtage;
    if(AdcVoltageGet(&PackAdcHandle) == false)
    {
        /*检测失败，返回0XFFFF*/
        return 0XFFFF;
    }
//    GpioBitReset((&PMON_Handle));
//    GpioBitReset((&VbatAdcCtrlHandle));
    RValtage = RValtage * 695 / 20;

//    printf("packV: %7.3fV\r\n", RValtage);
//    LOG_INFO(LOG_TAG, "packV: %7.3fV", RValtage);

    return (RValtage * 1000);
}

void OVCEnOpen(void)
{
    GpioBitSet((&OvcEnHandle));
}

void OVCEnClose(void)
{
    GpioBitReset((&OvcEnHandle));
}


/**
 * @brief  : OVC电压获取
 * @para   : n_sample_spaced_time : 采样间隔时间
 * @note   : None
 * @return : None
 */
int32_t OVCVoltageGet()
{
    float RValtage = 0;

    /*电压获取步骤：1.打开OVC使能，延迟2ms
                    2.adc数据采集，延迟2ms
                    3.关闭OVC使能，还原mos状态*/

    PackAdcHandle.current_chl  = ADC_CHANNEL_10;
    PackAdcHandle.pRValtage    = &RValtage;

    if(AdcVoltageGet(&PackAdcHandle) == false)
    {
        return false;
    }
    RValtage        = RValtage * 36360;        /* Vout = Vin * 120 / 3.3 * 1000 (mV)*/

    return RValtage;
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

/**
 * @brief  : 电池包NTC1温度获取
 * @para   : None
 * @note   : None
 * @return : Mos管温度
 */
float PackNtc1TemperatureGet()
{
    return TemperatureGet(ADC_CHANNEL_0);
}

/**
 * @brief  : 电池包NTC2温度获取
 * @para   : None
 * @note   : None
 * @return : Mos管温度
 */
float PackNtc2TemperatureGet()
{
    return TemperatureGet(ADC_CHANNEL_1);
}

/**
 * @brief  : 电池包NTC3温度获取
 * @para   : None
 * @note   : None
 * @return : Mos管温度
 */
float PackNtc3TemperatureGet()
{
    return TemperatureGet(ADC_CHANNEL_8);
}

/**
 * @brief  : 电池包NTC4温度获取
 * @para   : None
 * @note   : None
 * @return : Mos管温度
 */
float PackNtc4TemperatureGet()
{
    return TemperatureGet(ADC_CHANNEL_9);
}

MSH_CMD_EXPORT(PackVotageGet, get 48v power);

