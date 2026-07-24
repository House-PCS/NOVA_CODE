/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        bcu_sox.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-8-20
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-8-20    <td>V1.0.0    <td>          <td>InitialVersion
  * </table>
  ******************************************************************************
  */
  
#include "bcu_sox.h"
#include "systick.h"
#include "current_detect.h"
#include "bms_data_manage.h"

#define PACK_CURRENT_MIN_THRESHOLD      500/*mA*/

//平台期定义
#define PACK_PLATE_MIN_VOLTAGE          3250
#define PACK_PLATE_MAX_VOLTAGE          3350

PackInfo_t gPackInfo;

//static uint8_t _gSoxInitFlag = 0;

/*OCV,单位mV*/
#define OCV_SOC_TEMPERATURE_LEVEL       9
#define OCV_SOC_LEVEL                   21

const uint16_t OCVSOCDefault[OCV_SOC_TEMPERATURE_LEVEL][OCV_SOC_LEVEL] =
{
    {3225, 3226, 3226, 3227, 3227, 3228, 3229, 3229, 3230, 3231, 3232, 3233, 3235, 3236, 3238, 3240, 3242, 3245, 3250, 3258, 3342},
    {3230, 3231, 3232, 3233, 3234, 3236, 3237, 3238, 3240, 3242, 3243, 3245, 3247, 3250, 3252, 3256, 3261, 3269, 3286, 3295, 3389},
    {3225, 3230, 3235, 3238, 3241, 3244, 3247, 3249, 3252, 3255, 3259, 3263, 3267, 3270, 3274, 3277, 3280, 3286, 3304, 3325, 3373},
    {3220, 3230, 3238, 3243, 3247, 3251, 3256, 3260, 3264, 3269, 3275, 3281, 3287, 3291, 3295, 3298, 3300, 3302, 3322, 3356, 3358},
    {3215, 3229, 3240, 3248, 3254, 3259, 3265, 3271, 3276, 3283, 3291, 3299, 3306, 3312, 3316, 3319, 3320, 3319, 3339, 3386, 3343},
    {3158, 3210, 3225, 3237, 3249, 3259, 3266, 3272, 3277, 3283, 3289, 3295, 3301, 3307, 3312, 3316, 3318, 3319, 3329, 3352, 3358},
    {3103, 3192, 3210, 3227, 3246, 3259, 3268, 3274, 3279, 3284, 3288, 3292, 3298, 3304, 3309, 3314, 3318, 3319, 3319, 3318, 3374},
    {2688, 2702, 2818, 3101, 3206, 3226, 3249, 3270, 3280, 3290, 3294, 3296, 3299, 3308, 3320, 3325, 3327, 3327, 3328, 3326, 3371},
    {2514, 2564, 2647, 3080, 3204, 3220, 3247, 3267, 3282, 3291, 3295, 3298, 3302, 3311, 3323, 3327, 3329, 3329, 3329, 3329, 3370},
};
const int8_t OCVTemperatureIndex[OCV_SOC_TEMPERATURE_LEVEL] = { -20, -10, -5, 0, 5, 10, 25, 40, 50};
const uint8_t OCVSOCIndex[OCV_SOC_LEVEL] = {0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100};




/** 
* @brief  SOX模块初始化
* @param  none
* @return  NULL
* @note:上电获取上次所保存的参数，以及默认数据的初始化
*/
void BCUSoxInit()
{
    /*从EEPROM读取SOC、SOH等信息*/
    
    /*从FLASH读取默认参数等信息*/
    
    /*从EEPROM读取累计充放电量信息，并基于RTC进行时间判断*/
    
    
    /*部分实时参数初始化*/
    gPackInfo.q_real = gPackInfo.pack_fact_para.q_max * gPackInfo.sox_info.soh / 100;
    
    //下方代码先放这里，后续需要在各类初始化完成后再赋值
    gPackInfo.chg_max_current = gPackInfo.q_real * gPackInfo.pack_fact_para.chg_max_rate / 10;
    gPackInfo.dsg_max_current = gPackInfo.q_real * gPackInfo.pack_fact_para.dsg_max_rate / 10;
    
    /*安时积分计算周期*/
    gPackInfo.calc_period = 100;//300ms
    
//    _gSoxInitFlag = 1;
}


/** 
* @brief  从控采样信息获取到，可以执行安时积分和OCV了
* @param  none
* @return  NULL
* @note:  当BCU已经从BMU端获取到电压信息后再调用，将_gSoxInitFlag置1
*/
//void BMUInfoGet()
//{
//    _gSoxInitFlag = 1;
//}
    

/** 
* @brief  电流方向判断及OCV计时标记
* @param  none
* @return  NULL
* @note:  需要轮询实时调用或者在电流采样处调用
*/
void PackCurrentModeJudge()
{

    if(gPackInfo.current < PACK_CURRENT_MIN_THRESHOLD && gPackInfo.current > (0 - PACK_CURRENT_MIN_THRESHOLD))
    {
        gPackInfo.mode = EN_PACK_CURRENT_IDLE_MODE;
    }
    else if(gPackInfo.current > 0)
    {
        gPackInfo.mode = EN_PACK_CURRENT_DISCHARGE_MODE;
        gPackInfo.sox_info.ocv_time = GetRtcTimeStamp();//OCV计时更新，确保在充放电不触发静态超过2小时的情况
    }
    else
    {
        gPackInfo.mode = EN_PACK_CURRENT_CHARGE_MODE;
        gPackInfo.sox_info.ocv_time = GetRtcTimeStamp();//OCV计时更新，确保在充放电不触发静态超过2小时的情况
    }
}



/** 
* @brief  双线性插值(整数型)
* @param  x1,y1:插值下限，x2,y2:插值上限，x：插值自变量
* @return  y:x通过插值运算后对应的值
* @note:  
*/
static float _InterpolationOperation(int32_t x1, int32_t x2, int32_t y1, int32_t y2, int32_t x)
{
    float y;
    if(x1 == x2)
    {
        y = y1;
        return y;
    }

    y = (float)(y2 - y1) / (x2 - x1) * (x - x1) + y1;
    return y;
}

/** 
* @brief  双线性插值(浮点型)
* @param  x1,y1:插值下限，x2,y2:插值上限，x：插值自变量
* @return  y:x通过插值运算后对应的值
* @note:  
*/
static float _InterpolationOperationf(float x1, float x2, float y1, float y2, float x)
{
    float y;
    if(x1 == x2)
    {
        y = y1;
        return y;
    }

    y = (float)(y2 - y1) / (x2 - x1) * (x - x1) + y1;
    return y;
}

/** 
 * @brief  根据插值法，求对应的数在属于数组里哪个区间(int8_t型)
 * @param  value：要确定数组区间范围的数
 * @param  p_index_table：对应的数组
 * @param  table_level:数组成员数量
 * @param  p_index_d：根据value数算出的数组区间下限指针
 * @param  p_index_u：根据value数算出的数组区间上限指针
 * @return  y:x通过插值运算后对应的值
 * @note:  仅支持数组升序
 */
static void _GetTableRangeIndex8(int8_t value, const int8_t *p_index_table, uint8_t table_level, uint8_t *p_index_d, uint8_t *p_index_u)
{
    if(value <= p_index_table[0])
    {
        *p_index_d = 0;
        *p_index_u = 0;
    }
    else if(value > p_index_table[table_level - 1])
    {
        *p_index_d = table_level - 1;
        *p_index_u = table_level - 1;
    }
    else
    {
        for(uint8_t i = 1; i < table_level; i++)
        {
            if(value <= p_index_table[i])
            {
                *p_index_d = i - 1;
                *p_index_u = i;
                return;
            }
        }
    }
}

/** 
 * @brief  根据插值法，求对应的数在属于数组里哪个区间(uint16_t型)
 * @param  value：要确定数组区间范围的数
 * @param  p_index_table：对应的数组
 * @param  table_level:数组成员数量
 * @param  p_index_d：根据value数算出的数组区间下限指针
 * @param  p_index_u：根据value数算出的数组区间上限指针
 * @return  y:x通过插值运算后对应的值
 * @note:  仅支持数组升序
 */
static void _GetTableRangeIndex16u(uint16_t value, const uint16_t *p_index_table, uint8_t table_level, uint8_t *p_index_d, uint8_t *p_index_u)
{
    if(value <= p_index_table[0])
    {
        *p_index_d = 0;
        *p_index_u = 0;
    }
    else if(value > p_index_table[table_level - 1])
    {
        *p_index_d = table_level - 1;
        *p_index_u = table_level - 1;
    }
    else
    {
        for(uint8_t i = 1; i < table_level; i++)
        {
            if(value <= p_index_table[i])
            {
                *p_index_d = i - 1;
                *p_index_u = i;
                return;
            }
        }
    }
}



/** 
* @brief  差值法通过OCV电压换算SOC
* @param  ocv_voltage:OCV电压
* @return  SOC
* @note:  
*/
static float _OCV2SOCWithInterpolation(uint16_t ocv_voltage)
{
    const uint16_t *p_ocv_tempd, *p_ocv_tempu;
    uint8_t i = 0, j = 0;
    uint8_t soc_index;
    uint16_t soc_ret1, soc_ret2;

    int16_t temp_d, temp_u;
    uint8_t temp_index_d, temp_index_u;
    uint16_t soc_tempd_d, soc_tempd_u, soc_tempu_d, soc_tempu_u;
    uint8_t soc_index_tempd_d, soc_index_tempd_u, soc_index_tempu_d, soc_index_tempu_u;


    float soc_tempd_oi, soc_tempu_oi, soc_oi;
    uint16_t soc_ret;
    int8_t temperature = gPackInfo.cell_ave_temperature;
    
    _GetTableRangeIndex8(temperature, OCVTemperatureIndex, OCV_SOC_TEMPERATURE_LEVEL, &temp_index_d, &temp_index_u);
    temp_d = OCVTemperatureIndex[temp_index_d];
    temp_u = OCVTemperatureIndex[temp_index_u];
    
    p_ocv_tempd = OCVSOCDefault[temp_index_d];
    p_ocv_tempu = OCVSOCDefault[temp_index_u];

    _GetTableRangeIndex16u(ocv_voltage, p_ocv_tempd, OCV_SOC_LEVEL, &soc_index_tempd_d, &soc_index_tempd_u);
    soc_tempd_d = OCVSOCIndex[soc_index_tempd_d];
    soc_tempd_u = OCVSOCIndex[soc_index_tempd_u];
    soc_tempd_oi = _InterpolationOperation(p_ocv_tempd[soc_index_tempd_d], p_ocv_tempd[soc_index_tempd_u], soc_tempd_d, soc_tempd_u, ocv_voltage);


    _GetTableRangeIndex16u(ocv_voltage, p_ocv_tempu, OCV_SOC_LEVEL, &soc_index_tempu_d, &soc_index_tempu_u);
    soc_tempu_d = OCVSOCIndex[soc_index_tempu_d];
    soc_tempu_u = OCVSOCIndex[soc_index_tempu_u];
    soc_tempu_oi = _InterpolationOperation(p_ocv_tempu[soc_index_tempu_d], p_ocv_tempu[soc_index_tempu_u], soc_tempu_d, soc_tempu_u, ocv_voltage);

    soc_oi = _InterpolationOperationf(temp_d, temp_u, soc_tempd_oi, soc_tempu_oi, temperature);
    return soc_oi;
}

/** 
 * @brief  OCV校准
 * @param  type:0表示判断电压范围，在平台期不校准，1表示强制校准
 * @return  NULL
 * @note:
 */
void OCVCalibration(uint8_t type)
{
    uint16_t voltage = gPackInfo.cell_min_voltage;
    
    if(gPackInfo.rt_info_valid == false)
    {
        return;
    }
    
    if(type == 0)
    {
        if(voltage >= PACK_PLATE_MIN_VOLTAGE && voltage <= PACK_PLATE_MAX_VOLTAGE)//在平台期，不做OCV校准
        {
            return;
        }
    }
    
    gPackInfo.sox_info.soc_real = _OCV2SOCWithInterpolation(voltage);
    gPackInfo.sox_info.ocv_time = GetRtcTimeStamp();
    if(gPackInfo.sox_info.soc_real < gPackInfo.pack_fact_para.soc_max)
    {
        gPackInfo.full_flag = false;
    }
    if(gPackInfo.sox_info.soc_real > gPackInfo.pack_fact_para.soc_min)
    {
        gPackInfo.empty_flag = false;
    }
}
    


/** 
 * @brief  满充校准
 * @param  none
 * @return  NULL
 * @note:
 */
void FullChargeCalibration()
{
    uint16_t voltage = gPackInfo.cell_max_voltage;
    uint16_t max_chg_voltage = gPackInfo.pack_fact_para.cell_max_chg_voltage;
    
    if(gPackInfo.rt_info_valid == false)
    {
        return;
    }
    
    /*电压比SOC快*/
    if(voltage >= max_chg_voltage)
    {
        gPackInfo.sox_info.soc_real = gPackInfo.pack_fact_para.soc_max;
        gPackInfo.sox_info.soc_disp = gPackInfo.pack_fact_para.soc_max + 0.1f;
        gPackInfo.full_flag = true;
    }
    else if(voltage >= max_chg_voltage - 70) //减去的值可调整   
    {
        if(gPackInfo.sox_info.soc_real <= gPackInfo.pack_fact_para.soc_max - 1)
        {
            gPackInfo.sox_info.soc_real = gPackInfo.pack_fact_para.soc_max - 1;
        }
    }
    /*SOC比电压快*/
    if(voltage < max_chg_voltage - 160)//实际还没充满
    {
        if(gPackInfo.sox_info.soc_real >= gPackInfo.pack_fact_para.soc_max)
        {
            gPackInfo.sox_info.soc_real = gPackInfo.pack_fact_para.soc_max - 0.6f;
        }
        if(gPackInfo.sox_info.soc_disp >= gPackInfo.pack_fact_para.soc_max)
        {
            gPackInfo.sox_info.soc_disp = gPackInfo.pack_fact_para.soc_max - 0.6f;
        }
    }
    
    
    if(gPackInfo.sox_info.soc_real <= gPackInfo.pack_fact_para.soc_max - 1.5)
    {
        gPackInfo.full_flag = false;
    }
    else if(gPackInfo.sox_info.soc_real >= gPackInfo.pack_fact_para.soc_max)
    {
        gPackInfo.full_flag = true;
    }
    
}


/** 
 * @brief  空放校准
 * @param  none
 * @return  NULL
 * @note:
 */
void EmptyDischargeCalibration()
{
    uint16_t voltage = gPackInfo.cell_min_voltage;
    uint16_t min_dsg_voltage = gPackInfo.pack_fact_para.cell_min_dsg_voltage;
    
    if(gPackInfo.rt_info_valid == false)
    {
        return;
    }
    
    /*电压比SOC快*/
    if(voltage <= min_dsg_voltage)
    {
        gPackInfo.sox_info.soc_real = gPackInfo.pack_fact_para.soc_min;
        gPackInfo.sox_info.soc_disp = gPackInfo.pack_fact_para.soc_min;
        gPackInfo.empty_flag = true;
    }
    else if(voltage <= min_dsg_voltage + 150) //加上的值可调整
    {
        if(gPackInfo.sox_info.soc_real >= gPackInfo.pack_fact_para.soc_min + 1)
        {
            gPackInfo.sox_info.soc_real = gPackInfo.pack_fact_para.soc_min + 1;
        }
    }
    /*SOC比电压快*/
    if(voltage > min_dsg_voltage + 300) //实际还没放空
    {
        if(gPackInfo.sox_info.soc_real <= gPackInfo.pack_fact_para.soc_min)
        {
            gPackInfo.sox_info.soc_real = gPackInfo.pack_fact_para.soc_min + 0.6f;
        }
        if(gPackInfo.sox_info.soc_disp <= gPackInfo.pack_fact_para.soc_min)
        {
            gPackInfo.sox_info.soc_disp = gPackInfo.pack_fact_para.soc_min + 0.6f;
        }
    }
    
    if(gPackInfo.sox_info.soc_real >= gPackInfo.pack_fact_para.soc_min + 1.5)
    {
        gPackInfo.empty_flag = false;
    }
    else if(gPackInfo.sox_info.soc_real <= gPackInfo.pack_fact_para.soc_min)
    {
        gPackInfo.empty_flag = true;
    }
}




/** 
 * @brief  安时积分计算SOC
 * @param  ms:该方法每个多长时间执行一次，单位ms
 * @return  NULL
 * @note:
 */
void SOCAmpereHourIntegral(uint32_t ms)
{
    float soc_max = gPackInfo.pack_fact_para.soc_max;
    float soc_min = gPackInfo.pack_fact_para.soc_min;
    int32_t current = gPackInfo.current;
    float soc_real = gPackInfo.sox_info.soc_real;
    float soc_disp = gPackInfo.sox_info.soc_disp;
    
    uint16_t max_voltage = gPackInfo.cell_max_voltage;
    uint16_t max_chg_voltage = gPackInfo.pack_fact_para.cell_max_chg_voltage;
    uint16_t min_voltage = gPackInfo.cell_min_voltage;
    uint16_t min_dsg_voltage = gPackInfo.pack_fact_para.cell_min_dsg_voltage;
    
    double soc_diff = 0;//安时积分计算的差值，正值表示在放电，负值表示在充电
    uint32_t cr = gPackInfo.pack_fact_para.q_max * gPackInfo.sox_info.soh / 100;
    
    float r1 = 1, r2 = 1;
    
    gPackInfo.q_real = cr;
    
    if(gPackInfo.rt_info_valid == false)
    {
        return;
    }
    PackCurrentModeJudge();
    
    switch(gPackInfo.mode)
    {
        case EN_PACK_CURRENT_IDLE_MODE:
        {
            goto CALIBRATION;
        }
        case EN_PACK_CURRENT_DISCHARGE_MODE:
        {
            /*修正系数r1*/
            if(soc_real - soc_disp >= 10)
            {
                r1 = 0.5;
            }
            else if(soc_real - soc_disp >= 5)
            {
                r1 = 0.6;
            }
            else if(soc_real - soc_disp >= 3)
            {
                r1 = 0.7;
            }
            else if(soc_real - soc_disp >= 1)
            {
                r1 = 0.9;
            }
            else if(soc_real - soc_disp <= -10)
            {
                r1 = 2.0;
            }
            else if(soc_real - soc_disp <= -5)
            {
                r1 = 1.8;
            }
            else if(soc_real - soc_disp <= -3)
            {
                r1 = 1.5;
            }
            else if(soc_real - soc_disp <= -1)
            {
                r1 = 1.1;
            }
            else
            {
                r1 = 1;
            }
            /*修正系数r2*/
            if(soc_real < 30)
            {
                if(soc_disp - soc_real > 0.5f)
                {
                    r2 = 1.8;
                }
                else
                {
                    r2 = 1;
                }
            }
            else
            {
                r2 = 1;
            }
            break;
        }
        case EN_PACK_CURRENT_CHARGE_MODE:
        {
            /*修正系数r1*/
            if(soc_real - soc_disp >= 10)
            {
                r1 = 2.0;
            }
            else if(soc_real - soc_disp >= 5)
            {
                r1 = 1.8;
            }
            else if(soc_real - soc_disp >= 3)
            {
                r1 = 1.5;
            }
            else if(soc_real - soc_disp >= 1)
            {
                r1 = 1.1;
            }
            else if(soc_real - soc_disp <= -10)
            {
                r1 = 0.5;
            }
            else if(soc_real - soc_disp <= -5)
            {
                r1 = 0.6;
            }
            else if(soc_real - soc_disp <= -3)
            {
                r1 = 0.7;
            }
            else if(soc_real - soc_disp <= -1)
            {
                r1 = 0.9;
            }
            else
            {
                r1 = 1;
            }
            /*修正系数r2*/
            if(max_voltage >= max_chg_voltage - 70)
            {
                if(soc_real - soc_disp > 14)
                {
                    r2 = 6.5;
                }
                else if(soc_real - soc_disp > 0)
                {
                    r2 = (soc_real - soc_disp) / 3 + 1.5f;
                }
                else
                {
                    r2 = 1;
                }
            }
            else
            {
                r2 = 1;
            }
            break;
        }
        default:break;
    }
            
    soc_diff = (double)current * ms / 1000 / 3600 * 100 / cr;//mA, ms转h, 除以(1000 * 3600) * 100(%)
    
    if(gPackInfo.mode == EN_PACK_CURRENT_DISCHARGE_MODE)
    {
        if(soc_disp < soc_real)//放电，表显比真实快
        {
            soc_real -= soc_diff;
            if(soc_real < soc_min)
            {
                soc_real = soc_min;
            }
            soc_disp -= r1 * r2 * soc_diff;
            if(soc_disp < soc_min)
            {
                soc_disp = soc_min;
            }
            if(soc_disp > soc_real)//因为校准导致表显又比真实慢
            {
                soc_disp = soc_real;
            }
        }
        else//放电，表显比真实慢，或者相等
        {
            soc_real -= soc_diff;
            if(soc_real < soc_min)
            {
                soc_real = soc_min;
            }
            soc_disp -= r1 * r2 * soc_diff;
            if(soc_disp < soc_min)
            {
                soc_disp = soc_min;
            }
            if(soc_disp < soc_real)//因为校准导致表显又比真实快
            {
                soc_disp = soc_real;
            }
        }
        /*累积放电量统计, 因为电流为正值，所以需要加法*/
        gPackInfo.elect_statist.dsg_total_accum_q += (double)abs(current) * ms / 1000 / 3600;// /1000/3600表示小时
        
        gPackInfo.elect_statist.dsg_cur_day_accum_e += gPackInfo.cell_total_voltage * (double)abs(current) * ms / 1000 / 3600 / 1000 / 1000; // /1000/3600表示小时，/1000/1000表示W(mA*mV/1000/1000)
        gPackInfo.elect_statist.dsg_cur_month_accum_e += gPackInfo.cell_total_voltage * (double)abs(current) * ms / 1000 / 3600 / 1000 / 1000; // /1000/3600表示小时，/1000/1000表示W(mA*mV/1000/1000)
        gPackInfo.elect_statist.dsg_cur_year_accum_e += gPackInfo.cell_total_voltage * (double)abs(current) * ms / 1000 / 3600 / 1000 / 1000; // /1000/3600表示小时，/1000/1000表示W(mA*mV/1000/1000)
        gPackInfo.elect_statist.dsg_total_accum_e += gPackInfo.cell_total_voltage * (double)abs(current) * ms / 1000 / 3600 / 1000 / 1000; // /1000/3600表示小时，/1000/1000表示W(mA*mV/1000/1000)
    }
    if(gPackInfo.mode == EN_PACK_CURRENT_CHARGE_MODE)
    {
        if(soc_disp < soc_real)//充电，表显比真实慢
        {
            soc_real -= soc_diff;
            if(soc_real > soc_max)
            {
                soc_real = soc_max;
            }
            soc_disp -= r1 * r2 * soc_diff;
            if(soc_disp > soc_max)
            {
                soc_disp = soc_max;
            }
            if(soc_disp > soc_real)//因为校准导致表显又比真实快
            {
                soc_disp = soc_real;
            }
        }
        else//充电，表显比真实快，或者相等
        {
            soc_real -= soc_diff;
            if(soc_real > soc_max)
            {
                soc_real = soc_max;
            }
            soc_disp -= r1 * r2 * soc_diff;
            if(soc_disp > soc_max)
            {
                soc_disp = soc_max;
            }
            if(soc_disp < soc_real)//因为校准导致表显又比真实慢
            {
                soc_disp = soc_real;
            }
        }
        /*累积充电量统计, 因为电流为负值，所以需要减法*/
        gPackInfo.elect_statist.chg_total_accum_q += (double)abs(current) * ms / 1000 / 3600;// /1000/3600表示小时
        
        gPackInfo.elect_statist.chg_cur_day_accum_e += gPackInfo.cell_total_voltage * (double)abs(current) * ms / 1000 / 3600 / 1000 / 1000; // /1000/3600表示小时，/1000/1000表示W(mA*mV/1000/1000)
        gPackInfo.elect_statist.chg_cur_month_accum_e += gPackInfo.cell_total_voltage * (double)abs(current) * ms / 1000 / 3600 / 1000 / 1000; // /1000/3600表示小时，/1000/1000表示W(mA*mV/1000/1000)
        gPackInfo.elect_statist.chg_cur_year_accum_e += gPackInfo.cell_total_voltage * (double)abs(current) * ms / 1000 / 3600 / 1000 / 1000; // /1000/3600表示小时，/1000/1000表示W(mA*mV/1000/1000)
        gPackInfo.elect_statist.chg_total_accum_e += gPackInfo.cell_total_voltage * (double)abs(current) * ms / 1000 / 3600 / 1000 / 1000; // /1000/3600表示小时，/1000/1000表示W(mA*mV/1000/1000)
        
    }
    gPackInfo.sox_info.soc_real = soc_real;
    gPackInfo.sox_info.soc_disp = soc_disp;
    gPackInfo.soc_disp_covert = 100 * (soc_disp - soc_min) / (soc_max - soc_min);
    
    gPackInfo.sox_info.chg_cycle = gPackInfo.elect_statist.chg_total_accum_q / gPackInfo.pack_fact_para.q_max;
    gPackInfo.sox_info.dsg_cycle = gPackInfo.elect_statist.dsg_total_accum_q / gPackInfo.pack_fact_para.q_max;
    /*SOH计算，按累积充电量和累积放电量中的较大者计算*/
    if(gPackInfo.sox_info.chg_cycle > gPackInfo.sox_info.dsg_cycle)
    {
        gPackInfo.sox_info.soh = (100 - 80) *(float)(8000 - gPackInfo.sox_info.chg_cycle) / 8000 + 80;
    }
    else
    {
        gPackInfo.sox_info.soh = (100 - 80) * (float)(8000 - gPackInfo.sox_info.dsg_cycle) / 8000 + 80;
    }

CALIBRATION:    
    FullChargeCalibration();
    EmptyDischargeCalibration();
    
}



/** 
 * @brief  电压温度等实时数据获取
 * @param  none
 * @return  NULL
 * @note:  
 */
static void _RTPackInfoGet()
{
    if(gBMSDataInfo.bmu_calc_info.valid_flag == true)
    {
        /*电压温度等实时数据获取*/
        gPackInfo.cell_max_voltage = gBMSDataInfo.bmu_calc_info.cell_max_volt;
        gPackInfo.cell_min_voltage = gBMSDataInfo.bmu_calc_info.cell_min_volt;
        gPackInfo.cell_total_voltage = gBMSDataInfo.bmu_calc_info.cell_total_volt;
        gPackInfo.cell_max_temperature = gBMSDataInfo.bmu_calc_info.cell_max_temp;
        gPackInfo.cell_min_temperature = gBMSDataInfo.bmu_calc_info.cell_min_temp;
        gPackInfo.cell_ave_temperature = gBMSDataInfo.bmu_calc_info.cell_ave_temp;
        
        if(gPackInfo.cell_min_voltage > 2000 && gPackInfo.cell_min_voltage < 4000 &&
           gPackInfo.cell_max_voltage > 2000 && gPackInfo.cell_max_voltage < 4000 && //单体需在2v到4v之间
           abs(gPackInfo.current) <= 350000)//电流需在量程范围内
        {
            gPackInfo.rt_info_valid = true;
        }
        
    }
}





/** 
 * @brief  SOX处理大轮询
 * @param  none
 * @return  NULL
 * @note:  需要等待从控编址完成，且已有全量数据上报给主控后再执行
 */
void SOXProcess()
{
    /*实时数据获取*/
    _RTPackInfoGet();
    
    /*OCV判定*/
    if(GetRtcTimeStamp() - gPackInfo.sox_info.ocv_time > 2 * 60 * 60)//静置2小时后
    {
        OCVCalibration(0);
    }
//    /*限流检测处理*/
//    ReduceCurrentDeal();
    
    /*安时积分处理*/
    if(gPackInfo.calc_flag == true)
    {
        SOCAmpereHourIntegral(gPackInfo.calc_period);
        gPackInfo.calc_flag = false;
    }
    
    /*累计电量跨时间判断*/
    DevRtc_t dev_rtc_temp;
    uint32_t stamp_temp = gDevRtc.time_stamp;//增加该临时变量，是防止在执行下面一段代码过程中，如果时间是中断更新机制，可能导致时间跨度判断过程中发生跨度变化
    dev_rtc_temp.time_stamp = gPackInfo.elect_statist.accum_time_stamp;
    RtcSecConvert2Date(&dev_rtc_temp);
    //当天判断
    if(dev_rtc_temp.year != gDevRtc.year || dev_rtc_temp.month != gDevRtc.month || dev_rtc_temp.day != gDevRtc.day)
    {
        gPackInfo.elect_statist.dsg_cur_day_accum_e = 0;
        gPackInfo.elect_statist.chg_cur_day_accum_e = 0;
    }
    //当月判断
    if(dev_rtc_temp.year != gDevRtc.year || dev_rtc_temp.month != gDevRtc.month)
    {
        gPackInfo.elect_statist.dsg_cur_month_accum_e = 0;
        gPackInfo.elect_statist.chg_cur_month_accum_e = 0;
    }
    //当年判断
    if(dev_rtc_temp.year != gDevRtc.year)
    {
        gPackInfo.elect_statist.dsg_cur_year_accum_e = 0;
        gPackInfo.elect_statist.chg_cur_year_accum_e = 0;
    }
    gPackInfo.elect_statist.accum_time_stamp = stamp_temp;
}





