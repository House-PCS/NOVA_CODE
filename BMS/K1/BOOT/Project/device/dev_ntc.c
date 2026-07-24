#include "dev_ntc.h"
#include "math.h"


#define LOG_TAG                 ("ntc")
#define RTEMP_B		            (3450)
#define RTEMP25		            (10)//100
#define T25			            (25 + 273.15)

#define R_UP                    (15)

 #define LN			            (_appro_ln)
//#define LN			log

#define TEMPERATURE_GET_PERIOD  (100)

#define NTC_CHANNEL             (ADC_CHL_0)


static double _appro_ln(double a)
{
	uint8_t N = 15;//我们取前15+1项来估算
	uint8_t k, nk;
	double x, x2, y;
	
	x = (a - 1) / (a + 1);
	x2 = x * x;
	nk = 2*N + 1;
	y = 1.0 / nk;
	
	for(k = N; k > 0; k--)
	{
		nk -= 2;
		y = 1.0 / nk + x2 * y;
	}
	return 2.0 * x * y;
}

//单位K
static float RtempCal(float u_rtemp)
{
	float rtemp; 
	
    if(u_rtemp >= 3.3f)
    {
        u_rtemp = 3.299999f;
    }
	rtemp = R_UP * u_rtemp / (3.3f - u_rtemp);
	return rtemp;
}


float GetTemperature(float Rntc)
{
	float tmp1;
	float res;
	
	tmp1 = LN(Rntc / RTEMP25) / RTEMP_B + 1 / T25; 
	res = 1 / tmp1 - 273.15f;
	return res;
//    float N1,N2,N3,N4;
//    
//    N1 = (log(Rntc) - log(RTEMP25)) / RTEMP_B;
//    N2 = 1/T25 + N1;
//    N3 = 1 / N2;
//    N4 = N3 - 273.15;
//    return N4;
    
} 

float GetTemperature4200(float Rntc)
{
	float tmp1;
	float res;
	
	tmp1 = LN(Rntc / 100) / 4200 + 1 / T25; 
	res = 1 / tmp1 - 273.15f;
	return res;
//    float N1,N2,N3,N4;
//    
//    N1 = (log(Rntc) - log(RTEMP25)) / RTEMP_B;
//    N2 = 1/T25 + N1;
//    N3 = 1 / N2;
//    N4 = N3 - 273.15;
//    return N4;
    
}




/**
 * @brief  : 获取固定通道测量的温度
 * @para   : channel：通道
 * @note   : None
 * @return : 温度
 */
float TemperatureGet(uint8_t channel)
{
    float res;
    float    volt;
	float 	 rntc; 
    
//    adc_value = AdcGetData(channel);
    Adc_Handle.current_chl = channel;
    Adc_Handle.pRValtage = &volt;
    if(AdcVoltageGet(&Adc_Handle) == false)
    {
        return false;
    }
	rntc = RtempCal(volt);
	res  = GetTemperature(rntc);
    
    if(res > 30)
    {
        __NOP();
    }
    return res;
}

/**
 * @brief  : adc转换为温度
 * @para   : adc_vol：adc数值
             r_up   ：上拉电阻阻值
 * @note   : None
 * @return : 温度
 */
float TemperatureDataChange(float adc_vol, uint8_t r_up)
{
    float res;
    float    volt;
	float 	 rntc; 
    
    volt = adc_vol;
    if(volt >= 3.3f)
    {
        volt = 3.299999;
    }
	rntc = r_up * volt / (3.3f - volt);   //上拉电阻100K
    
	res  = GetTemperature(rntc);
    
    return res;
}

float TemperatureDataChange4200(float adc_vol, uint8_t r_up)
{
    float res;
    float    volt;
	float 	 rntc; 
    
    volt = adc_vol;
    if(volt >= 3.3f)
    {
        volt = 3.299999;
    }
	rntc = r_up * volt / (3.3f - volt);   //上拉电阻100K
    
	res  = GetTemperature4200(rntc);
    
    return res;
}


static void _ShowTempMessage()
{
    LOG_DEBUG(LOG_TAG, "ntc1:%7.3f C\r\n", TemperatureGet(ADC_CHANNEL_14));
    LOG_DEBUG(LOG_TAG, "ntc2:%7.3f C\r\n", TemperatureGet(ADC_CHANNEL_15));
}
MSH_CMD_EXPORT(_ShowTempMessage, ntc12);


//void TemperatureProcess()
//{
//    static uint32_t time = 0;
//    if(GetSysTick() - time >= TEMPERATURE_GET_PERIOD)
//    {
//        time = GetSysTick();
//        gSysInfo.current_temperature = TemperatureGet(NTC_CHANNEL);
//    }
//}







