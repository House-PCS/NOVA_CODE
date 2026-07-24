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



#include "bms_sample_hv_bsp.h"
#include "mcu_bsp.h"
#include "data_process.h"

BmsSampleHVType BmsSampleHV = {0};
const BmsSampleHVType *const IR_BmsSampleHV = &BmsSampleHV;

#define HV_GAIN_BSP     759.57
/** 
* @brief  高压采样模块主函数
* @param  none
* @return  none
* @note:
*
*/
void SampleHighVoltMain()
{
    static bool first=true;
    uint32_t hv_voltage=0;
    bool valid=true;
    uint8_t i=0;
    
    
    
    if(first)
    {
        for(i=0; i < HV_NUM; i++)
        {
//            hv_voltage = pIR_Adc2Handle->pRValtage[VBAT + i] * HV_GAIN_BSP;  
            BmsSampleHV.hv_voltage[i] = hv_voltage / 1000;
        }
//        first=false;
    }
//    else 
//    {
//        for(i=0; i < HV_NUM; i++)
//        {
//            hv_voltage = pIR_Adc2Handle->pRValtage[VBAT+i]*525;
//            hv_voltage /= 1000;
////            if(ABS_DIFF(BmsSampleHV.hv_voltage[i],hv_voltage) < 500)
//            {
//                BmsSampleHV.hv_voltage[i]=hv_voltage;
//            }
////            else
////            {
////                valid=false;
////            }
//        }
//    }
    if(valid)
    {
        BmsSampleHV.valid_cnt++;
    }
    else
    {
        BmsSampleHV.error_cnt++;
    }
}


