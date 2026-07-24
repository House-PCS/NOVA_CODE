/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        can.c
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
  * <tr><td>2022-11-10    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "bms_current.h"
#include "mcu_bsp.h"
#include "data_process.h"
#include "AppCfg.h"


#ifdef CURR_SHUNT       //分流器
#define CURRRNT_MODE  0
#elif defined (CURR_LOOP_SINGLE)    //电流环
#define CURRRNT_MODE  1 
#elif defined (CURR_LOOP_DOUBLE)    //双通道电流环
#define CURRRNT_MODE  2
#elif defined (CURR_OTHER)      //其他
#define CURRRNT_MODE  3
#endif

#ifndef CURRRNT_MODE
#define CURRRNT_MODE  3  
#endif

#if (CURRRNT_MODE == 0)       //分流器配置
#ifndef CURR_SHUNT_BLAS      //BSP
#define CURR_SHUNT_BLAS  15033  //0.1mV，目前需要校准
#endif
#ifndef CURR_SHUNT_GAIN
#define CURR_SHUNT_GAIN  (8.2*2.2)
#endif


#ifndef CURR_SHUNT_FSR      //ASP
#define CURR_SHUNT_FSR  100000
#endif
#ifndef CURR_SHUNT_SIGNAL
#define CURR_SHUNT_SIGNAL  500   //0.1mV
#endif

#define CURR_SHUNT_SPEC     (CURR_SHUNT_FSR/CURR_SHUNT_SIGNAL)

#define CURR_SHUNT_FILT_CALCULATE     4
#define CURR_SHUNT_FILT_CNT     (1<<CURR_SHUNT_FILT_CALCULATE)
#endif


#if (CURRRNT_MODE == 2)     //双量程电流环配置
#ifndef CURR_ADC_CH_SMALL
#define CURR_ADC_CH_SMALL       VCTA        //小量程通道
#endif
#ifndef CURR_ADC_CH_LARGE
#define CURR_ADC_CH_LARGE       VCTB        //大量程通道
#endif
#ifndef CURR_LOOP_SCOPE_SMALL
#define CURR_LOOP_SCOPE_SMALL       30000        //小量程范围，1mA
#endif
#ifndef CURR_LOOP_SPL_SMALL
#define CURR_LOOP_SPL_SMALL     667     //0.1mV
#endif
#ifndef CURR_LOOP_SPL_LARGE
#define CURR_LOOP_SPL_LARGE     57     //0.1mV
#endif
#endif



static BmsCurrentType gBmsCurrent={0};
const BmsCurrentType *const IR_BmsCurrent = &gBmsCurrent;

#if (CURRRNT_MODE == 3)

#endif

#define KALMAN_FILT_CURR     //使能电流卡尔曼滤波
#ifdef KALMAN_FILT_CURR
KalmanParaType Kf_Para = {
    .kalman = 0,
    .p = 10,
    .q = 0.002,
    .r = 0.05, 
    .kg = 0,
};
#endif

//#define CURR_TEST_CNT       500
//char test_buff[500]={0};


static int32_t OtherCurr = 0;
/** 
* @brief  电流采集主函数
* @param  none
* @return  none
* @note:
*
*/
void BmsCurrentMain()
{
    static uint32_t curr_times=0;
    uint32_t curr_valt=0;   //0.1mV
    
//    static float curr_valt_test[CURR_TEST_CNT]={0};
//    static int16_t curr_test[CURR_TEST_CNT]={0};
//    static uint16_t test_num = 0;
    
    if(tim_get_systime() - curr_times <1)
    {
        return;
    }
#if (CURRRNT_MODE == 0)
{
    static uint16_t adc_data[CURR_SHUNT_FILT_CNT]={0};
    static uint8_t data_cnt = 0;
    static bool data_ready = false;
    uint32_t temp_u32 = 0;
    
//    curr_valt_test[data_cnt]=(pIR_Adc2Handle->pRValtage[SHUNT]-CURR_SHUNT_BLAS) /CURR_SHUNT_GAIN;
    
    adc_data[data_cnt++] = pIR_Adc2Handle->pRValtage[SHUNT];
    if(data_cnt>= CURR_SHUNT_FILT_CNT)
    {
        data_cnt = 0;
        data_ready = true;
    }
    if(data_ready)
    {
        for(uint8_t i = 0; i < CURR_SHUNT_FILT_CNT; i++)
        {
            temp_u32 += adc_data[i];
        }
        temp_u32 >>= CURR_SHUNT_FILT_CALCULATE;
//        curr_valt=(pIR_Adc2Handle->pRValtage[SHUNT]-CURR_SHUNT_BLAS) /CURR_SHUNT_GAIN;
        curr_valt=((int32_t)temp_u32-CURR_SHUNT_BLAS) /CURR_SHUNT_GAIN;
        gBmsCurrent.current=curr_valt*CURR_SHUNT_SPEC;
        if(ABS_DIFF(gBmsCurrent.current,0) <= 300)
        {
            gBmsCurrent.current=0;
        }
        
        curr_valt_test[test_num]=temp_u32;
        curr_test[test_num++]= gBmsCurrent.current;
        if(test_num>=CURR_TEST_CNT)
        {
            test_num = 0;
        }
    }
}
#elif (CURRRNT_MODE == 1)
{
    uint16_t temp_u16=0;
    
    temp_u16 = pIR_Adc2Handle->pRValtage[CURR_ADC_CH];    //0.1mV
    curr_valt = ABS_DIFF(temp_u16, 25000);
    gBmsCurrent.current = curr_valt*1000 / CURR_LOOP_SPL_LARGE;
    if(temp_u16 < 25000)
    {
        gBmsCurrent.current = -gBmsCurrent.current;
    }
}
#elif (CURRRNT_MODE == 2)
{
    uint16_t temp_u16=0;
    
//    temp_u16 = pIR_Adc2Handle->pRValtage[CURR_ADC_CH_LARGE];    //0.1mV
    curr_valt = ABS_DIFF(temp_u16, 25000);
    gBmsCurrent.current = curr_valt*1000 / CURR_LOOP_SPL_LARGE;
    if(gBmsCurrent.current < CURR_LOOP_SCOPE_SMALL)
    {
//        temp_u16 = pIR_Adc2Handle->pRValtage[CURR_ADC_CH_SMALL];
        curr_valt = ABS_DIFF(temp_u16, 25000);
        gBmsCurrent.current = curr_valt*1000 / CURR_LOOP_SPL_SMALL;
    }
    
    if(temp_u16 < 25000)
    {
        gBmsCurrent.current = -gBmsCurrent.current;
    }

}
#elif (CURRRNT_MODE == 3)
    gBmsCurrent.current=OtherCurr;
#endif
//int32_t temp32 = gBmsCurrent.current;
//////////////////curr滤波////////////////////////
#ifdef KALMAN_FILT_CURR
    gBmsCurrent.current = KalmanFilter(gBmsCurrent.current,&Kf_Para);
#endif
///////////////curr滤波——end////////////////////////

//sprintf(test_buff,"%f,%f\n",(float)temp32,(float)gBmsCurrent.current);
//uint16_t temp = strlen(test_buff);
//    VofaSend((uint8_t*)test_buff,strlen(test_buff));
    if(gBmsCurrent.current < 0)
    {
        gBmsCurrent.charge_integral-=gBmsCurrent.current*(tim_get_systime() - curr_times);
    }
    else if(gBmsCurrent.current > 0)
    {
        gBmsCurrent.discharge_integral+=gBmsCurrent.current*(tim_get_systime() - curr_times);
    }
    curr_times=tim_get_systime();   //必须在积分后面
}
/** 
* @brief  其他电流采集方式接口函数
* @param  curr：电流采集值，mA
* @return  none
* @note:
*
*/
void OtherCurrSet(int32_t curr)
{
    OtherCurr=curr;
}
/** 
* @brief  电流积分清除函数
* @param  none
* @return  none
* @note:
*
*/
void CurrIntegralClear()
{
    gBmsCurrent.charge_integral %= 3600000;        //SOC积分取其mAH,剩余部分继续累计
    gBmsCurrent.discharge_integral %= 3600000;
}

