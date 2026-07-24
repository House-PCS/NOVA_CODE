/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        current_limit.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-09-04 
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-09-04    <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "current_limit.h"
#include "alarm.h"
#include "bms_data_manage.h"
#include "peri_ctl_stat_api.h"


/** 
 * @brief  限流检测处理，包括放电和充电
 * @param  none
 * @return  NULL
 * @note:  结合电压、温度以及其他故障等因素，计算限流
 */
void _CurrentLimitCalc()
{
    uint32_t normal_chg_max_current = gPackInfo.q_real * gPackInfo.pack_fact_para.chg_max_rate / 10;
    uint32_t normal_dsg_max_current = gPackInfo.q_real * gPackInfo.pack_fact_para.dsg_max_rate / 10;
    
    /*不同故障的充电电流限制*/
    //0-单体过压
    //1-单体过温
    //2-单体欠温
    //3-高压箱继电器过温
    //4-高压箱环境过温
    //5-B+过压
    //6-充电过流
    //7-电芯压差大
    //8-电芯温差大
    //9-极柱过温
    //10-极柱欠温
    //11-温度采样线短路
    //12-温度采样线断开
    //13-AFE故障
    //14-均衡回路故障
    //15-主从控通信故障
    //16-BMU风扇故障
    //17-电芯电压采集故障
    //18-主正继电器粘连故障
    //19-EEPROM故障
    //20-断路器继电器粘连故障
    //21-PCS失联
    //22-SOC低(充电电流不限制，只是为了和放电限制个数保持一致)
    uint32_t  dif_alarm_chg_max_current[23];
    
    /*不同故障的放电电流限制*/
    //0-单体欠压
    //1-单体过温
    //2-单体欠温
    //3-高压箱继电器过温
    //4-高压箱环境过温
    //5-B+欠压
    //6-放电过流
    //7-电芯压差大
    //8-电芯温差大
    //9-极柱过温
    //10-极柱欠温
    //11-温度采样线短路
    //12-温度采样线断开
    //13-AFE故障
    //14-均衡回路故障
    //15-主从控通信故障
    //16-BMU风扇故障
    //17-电芯电压采集故障
    //18-主正继电器粘连故障
    //19-EEPROM故障
    //20-断路器继电器粘连故障
    //21-PCS失联
    //22-SOC低
    uint32_t  dif_alarm_dsg_max_current[23];
    uint8_t cnt = 0;
    
    //先将电流限制设置为正常值
    memset(dif_alarm_chg_max_current, normal_chg_max_current, sizeof(dif_alarm_chg_max_current)/sizeof(uint32_t));
    memset(dif_alarm_dsg_max_current, normal_dsg_max_current, sizeof(dif_alarm_dsg_max_current)/sizeof(uint32_t));
    
    //0-单体过压和单体欠压
    if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_3] & (1 << EN_MEASURE_ALARM_CELL_VOLT_HIGH)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
        if(abs(gPackInfo.current) > gBMSDataInfo.bcu_func_threshold.cut_off_relay_p_max_current)
        {
            //切断断路器
            gBMSDataInfo.breaker_stat = true;
            BreakerCtl(gBMSDataInfo.breaker_stat);
        }
        else
        {
            //切断主正继电器
            gBMSDataInfo.relay_p_stat = false;
            RelayPCtl(gBMSDataInfo.relay_p_stat);
        }
    }
    else if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_2] & (1 << EN_MEASURE_ALARM_CELL_VOLT_HIGH)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
    }
    else if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_1] & (1 << EN_MEASURE_ALARM_CELL_VOLT_HIGH)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
    }
    else
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current; 
    }
    if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_3] & (1 << EN_MEASURE_ALARM_CELL_VOLT_LOW)) != 0)
    {
        dif_alarm_dsg_max_current[cnt] = 0;
        if(abs(gPackInfo.current) > gBMSDataInfo.bcu_func_threshold.cut_off_relay_p_max_current)
        {
            //切断断路器
            gBMSDataInfo.breaker_stat = true;
            BreakerCtl(gBMSDataInfo.breaker_stat);
        }
        else
        {
            //切断主正继电器
            gBMSDataInfo.relay_p_stat = false;
            RelayPCtl(gBMSDataInfo.relay_p_stat);
        }
    }
    else if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_2] & (1 << EN_MEASURE_ALARM_CELL_VOLT_LOW)) != 0)
    {
        dif_alarm_dsg_max_current[cnt] = 0;
    }
    else if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_1] & (1 << EN_MEASURE_ALARM_CELL_VOLT_LOW)) != 0)
    {
        dif_alarm_dsg_max_current[cnt] = 0;
    }
    else
    {
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current; 
    }
    cnt++;
    
    //1-单体过温
    if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_3] & (1 << EN_MEASURE_ALARM_CELL_TEMP_HIGH)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
        dif_alarm_dsg_max_current[cnt] = 0;
        if(abs(gPackInfo.current) > gBMSDataInfo.bcu_func_threshold.cut_off_relay_p_max_current)
        {
            //切断断路器
            gBMSDataInfo.breaker_stat = true;
            BreakerCtl(gBMSDataInfo.breaker_stat);
        }
        else
        {
            //切断主正继电器
            gBMSDataInfo.relay_p_stat = false;
            RelayPCtl(gBMSDataInfo.relay_p_stat);
        }
    }
    else if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_2] & (1 << EN_MEASURE_ALARM_CELL_TEMP_HIGH)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
        dif_alarm_dsg_max_current[cnt] = 0;
    }
    else if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_1] & (1 << EN_MEASURE_ALARM_CELL_TEMP_HIGH)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current / 2;
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current / 2;
    }
    else
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current; 
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current; 
    }
    cnt++;
    
    //2-单体欠温
    if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_3] & (1 << EN_MEASURE_ALARM_CELL_TEMP_LOW)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
        dif_alarm_dsg_max_current[cnt] = 0;
        if(abs(gPackInfo.current) > gBMSDataInfo.bcu_func_threshold.cut_off_relay_p_max_current)
        {
            //切断断路器
            gBMSDataInfo.breaker_stat = true;
            BreakerCtl(gBMSDataInfo.breaker_stat);
        }
        else
        {
            //切断主正继电器
            gBMSDataInfo.relay_p_stat = false;
            RelayPCtl(gBMSDataInfo.relay_p_stat);
        }
    }
    else if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_2] & (1 << EN_MEASURE_ALARM_CELL_TEMP_LOW)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
        dif_alarm_dsg_max_current[cnt] = 0;
    }
    else if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_1] & (1 << EN_MEASURE_ALARM_CELL_TEMP_LOW)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current / 2;
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current / 2;
    }
    else
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current; 
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current; 
    }
    cnt++;
    
    //3-高压箱继电器过温
    if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_3] & (1 << EN_MEASURE_ALARM_BOX1_TEMP_HIGH)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
        dif_alarm_dsg_max_current[cnt] = 0;
        if(abs(gPackInfo.current) > gBMSDataInfo.bcu_func_threshold.cut_off_relay_p_max_current)
        {
            //切断断路器
            gBMSDataInfo.breaker_stat = true;
            BreakerCtl(gBMSDataInfo.breaker_stat);
        }
        else
        {
            //切断主正继电器
            gBMSDataInfo.relay_p_stat = false;
            RelayPCtl(gBMSDataInfo.relay_p_stat);
        }
    }
    else if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_2] & (1 << EN_MEASURE_ALARM_BOX1_TEMP_HIGH)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
        dif_alarm_dsg_max_current[cnt] = 0;
    }
    else if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_1] & (1 << EN_MEASURE_ALARM_BOX1_TEMP_HIGH)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current / 2;
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current / 2;
    }
    else
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current; 
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current; 
    }
    cnt++;
    
    //4-高压箱环境过温
    if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_3] & (1 << EN_MEASURE_ALARM_BOX2_TEMP_HIGH)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
        dif_alarm_dsg_max_current[cnt] = 0;
        if(abs(gPackInfo.current) > gBMSDataInfo.bcu_func_threshold.cut_off_relay_p_max_current)
        {
            //切断断路器
            gBMSDataInfo.breaker_stat = true;
            BreakerCtl(gBMSDataInfo.breaker_stat);
        }
        else
        {
            //切断主正继电器
            gBMSDataInfo.relay_p_stat = false;
            RelayPCtl(gBMSDataInfo.relay_p_stat);
        }
    }
    else if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_2] & (1 << EN_MEASURE_ALARM_BOX2_TEMP_HIGH)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
        dif_alarm_dsg_max_current[cnt] = 0;
    }
    else if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_1] & (1 << EN_MEASURE_ALARM_BOX2_TEMP_HIGH)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current / 2;
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current / 2;
    }
    else
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current; 
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current; 
    }
    cnt++;
    
    //5-B+过压和B+欠压
    if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_3] & (1 << EN_MEASURE_ALARM_BP_OVER)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
        if(abs(gPackInfo.current) > gBMSDataInfo.bcu_func_threshold.cut_off_relay_p_max_current)
        {
            //切断断路器
            gBMSDataInfo.breaker_stat = true;
            BreakerCtl(gBMSDataInfo.breaker_stat);
        }
        else
        {
            //切断主正继电器
            gBMSDataInfo.relay_p_stat = false;
            RelayPCtl(gBMSDataInfo.relay_p_stat);
        }
    }
    else if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_2] & (1 << EN_MEASURE_ALARM_BP_OVER)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
    }
    else if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_1] & (1 << EN_MEASURE_ALARM_BP_OVER)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
    }
    else
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current; 
    }
    if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_3] & (1 << EN_MEASURE_ALARM_BP_UNDER)) != 0)
    {
        dif_alarm_dsg_max_current[cnt] = 0;
        if(abs(gPackInfo.current) > gBMSDataInfo.bcu_func_threshold.cut_off_relay_p_max_current)
        {
            //切断断路器
            gBMSDataInfo.breaker_stat = true;
            BreakerCtl(gBMSDataInfo.breaker_stat);
        }
        else
        {
            //切断主正继电器
            gBMSDataInfo.relay_p_stat = false;
            RelayPCtl(gBMSDataInfo.relay_p_stat);
        }
    }
    else if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_2] & (1 << EN_MEASURE_ALARM_BP_UNDER)) != 0)
    {
        dif_alarm_dsg_max_current[cnt] = 0;
    }
    else if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_1] & (1 << EN_MEASURE_ALARM_BP_UNDER)) != 0)
    {
        dif_alarm_dsg_max_current[cnt] = 0;
    }
    else
    {
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current; 
    }
    cnt++;
    
    //6-充电过流和放电过流
    if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_3] & (1 << EN_MEASURE_ALARM_CHG_CURRENT_HIGH)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
        if(abs(gPackInfo.current) > gBMSDataInfo.bcu_func_threshold.cut_off_relay_p_max_current)
        {
            //切断断路器
            gBMSDataInfo.breaker_stat = true;
            BreakerCtl(gBMSDataInfo.breaker_stat);
        }
        else
        {
            //切断主正继电器
            gBMSDataInfo.relay_p_stat = false;
            RelayPCtl(gBMSDataInfo.relay_p_stat);
        }
    }
    else if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_2] & (1 << EN_MEASURE_ALARM_CHG_CURRENT_HIGH)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
    }
    else if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_1] & (1 << EN_MEASURE_ALARM_CHG_CURRENT_HIGH)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current / 2;
    }
    else
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current; 
    }
    if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_3] & (1 << EN_MEASURE_ALARM_DSG_CURRENT_HIGH)) != 0)
    {
        dif_alarm_dsg_max_current[cnt] = 0;
        if(abs(gPackInfo.current) > gBMSDataInfo.bcu_func_threshold.cut_off_relay_p_max_current)
        {
            //切断断路器
            gBMSDataInfo.breaker_stat = true;
            BreakerCtl(gBMSDataInfo.breaker_stat);
        }
        else
        {
            //切断主正继电器
            gBMSDataInfo.relay_p_stat = false;
            RelayPCtl(gBMSDataInfo.relay_p_stat);
        }
    }
    else if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_2] & (1 << EN_MEASURE_ALARM_DSG_CURRENT_HIGH)) != 0)
    {
        dif_alarm_dsg_max_current[cnt] = 0;
    }
    else if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_1] & (1 << EN_MEASURE_ALARM_DSG_CURRENT_HIGH)) != 0)
    {
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current / 2;
    }
    else
    {
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current; 
    }
    cnt++;
    
    //7-电芯压差大
    if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_3] & (1 << EN_MEASURE_ALARM_CELL_VOLT_DIFF_HIGH)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
        dif_alarm_dsg_max_current[cnt] = 0;
        if(abs(gPackInfo.current) > gBMSDataInfo.bcu_func_threshold.cut_off_relay_p_max_current)
        {
            //切断断路器
            gBMSDataInfo.breaker_stat = true;
            BreakerCtl(gBMSDataInfo.breaker_stat);
        }
        else
        {
            //切断主正继电器
            gBMSDataInfo.relay_p_stat = false;
            RelayPCtl(gBMSDataInfo.relay_p_stat);
        }
    }
    else if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_2] & (1 << EN_MEASURE_ALARM_CELL_VOLT_DIFF_HIGH)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
        dif_alarm_dsg_max_current[cnt] = 0;
    }
    else if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_1] & (1 << EN_MEASURE_ALARM_CELL_VOLT_DIFF_HIGH)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current / 2;
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current / 2;
    }
    else
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current; 
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current; 
    }
    cnt++;
    
    //8-电芯温差大
    if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_3] & (1 << EN_MEASURE_ALARM_CELL_TEMP_DIFF_HIGH)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
        dif_alarm_dsg_max_current[cnt] = 0;
        if(abs(gPackInfo.current) > gBMSDataInfo.bcu_func_threshold.cut_off_relay_p_max_current)
        {
            //切断断路器
            gBMSDataInfo.breaker_stat = true;
            BreakerCtl(gBMSDataInfo.breaker_stat);
        }
        else
        {
            //切断主正继电器
            gBMSDataInfo.relay_p_stat = false;
            RelayPCtl(gBMSDataInfo.relay_p_stat);
        }
    }
    else if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_2] & (1 << EN_MEASURE_ALARM_CELL_TEMP_DIFF_HIGH)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
        dif_alarm_dsg_max_current[cnt] = 0;
    }
    else if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_1] & (1 << EN_MEASURE_ALARM_CELL_TEMP_DIFF_HIGH)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current / 2;
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current / 2;
    }
    else
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current; 
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current; 
    }
    cnt++;
    
    //9-极柱过温
    if((gAlarmCode.other_alarm_code[EN_ALARM_LEVEL_3] & (1 << EN_OTHER_ALARM_POLE_TEMP_HIGH)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
        dif_alarm_dsg_max_current[cnt] = 0;
        if(abs(gPackInfo.current) > gBMSDataInfo.bcu_func_threshold.cut_off_relay_p_max_current)
        {
            //切断断路器
            gBMSDataInfo.breaker_stat = true;
            BreakerCtl(gBMSDataInfo.breaker_stat);
        }
        else
        {
            //切断主正继电器
            gBMSDataInfo.relay_p_stat = false;
            RelayPCtl(gBMSDataInfo.relay_p_stat);
        }
    }
    else if((gAlarmCode.other_alarm_code[EN_ALARM_LEVEL_2] & (1 << EN_OTHER_ALARM_POLE_TEMP_HIGH)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
        dif_alarm_dsg_max_current[cnt] = 0;
    }
    else if((gAlarmCode.other_alarm_code[EN_ALARM_LEVEL_1] & (1 << EN_OTHER_ALARM_POLE_TEMP_HIGH)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current / 2;
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current / 2;
    }
    else
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current; 
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current; 
    }
    cnt++;
    
    //10-极柱欠温
    if((gAlarmCode.other_alarm_code[EN_ALARM_LEVEL_3] & (1 << EN_OTHER_ALARM_POLE_TEMP_LOW)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
        dif_alarm_dsg_max_current[cnt] = 0;
        if(abs(gPackInfo.current) > gBMSDataInfo.bcu_func_threshold.cut_off_relay_p_max_current)
        {
            //切断断路器
            gBMSDataInfo.breaker_stat = true;
            BreakerCtl(gBMSDataInfo.breaker_stat);
        }
        else
        {
            //切断主正继电器
            gBMSDataInfo.relay_p_stat = false;
            RelayPCtl(gBMSDataInfo.relay_p_stat);
        }
    }
    else if((gAlarmCode.other_alarm_code[EN_ALARM_LEVEL_2] & (1 << EN_OTHER_ALARM_POLE_TEMP_LOW)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
        dif_alarm_dsg_max_current[cnt] = 0;
    }
    else if((gAlarmCode.other_alarm_code[EN_ALARM_LEVEL_1] & (1 << EN_OTHER_ALARM_POLE_TEMP_LOW)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current / 2;
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current / 2;
    }
    else
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current; 
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current; 
    }
    cnt++;
    
    //11-温度采样线短路
    if((gAlarmCode.dev_alarm_code[EN_ALARM_LEVEL_1] & (1 << EN_DEV_ALARM_CELL_NTC_SHORT_FAULT)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
        dif_alarm_dsg_max_current[cnt] = 0;
        if(abs(gPackInfo.current) > gBMSDataInfo.bcu_func_threshold.cut_off_relay_p_max_current)
        {
            //切断断路器
            gBMSDataInfo.breaker_stat = true;
            BreakerCtl(gBMSDataInfo.breaker_stat);
        }
        else
        {
            //切断主正继电器
            gBMSDataInfo.relay_p_stat = false;
            RelayPCtl(gBMSDataInfo.relay_p_stat);
        }
    }
    else
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current; 
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current; 
    }
    cnt++;
    
    //12-温度采样线断开
    if((gAlarmCode.dev_alarm_code[EN_ALARM_LEVEL_1] & (1 << EN_DEV_ALARM_CELL_NTC_BREAK_FAULT)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
        dif_alarm_dsg_max_current[cnt] = 0;
        if(abs(gPackInfo.current) > gBMSDataInfo.bcu_func_threshold.cut_off_relay_p_max_current)
        {
            //切断断路器
            gBMSDataInfo.breaker_stat = true;
            BreakerCtl(gBMSDataInfo.breaker_stat);
        }
        else
        {
            //切断主正继电器
            gBMSDataInfo.relay_p_stat = false;
            RelayPCtl(gBMSDataInfo.relay_p_stat);
        }
    }
    else
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current; 
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current; 
    }
    cnt++;
    
    //13-AFE故障
    if((gAlarmCode.dev_alarm_code[EN_ALARM_LEVEL_1] & (1 << EN_DEV_ALARM_AFE_FAULT)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
        dif_alarm_dsg_max_current[cnt] = 0;
        if(abs(gPackInfo.current) > gBMSDataInfo.bcu_func_threshold.cut_off_relay_p_max_current)
        {
            //切断断路器
            gBMSDataInfo.breaker_stat = true;
            BreakerCtl(gBMSDataInfo.breaker_stat);
        }
        else
        {
            //切断主正继电器
            gBMSDataInfo.relay_p_stat = false;
            RelayPCtl(gBMSDataInfo.relay_p_stat);
        }
    }
    else
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current; 
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current; 
    }
    cnt++;
    
    //14-均衡回路故障
    if((gAlarmCode.dev_alarm_code[EN_ALARM_LEVEL_1] & (1 << EN_DEV_ALARM_BALANCE_MOS_FAULT)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
        dif_alarm_dsg_max_current[cnt] = 0;
        if(abs(gPackInfo.current) > gBMSDataInfo.bcu_func_threshold.cut_off_relay_p_max_current)
        {
            //切断断路器
            gBMSDataInfo.breaker_stat = true;
            BreakerCtl(gBMSDataInfo.breaker_stat);
        }
        else
        {
            //切断主正继电器
            gBMSDataInfo.relay_p_stat = false;
            RelayPCtl(gBMSDataInfo.relay_p_stat);
        }
    }
    else
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current; 
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current; 
    }
    cnt++;
    
    //15-主从控通信故障
    if((gAlarmCode.dev_alarm_code[EN_ALARM_LEVEL_1] & (1 << EN_DEV_ALARM_BMU_COMM_FAULT)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
        dif_alarm_dsg_max_current[cnt] = 0;
        if(abs(gPackInfo.current) > gBMSDataInfo.bcu_func_threshold.cut_off_relay_p_max_current)
        {
            //切断断路器
            gBMSDataInfo.breaker_stat = true;
            BreakerCtl(gBMSDataInfo.breaker_stat);
        }
        else
        {
            //切断主正继电器
            gBMSDataInfo.relay_p_stat = false;
            RelayPCtl(gBMSDataInfo.relay_p_stat);
        }
    }
    else
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current; 
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current; 
    }
    cnt++;
    
    //16-BMU风扇故障
    if((gAlarmCode.dev_alarm_code[EN_ALARM_LEVEL_1] & (1 << EN_DEV_ALARM_BMU_FAN_FAULT)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
        dif_alarm_dsg_max_current[cnt] = 0;
        if(abs(gPackInfo.current) > gBMSDataInfo.bcu_func_threshold.cut_off_relay_p_max_current)
        {
            //切断断路器
            gBMSDataInfo.breaker_stat = true;
            BreakerCtl(gBMSDataInfo.breaker_stat);
        }
        else
        {
            //切断主正继电器
            gBMSDataInfo.relay_p_stat = false;
            RelayPCtl(gBMSDataInfo.relay_p_stat);
        }
    }
    else
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current; 
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current; 
    }
    cnt++;
    
    //17-电芯电压采集故障
    if((gAlarmCode.dev_alarm_code[EN_ALARM_LEVEL_1] & (1 << EN_DEV_ALARM_CELL_VOLT_SAMP_FAULT)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
        dif_alarm_dsg_max_current[cnt] = 0;
        if(abs(gPackInfo.current) > gBMSDataInfo.bcu_func_threshold.cut_off_relay_p_max_current)
        {
            //切断断路器
            gBMSDataInfo.breaker_stat = true;
            BreakerCtl(gBMSDataInfo.breaker_stat);
        }
        else
        {
            //切断主正继电器
            gBMSDataInfo.relay_p_stat = false;
            RelayPCtl(gBMSDataInfo.relay_p_stat);
        }
    }
    else
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current; 
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current; 
    }
    cnt++;
    
    //18-主正继电器粘连故障
    if((gAlarmCode.dev_alarm_code[EN_ALARM_LEVEL_1] & (1 << EN_DEV_ALARM_RELAY_P_ADH_FAULT)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
        dif_alarm_dsg_max_current[cnt] = 0;
        if(abs(gPackInfo.current) > gBMSDataInfo.bcu_func_threshold.cut_off_relay_p_max_current)
        {
            //切断断路器
            gBMSDataInfo.breaker_stat = true;
            BreakerCtl(gBMSDataInfo.breaker_stat);
        }
        else
        {
            //切断主正继电器
            gBMSDataInfo.relay_p_stat = false;
            RelayPCtl(gBMSDataInfo.relay_p_stat);
        }
    }
    else
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current; 
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current; 
    }
    cnt++;
    
    //19-EEPROM故障
    if((gAlarmCode.dev_alarm_code[EN_ALARM_LEVEL_1] & (1 << EN_DEV_ALARM_EEPROM_FAULT)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
        dif_alarm_dsg_max_current[cnt] = 0;
        if(abs(gPackInfo.current) > gBMSDataInfo.bcu_func_threshold.cut_off_relay_p_max_current)
        {
            //切断断路器
            gBMSDataInfo.breaker_stat = true;
            BreakerCtl(gBMSDataInfo.breaker_stat);
        }
        else
        {
            //切断主正继电器
            gBMSDataInfo.relay_p_stat = false;
            RelayPCtl(gBMSDataInfo.relay_p_stat);
        }
    }
    else
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current; 
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current; 
    }
    cnt++;
    
    //20-断路器继电器粘连故障
    if((gAlarmCode.dev_alarm_code[EN_ALARM_LEVEL_1] & (1 << EN_DEV_ALARM_BREAKER_ADH_FAULT)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
        dif_alarm_dsg_max_current[cnt] = 0;
        if(abs(gPackInfo.current) > gBMSDataInfo.bcu_func_threshold.cut_off_relay_p_max_current)
        {
            //切断断路器
            gBMSDataInfo.breaker_stat = true;
            BreakerCtl(gBMSDataInfo.breaker_stat);
        }
        else
        {
            //切断主正继电器
            gBMSDataInfo.relay_p_stat = false;
            RelayPCtl(gBMSDataInfo.relay_p_stat);
        }
    }
    else
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current; 
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current; 
    }
    cnt++;
    
    //21-PCS失联
    if((gAlarmCode.other_alarm_code[EN_ALARM_LEVEL_1] & (1 << EN_OTHER_ALARM_PCS_LOST)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = 0;
        dif_alarm_dsg_max_current[cnt] = 0;
        if(abs(gPackInfo.current) > gBMSDataInfo.bcu_func_threshold.cut_off_relay_p_max_current)
        {
            //切断断路器
            gBMSDataInfo.breaker_stat = true;
            BreakerCtl(gBMSDataInfo.breaker_stat);
        }
        else
        {
            //切断主正继电器
            gBMSDataInfo.relay_p_stat = false;
            RelayPCtl(gBMSDataInfo.relay_p_stat);
        }
    }
    else
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current; 
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current; 
    }
    cnt++;
    
    //22-SOC低(只限制放电)
    if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_3] & (1 << EN_MEASURE_ALARM_SOC_LOW)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current;
        dif_alarm_dsg_max_current[cnt] = 0;
    }
    else if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_2] & (1 << EN_MEASURE_ALARM_SOC_LOW)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current;
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current / 2;
    }
    else if((gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_1] & (1 << EN_MEASURE_ALARM_SOC_LOW)) != 0)
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current;
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current;
    }
    else
    {
        dif_alarm_chg_max_current[cnt] = normal_chg_max_current; 
        dif_alarm_dsg_max_current[cnt] = normal_dsg_max_current; 
    }
    cnt++;
    
    
    gPackInfo.chg_max_current = normal_chg_max_current;
    gPackInfo.dsg_max_current = normal_dsg_max_current;
    
    for(uint8_t i = 0; i < sizeof(dif_alarm_chg_max_current)/sizeof(uint32_t); i++)
    {
        if(dif_alarm_chg_max_current[i] < gPackInfo.chg_max_current)
        {
            gPackInfo.chg_max_current = dif_alarm_chg_max_current[i];
        }
    }
    
    for(uint8_t i = 0; i < sizeof(dif_alarm_dsg_max_current)/sizeof(uint32_t); i++)
    {
        if(dif_alarm_dsg_max_current[i] < gPackInfo.dsg_max_current)
        {
            gPackInfo.dsg_max_current = dif_alarm_dsg_max_current[i];
        }
    }
}




/** 
 * @brief  限流检测处理，包括放电和充电
 * @param  none
 * @return  NULL
 * @note:  结合电压、温度以及其他故障等因素，计算限流
 */
void CurrentLimitProcess()
{
    static uint32_t chg_timecount = 0, dsg_timecount = 0;
    static uint8_t start_flag = 0;//0代表第一次启动，1代表运行过程中
    uint16_t max_voltage = gPackInfo.cell_max_voltage;
    uint16_t min_voltage = gPackInfo.cell_min_voltage;
    uint32_t lim_chg_min_current, lim_dsg_min_current;
    
    _CurrentLimitCalc();
    
    
    if(gBMSDataInfo.current_forbid_flag == true)//禁止充放电标志
    {
        gPackInfo.chg_cur_current_limit = 0;
        gPackInfo.dsg_cur_current_limit = 0;
        return;
    }
    else
    {
        lim_chg_min_current = gPackInfo.chg_max_current * gPackInfo.pack_fact_para.chg_reduce_cur_rate / gPackInfo.pack_fact_para.chg_max_rate;
        lim_dsg_min_current = gPackInfo.dsg_max_current * gPackInfo.pack_fact_para.dsg_reduce_cur_rate / gPackInfo.pack_fact_para.dsg_max_rate;
    }
    
    if(gBMSDataInfo.bmu_calc_info.valid_flag != true)//至少获取一次从控全量数据，并完成一次计算才能计算电流限值
    {
        gPackInfo.chg_cur_current_limit = 0;
        gPackInfo.dsg_cur_current_limit = 0;
        return;
    }
    else
    {
        if(start_flag == 0)//上电第一次确定电流限制，计算初始值
        {
            start_flag = 1;
            /*充电判断，基于SOC判断*/
            if(gPackInfo.sox_info.soc_real <= 80)
            {
                gPackInfo.chg_cur_current_limit = gPackInfo.chg_max_current;
            }
            /*放电判断，基于SOC判断*/
            if(gPackInfo.sox_info.soc_real >= 20)
            {
                gPackInfo.dsg_cur_current_limit = gPackInfo.dsg_max_current;
            }
        }
    }
    
    /*最大充放电电流限制不能超过最大充放电电流*/
    if(gPackInfo.chg_cur_current_limit > gPackInfo.chg_max_current)
    {
        gPackInfo.chg_cur_current_limit = gPackInfo.chg_max_current;
    }
    if(gPackInfo.dsg_cur_current_limit > gPackInfo.dsg_max_current)
    {
        gPackInfo.dsg_cur_current_limit = gPackInfo.dsg_max_current;
    }
    
    /*充电判断*/
    if(max_voltage >= gPackInfo.pack_fact_para.cell_max_chg_voltage || gPackInfo.sox_info.soc_real >= gPackInfo.pack_fact_para.soc_max)//停充
    {
        gPackInfo.chg_cur_current_limit = 0;
        FullChargeCalibration();
    }
    else if(max_voltage > gPackInfo.pack_fact_para.chg_reduce_cur_voltage)//降流充
    {
        if((uint32_t)(tim_get_systime() - chg_timecount) > 2000)
        {
            chg_timecount = tim_get_systime();
            gPackInfo.chg_cur_current_limit = gPackInfo.chg_cur_current_limit * 95 / 100; //按照0.95倍递减
            if(gPackInfo.chg_cur_current_limit < lim_chg_min_current)
            {
                gPackInfo.chg_cur_current_limit = lim_chg_min_current;
            }
        }
    }
    else//正常充
    {
        if((uint32_t)(tim_get_systime() - chg_timecount) > 2000)
        {
            chg_timecount = tim_get_systime();
            gPackInfo.chg_cur_current_limit = gPackInfo.chg_cur_current_limit * 110 / 100; //按照1.1倍递增
            if(gPackInfo.chg_cur_current_limit > gPackInfo.chg_max_current)
            {
                gPackInfo.chg_cur_current_limit = gPackInfo.chg_max_current;
            }
            else if(gPackInfo.chg_cur_current_limit == 0)
            {
                gPackInfo.chg_cur_current_limit = lim_chg_min_current;
            }
        }
    }
    
    /*放电判断*/
    if(min_voltage < gPackInfo.pack_fact_para.cell_min_dsg_voltage || gPackInfo.sox_info.soc_real <= gPackInfo.pack_fact_para.soc_min)//停放
    {
        gPackInfo.dsg_cur_current_limit = 0;
        EmptyDischargeCalibration();
    }
    if(min_voltage < gPackInfo.pack_fact_para.dsg_reduce_cur_voltage)//降流放
    {
        if((uint32_t)(tim_get_systime() - dsg_timecount) > 2000)
        {
            dsg_timecount = tim_get_systime();
            gPackInfo.dsg_cur_current_limit = gPackInfo.dsg_cur_current_limit * 98 / 100; //按照0.98倍递减
            if(gPackInfo.dsg_cur_current_limit < lim_dsg_min_current)
            {
                gPackInfo.dsg_cur_current_limit = lim_dsg_min_current;
            }
        }
    }
    else//正常放
    {
        if((uint32_t)(tim_get_systime() - dsg_timecount) > 2000)
        {
            dsg_timecount = tim_get_systime();
            gPackInfo.dsg_cur_current_limit = gPackInfo.dsg_cur_current_limit * 110 / 100; //按照1.1倍递增
            if(gPackInfo.dsg_cur_current_limit > gPackInfo.dsg_max_current)
            {
                gPackInfo.dsg_cur_current_limit = gPackInfo.dsg_max_current;
            }
            else if(gPackInfo.dsg_cur_current_limit == 0)
            {
                gPackInfo.dsg_cur_current_limit = lim_dsg_min_current;
            }
        }
    }
    /*过放后，充电电流也要限制*/
    if(min_voltage < gPackInfo.pack_fact_para.cell_min_dsg_voltage - 100)
    {
        if(gPackInfo.chg_cur_current_limit > gPackInfo.chg_max_current / 10)
        {
            gPackInfo.chg_cur_current_limit = gPackInfo.chg_max_current / 10;
        }
    }
}





