

/* Includes ------------------------------------------------------------------*/
#include "sensor_control.h"

#define LOG_TAG     "sensor_control"

void SensorInit(void)
{
    ADCBspInit();
}

bool PackVotageGet(void)
{
    uint8_t c_current_chl = ADC_CHANNEL_10;
    float RValtage = 0;

    GpioBitSet((&PMON_Handle));
    Adc_Handle.current_chl = c_current_chl;
    Adc_Handle.pRValtage = &RValtage;
    if(AdcVoltageGet(&Adc_Handle) == false)
    {
        return false; 
    }
    GpioBitReset((&PMON_Handle));
    RValtage = RValtage * 31;
//    printf("packV: %7.3fV\r\n", RValtage);
    LOG_INFO(LOG_TAG, "packV: %7.3fV", RValtage);
    
    return true;
}

bool Dc12VCurrentGet(void)
{
    uint8_t c_current_chl = ADC_CHANNEL_11;
    float RValtage = 0;

    Adc_Handle.current_chl = c_current_chl;
    Adc_Handle.pRValtage = &RValtage;
    if(AdcVoltageGet(&Adc_Handle) == false)
    {
        return false;
    }
//    printf("DC12V_c: %7.3fV\r\n", RValtage);
    LOG_INFO(LOG_TAG, "Dc12VCurrent: %7.3fV", RValtage);
    
    return true;
}

//bool MosTemperature1Get(void)
//{
//    uint8_t c_current_chl = ADC_CHANNEL_14;
//    float RValtage = 0;

//    Adc_Handle.current_chl = c_current_chl;
//    Adc_Handle.pRValtage = &RValtage;
//    if(AdcVoltageGet(&Adc_Handle) == false)
//    {
//        return false;
//    }
////    printf("DC12V_c: %7.3fV\r\n", RValtage);
//    LOG_INFO(LOG_TAG, "MosTemperature1: %7.3fV", RValtage);
//    
//    return true;
//}

//bool MosTemperature2Get(void)
//{
//    uint8_t c_current_chl = ADC_CHANNEL_15;
//    float RValtage = 0;

//    Adc_Handle.current_chl = c_current_chl;
//    Adc_Handle.pRValtage = &RValtage;
//    if(AdcVoltageGet(&Adc_Handle) == false)
//    {
//        return false;
//    }
////    printf("DC12V_c: %7.3fV\r\n", RValtage);
//    LOG_INFO(LOG_TAG, "MosTemperature2: %7.3fV", RValtage);
//    
//    return true;
//}

MSH_CMD_EXPORT(PackVotageGet, get 48v power);
MSH_CMD_EXPORT(Dc12VCurrentGet, get 12v Current);
