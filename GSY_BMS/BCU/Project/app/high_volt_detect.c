/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        high_volt_detect.c
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
  * <tr><td>2024-09-02    <td>V1.0.0    <td>          <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "high_volt_detect.h"
#include "filter_calc.h"
#include "systick.h"

#define HV_R_DIV                (510 * 1000)
#define HV_R_DIV_NUM            (7 * 2)
#define HV_R_AMP                1000
#define HV_R_FB                 4700

HighVoltInfo_t gHighVoltInfo;



/** 
* @brief    高压检测计算大轮询
* @param    none
* @return   none
* @note: 
*/
void HighVoltDetectProcess()
{
    uint16_t bp_range_volt[CONT_SAMPLE_NUM];
    uint16_t pp_range_volt[CONT_SAMPLE_NUM];
    
    double vbat_diff;//R1_VBAT+减去R1_VBAT-
    double vpack_diff;//R2_VPACK+减去R2_VBAT-
    
    double v_bat_out; 
    double v_pack_out; 
    
    static uint32_t timecount = 0;
    
    if((uint32_t)(tim_get_systime() - timecount) > 1000)//1S执行一次
    {
        timecount = tim_get_systime();
#if ADC_USE_CONT_MODE
        memcpy(bp_range_volt, DevGetMultiContVoltValue(EN_DEV_VOLT_DETECT_BP), sizeof(bp_range_volt));
        memcpy(pp_range_volt, DevGetMultiContVoltValue(EN_DEV_VOLT_DETECT_PP), sizeof(pp_range_volt));
#else
        for(uint16_t i = 0; i < CONT_SAMPLE_NUM; i++)
        {
            bp_range_volt[i] = DevGetVoltValue(EN_DEV_VOLT_DETECT_BP);
            pp_range_volt[i] = DevGetVoltValue(EN_DEV_VOLT_DETECT_PP);
        }
#endif
        v_bat_out = FilterCalcMedian(bp_range_volt, CONT_SAMPLE_NUM);
        vbat_diff = v_bat_out * HV_R_AMP / HV_R_FB;
        gHighVoltInfo.v_bp = vbat_diff * (HV_R_DIV_NUM * HV_R_DIV + 2 * HV_R_AMP) / (2 * HV_R_AMP);
        
        v_pack_out = FilterCalcMedian(pp_range_volt, CONT_SAMPLE_NUM);
        vpack_diff = v_pack_out * HV_R_AMP / HV_R_FB;
        gHighVoltInfo.v_pp = vpack_diff * (HV_R_DIV_NUM * HV_R_DIV + 2 * HV_R_AMP) / (2 * HV_R_AMP);
    }
}









