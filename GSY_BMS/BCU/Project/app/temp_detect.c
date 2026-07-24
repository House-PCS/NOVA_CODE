/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        temp_detect.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-09-09
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-09-09    <td>V1.0.0    <td>          <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "temp_detect.h"
#include "ntc_convert.h"

TempInfo_t  gTempInfo;

#define TEMP_V_MAX   3300
#define TEMP_R_UP   10000

/** 
* @brief    各通道温度检测
* @param    none
* @return   none
* @note: 
*/
static EN_OP_RES _TempChannelDetect(EN_DEV_TEMP_DETECT_IDX idx)
{
    static uint32_t time_count = 0;
    static EN_SGM_TEMP_GET_OP stat = EN_SGM_TEMP_GET_CMD;
    uint16_t volt_value;
    float *p_temp;
    
    switch(stat)
    {
        case EN_SGM_TEMP_GET_CMD:
        {
            DevSgm58031VoltGetCmd(idx);
            stat = EN_SGM_TEMP_GET_STAT;
            time_count = tim_get_systime();
            return EN_OP_RES_ING;
            break;
        }
        case EN_SGM_TEMP_GET_STAT:
        {
            EN_DEV_SGM_OP_STATUS res = DevSgm58031VoltGetStat(&volt_value);
            if(res == EN_DEV_SGM_OP_STATUS_COMPLETE)
            {
                /*计算温度*/
                switch(idx)
                {
                    case EN_DEV_TEMP_DETECT_1:
                    {
                        p_temp = &gTempInfo.temp_relay_p;
                        break;
                    }
                    case EN_DEV_TEMP_DETECT_2:
                    {
                        p_temp = &gTempInfo.temp_relay_n;
                        break;
                    }
                    case EN_DEV_TEMP_DETECT_3:
                    {
                        p_temp = &gTempInfo.temp_relay_pre;
                        break;
                    }
                    case EN_DEV_TEMP_DETECT_4:
                    {
                        p_temp = &gTempInfo.temp_hvbox;
                        break;
                    }
                    default:
                    {
                        stat = EN_SGM_TEMP_GET_CMD;
                        return EN_OP_RES_FAIL;
                    }
                }
                *p_temp = TemperatureGet(volt_value, TEMP_V_MAX, TEMP_R_UP, EN_NTC_B_3435, EN_NTC_T25_10K);
                stat = EN_SGM_TEMP_GET_CMD;
                return EN_OP_RES_COMPLETE;
            }
            else if(res == EN_DEV_SGM_OP_STATUS_ERROR)
            {
                stat = EN_SGM_TEMP_GET_CMD;
                return EN_OP_RES_FAIL;
            }
            else
            {
                if((uint32_t)(tim_get_systime() - time_count) > 1000)//暂定1s作为超时时间
                {
                    stat = EN_SGM_TEMP_GET_CMD;
                    return EN_OP_RES_FAIL;
                }
            }
            break;
        }
        default:break;
    }
    
    return EN_OP_RES_ING;
}


/** 
* @brief    温度检测计算大轮询
* @param    none
* @return   none
* @note: 
*/
void TempDetectProcess()
{
    static uint32_t time_count = 0;
    static EN_DEV_TEMP_DETECT_IDX cnt = EN_DEV_TEMP_DETECT_1;//四个温度顺序执行
    if((uint32_t)(tim_get_systime() - time_count) > 100)//暂定每100ms执行一次
    {
        if(_TempChannelDetect(cnt) != EN_OP_RES_ING)
        {
            cnt++;
            cnt %= EN_DEV_TEMP_NUM;
        }
    }
}















