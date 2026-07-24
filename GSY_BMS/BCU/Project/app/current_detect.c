/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        current_detect.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-09-05
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-09-05    <td>V1.0.0    <td>          <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "current_detect.h"
#include "filter_calc.h"

#define HALL_G_SMALL    (667 / 10)
#define HALL_G_BIG      (57 / 10)
#define HALL_VO         2500

CurrentInfo_t gCurrentInfo;






/** 
* @brief    电流检测数据获取
* @param    none
* @return   none
* @note: 
*/
static void _CurrentDetectDataGet()
{
    uint16_t small_range_volt[CONT_SAMPLE_NUM];
    uint16_t big_range_volt[CONT_SAMPLE_NUM];
    uint16_t small_range_mid_volt, bit_range_mid_volt;

    
#if ADC_USE_CONT_MODE
    memcpy(small_range_volt, DevGetMultiContVoltValue(EN_DEV_VOLT_DETECT_CUR1), sizeof(small_range_volt));
    memcpy(big_range_volt, DevGetMultiContVoltValue(EN_DEV_VOLT_DETECT_CUR2), sizeof(big_range_volt));
#else
    for(uint16_t i = 0; i < CONT_SAMPLE_NUM; i++)
    {
        small_range_volt[i] = DevGetVoltValue(EN_DEV_VOLT_DETECT_CUR1);
        big_range_volt[i] = DevGetVoltValue(EN_DEV_VOLT_DETECT_CUR2);
    }
#endif  
    small_range_mid_volt = FilterCalcMedian(small_range_volt, CONT_SAMPLE_NUM);
    bit_range_mid_volt = FilterCalcMedian(big_range_volt, CONT_SAMPLE_NUM);
    /*Ip = ((5 / Uc) x Vout - Vo) / G, 其中Uc为实际给霍尔的供电电压，Vout为霍尔输出的电压，G为霍尔传感器的灵敏度系数*/
    gCurrentInfo.rt_small_range_current = (small_range_mid_volt * 2 - HALL_VO) * 1000 / HALL_G_SMALL;//x2的原因是MCU采集到的电压是霍尔输出的一半
    gCurrentInfo.rt_big_range_current = (bit_range_mid_volt * 2 - HALL_VO) * 1000 / HALL_G_BIG;//x2的原因是MCU采集到的电压是霍尔输出的一半
    gCurrentInfo.rt_small_range_current = gCurrentInfo.small_corr_coef_k * gCurrentInfo.rt_small_range_current + gCurrentInfo.small_corr_coef_b;
    gCurrentInfo.rt_big_range_current = gCurrentInfo.big_corr_coef_k * gCurrentInfo.rt_big_range_current + gCurrentInfo.big_corr_coef_b;
//    gCurrentInfo.rt_small_range_current = 0.8644 * gCurrentInfo.rt_small_range_current;
//    gCurrentInfo.rt_big_range_current = 0.8644 * gCurrentInfo.rt_big_range_current - 3416;
}



#define SEC_SAMP_FILT_PERIOD    20 //二次采样执行周期
#define SEC_SAMP_FILT_NUM       10 //二次采样多少次后开始滤波
/** 
* @brief    电流检测计算大轮询
* @param    none
* @return   none
* @note: 
*/
void CurrentDetectProcess()
{
    static uint32_t timecount = 0;
    static long small_cur_sum = 0;
    static long big_cur_sum = 0;
    static uint8_t cnt = 0;

    if((uint32_t)(tim_get_systime() - timecount) > SEC_SAMP_FILT_PERIOD)
    {
        timecount = tim_get_systime();
        _CurrentDetectDataGet();
        if(cnt == 0)
        {
            small_cur_sum = 0;
            big_cur_sum = 0;
        }
        small_cur_sum += gCurrentInfo.rt_small_range_current;
        big_cur_sum += gCurrentInfo.rt_big_range_current;
        cnt++;
        if(cnt >= SEC_SAMP_FILT_NUM)
        {
            cnt = 0;
            /*计算多次平均值*/
            gCurrentInfo.filter_small_range_current = small_cur_sum / SEC_SAMP_FILT_NUM;
            gCurrentInfo.filter_big_range_current = big_cur_sum / SEC_SAMP_FILT_NUM;
        }
    }
}






















