/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        fault_diagnosis.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-5-31
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-5-31    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "fault_diagnosis.h"
#include "mppt_process.h"
#include "inverter_loop_cntl.h"
#include "algo_err_check.h"
#include "algo_info.h"
#include "modbus_analysis.h"
/* Includes ------------------------------------------------------------------*/

#define GRID_HIGH_VOL_THRESHOLD      (265)         //电网高压阈值
#define GRID_LOW_VOL_THRESHOLD      (170)          //电网低压阈值
#define GRID_HIGH_FRE_THRESHOLD      (55)          //电网高频阈值
#define GRID_LOW_FRE_THRESHOLD      (45)          //电网低频阈值
#define BAT_HIGH_VOL_THRESHOLD       (59)         //电池高压阈值
#define BAT_LOW_VOL_THRESHOLD       (32)          //电池低压阈值 45 // 20260311，改为32V
#define BAT_UNDERVOLTAGE             (41.6)       //电池欠压

#define PV_MISS                      (70)         //PV未接阈值

#define BUS_HIGH_VOL_THRESHOLD       (520)         //母线高压阈值
#define BUS_LOW_VOL_THRESHOLD        (378)          //母线低压阈值

#define OVERLOAD_lV1                    (5500)         //过载阈值1
#define OVERLOAD_lV2                    (6000)         //过载阈值2
#define INV_OVERCUR_THERSHOLD       (50)           //逆变过流
#define LEAK_CURRENT                (30)           //漏电流

#define BAT_CHG_OVERCUR             (120)          //电池充电过流
#define BAT_DSG_OVERCUR             (-120)          //电池放电过流

#define INV_OVER_TEMP               (850)          //散热器过温

#define GPIO1           28
#include "fault_diagnosis.h"
#include "dev_sensor.h"
#include "inverter_loop_cntl.h"
static InverterState_t* _gInvSta;
static float load_power;
FaultInfo_t gstFaultInfo ;
DiagnosisInfo_t gstDiagnosisInfo;
uint16_t gComFail;
uint16_t gDeiceComFlag;

uint16_t arc_fault_flag = 0;
float gLeak3ma,gLeakCur,gLeakCmp;

float gLeakCheckThreshold = 30.0f; //漏电流检测阈值（ma）
float gLeakChecktime = 300.0f; //漏电流检测阈值 (ms)

float WATCH_LeakCurrent = 0;
float WATCH_LeakCurrent_rms = 0;
uint16_t count_SlidingWindow = 0;
uint16_t flag_SlidingWindow = 0;

uint16_t WATCH_flag_warning_LeakCurrent = 0;
uint32_t WATCH_count_warning_LeakCurrent = 0;
uint16_t gArcFaultStandbyFlag = 0;
//void WarnningDiagnosisProcess(void)
//{
//    _gInvSta = InverterGetState();
//    if(_gInvSta->sta_grid_vol_rms > )
//}

void WarnningDiagnosisProcess(void)//告警判断
{

//    /* 电网电压低于下限 */
//    if(_gInvSta->sta_grid_vol_rms < GRID_LOW_VOL_THRESHOLD)
//    {
//        gstDiagnosisInfo.grid_vol_high_count = 0;
//        gstDiagnosisInfo.grid_vol_low_count++;
//    }
//    else if(_gInvSta->sta_grid_vol_rms > GRID_HIGH_VOL_THRESHOLD) /* 电网电压大于上限 */
//    {
//        gstDiagnosisInfo.grid_vol_low_count = 0;
//        gstDiagnosisInfo.grid_vol_high_count++;
//    }
//    else
//    {
//        gstDiagnosisInfo.grid_vol_high_count = 0;
//        gstDiagnosisInfo.grid_vol_low_count = 0;
//    }
//
//    if(gstDiagnosisInfo.grid_vol_low_count > 5)
//    {
//        gstFaultInfo.worning_code.b.grid_vol_low = 1;
//    }
//    else
//    {
//        gstFaultInfo.worning_code.b.grid_vol_low = 0;
//    }
//
//    if(gstDiagnosisInfo.grid_vol_high_count > 5)
//    {
//        gstFaultInfo.worning_code.b.grid_vol_high = 1;
//    }
//    else
//    {
//        gstFaultInfo.worning_code.b.grid_vol_high = 0;
//    }
    /*电网频率高*/
//    if(_gInvSta->)

    /* 电池电压低*/
    if(gstSensorData.bat_voltage < BAT_LOW_VOL_THRESHOLD) //// 20260311，改为32V
    {
        gstDiagnosisInfo.bat_vol_high_count = 0;
        gstDiagnosisInfo.bat_vol_low_count++;
    }
    /*电池电压高*/
    else if(gstSensorData.bat_voltage > BAT_HIGH_VOL_THRESHOLD)
    {
        gstDiagnosisInfo.bat_vol_low_count = 0;
        gstDiagnosisInfo.bat_vol_high_count++;
    }
    else
    {
        gstDiagnosisInfo.bat_vol_low_count = 0;
        gstDiagnosisInfo.bat_vol_high_count= 0;
    }
    if(gstDiagnosisInfo.bat_vol_low_count > 5)
    {
        gstFaultInfo.worning_code.b.bat_vol_low = 1;
    }
    else
    {
        gstFaultInfo.worning_code.b.bat_vol_low = 0;
    }

    if(gstDiagnosisInfo.bat_vol_high_count > 5)
    {
        gstFaultInfo.worning_code.b.bat_vol_high = 1;
    }
    else
    {
        gstFaultInfo.worning_code.b.bat_vol_high = 0;
    }
//    /*PV1未接*/
//    if(gPv1_Voltage_Ave < PV_MISS)
//    {
//        gstFaultInfo.worning_code.b.pv_no_connected = 1;
//    }
//    else
//    {
//        gstFaultInfo.worning_code.b.pv_no_connected = 0;
//    }
//        /*电池未连接*/  //默认为电压为0
//        if(gstSensorData.bat_voltage == 0)
//        {
//            gstFaultInfo.worning_code.b.bat_under_vol = 1;
//        }
//        else
//        {
//            gstFaultInfo.worning_code.b.bat_under_vol = 0;
//        }
//
    /*过载*/
       if(load_power > OVERLOAD_lV1)
       {
           gstDiagnosisInfo.over_load_count++;
       }
       else
       {
           gstDiagnosisInfo.over_load_count = 0;

       }
       if(gstDiagnosisInfo.over_load_count > 5)
       {
           gstFaultInfo.worning_code.b.overload = 1;
       }
       else
       {
           gstFaultInfo.worning_code.b.overload = 0;
       }
    /*漏电流高*/
//       if(gstSensorData.leak_current > LEAK_CURRENT)
//         {
//             gstDiagnosisInfo.leak_current_over_count++;
//         }
//         else
//         {
//             gstDiagnosisInfo.leak_current_over_count = 0;
//         }
//         if(gstDiagnosisInfo.leak_current_over_count > 5)
//         {
//             gstFaultInfo.worning_code.b.leak_curr_high = 1;
//         }
//         else
//         {
//             gstFaultInfo.worning_code.b.leak_curr_high = 0;
//         }
//    /* 逆变散热器过温 */
//    if(gstSensorData.temp[4] > INV_OVER_TEMP)
//    {
//        gstDiagnosisInfo.inv_overtempe_count++;
//    }
//    else
//    {
//        if(gstDiagnosisInfo.inv_overtempe_count < 1)
//        {
//            gstDiagnosisInfo.inv_overtempe_count = 0;
//        }
//        else
//        {
//            gstDiagnosisInfo.inv_overtempe_count--;
//        }
//    }
//
//    if(gstDiagnosisInfo.inv_overtempe_count > 6)
//    {
//        gstFaultInfo.error_code.b.box_over_temp = 1;
//        gstFaultInfo.error_code.b.radiator_over_temp = 1;
//    }
//    else
//    {
//        gstFaultInfo.error_code.b.box_over_temp = 0;
//        gstFaultInfo.error_code.b.radiator_over_temp = 0;
//    }


}
void ErrorDiagnosis(void)//故障判断,需要加滤波？
{
    static uint32_t delay_times = 0;

//     /*母线电压低*/
//    if(gBus_Voltage_Ave < BUS_LOW_VOL_THRESHOLD)
//    {
//        gstFaultInfo.error_code.b.bus_vol_low = 1;
//    }
//    else
//    {
//        gstFaultInfo.error_code.b.bus_vol_low = 0;
//    }
//    /*母线电压高*/
//    if (gBus_Voltage_Ave > BUS_HIGH_VOL_THRESHOLD)
//    {
//        gstFaultInfo.error_code.b.bus_vol_high = 1;
//    }
//    else
//    {
//        gstFaultInfo.error_code.b.bus_vol_high = 0;
//    }
//    /*电池欠压*/
//    if(gBat_Voltage_Ave < BAT_UNDERVOLTAGE)
//    {
//        gstFaultInfo.error_code.b.battery_undervoltage = 1;
//    }
//    else
//    {
//        gstFaultInfo.error_code.b.battery_undervoltage = 0;
//    }
//    /*逆变过流*/
//    if(_gInvSta->sta_inv_cur_rms > INV_OVERCUR_THERSHOLD)
//    {
//        gstDiagnosisInfo.inv_overcur_count++;
//    }
//    else
//    {
//        gstDiagnosisInfo.inv_overcur_count = 0;
//    }
//    if(gstDiagnosisInfo.inv_overcur_count > 5)
//    {
//        gstFaultInfo.error_code.b.inv_over_curr = 1;
//    }
//    else
//    {
//        gstFaultInfo.error_code.b.inv_over_curr = 0;
//    }
//    /*过载*/
//    if(load_power > OVERLOAD_lV2)
//    {
//        gstFaultInfo.error_code.b.always_over_load2 = 1;
//    }
//    else
//    {
//        gstFaultInfo.error_code.b.always_over_load2 = 0;
//    }
//    /*电池充电过流*/
//    if(gBat_Current_Ave > BAT_CHG_OVERCUR)
//    {
//        gstFaultInfo.error_code.b.bat_chg_over_curr = 1;
//    }
//    else
//    {
//        gstFaultInfo.error_code.b.bat_chg_over_curr = 0;
//    }
//    /*电池放电过流*/
//    if(gBat_Current_Ave < BAT_DSG_OVERCUR)
//    {
//        gstFaultInfo.error_code.b.bat_dsg_over_curr = 1;
//    }
//    else
//    {
//        gstFaultInfo.error_code.b.bat_dsg_over_curr = 0;
//    }
    /*通信故障*/
    if(GetSciaRecvFlag()!=1)
    {
        if(TimGetSystime() - delay_times > 10000)//超过10S后判断通讯故障
        {
//            gstFaultInfo.error_code.value = 1 << 32;
            gComFail = 0x8000;
            gDeiceComFlag = 0;
            delay_times = TimGetSystime();
        }
    }
    else
    {
//        gstFaultInfo.error_code.b.always_comm_error = 0;
        gComFail = 0;
        gDeiceComFlag = 1;
        delay_times = TimGetSystime();
    }
//    uint32_t value = gAlgoErrDef.byte[1]|;

//    gstFaultInfo.error_code.value = (uint32_t)gAlgoErrDef。byte[0];
//    gstFaultInfo.error_code.value = ((gstFaultInfo.error_code.value | gAlgoErrDef。byte[1]) << 16) || gAlgoErrDef。byte[0];

}
void FaultDiagnosisProcess(void)
{
    _gInvSta = InverterGetState();
    static uint32_t diagnosis_times = 0;
    load_power = _gInvSta->sta_load_cur_rms * _gInvSta->sta_load_vol_rms;
    if(TimDifftimeFromNow(diagnosis_times) > 200)
    {
        WarnningDiagnosisProcess();
        ErrorDiagnosis();
        diagnosis_times = TimGetSystime();
    }
}

//void ErrorHandling()
//{
//    uint32_t error_flag = gstFaultInfo.error_code.value;
//    uint8_t failure_level = 0;
//    if((error_flag && 0x100)!=0)//故障类型1：只保留LLC
//    {
//        failure_level = 1;
//    }
//    if((error_flag && 0x0035BC73) != 0)//故障类型2：故障后1分钟重连
//    {
//        failure_level = 2;
//    }
//    if((error_flag && 0x80000200)!=0)//故障类型3：故障后不再恢复  判断顺序很重要，影响执行动作等级
//    {
//        failure_level = 3;
//    }
//
//    switch(failure_level)
//    {
//        case 1:
//        {
//
//            break;
//        }
//        case 2:
//        {
//
//            break;
//        }
//        case 3:
//        {
//
//            break;
//        }
//
//
//    }
//}

/**
 * @brief
 * @param LeakCurrCheck
 * @param 漏电流检测函数
 * @return
 */

void LeakCurrCheck(void)
{
    static uint32_t tempcnt = 0;
    static uint16_t valuemax = 0;
    static uint16_t valuecompare = 0;
    static uint16_t valuenormal = 0;
    static float valueCurrent=0;
    static uint16_t leakcurrinitfinish = 0;
    float checkCntThreshold = 0;
    static float NewLeakCurrent = 0;
    static float NewLeakCurrent_rms = 0;
    static uint16_t count_rms = 0;
    static float NewLeakCurrent_SlidingWindow_average = 0;
    static float NewLeakCurrent_SlidingWindow_old = 0;
    static float NewLeakCurrent_SlidingWindow_sum = 0;
    static float NewLeakCurrent_SlidingWindow_new = 0;
    static float NewLeakCurrent_SlidingWindow_num = 400;
    static uint16_t flag_warning_LeakCurrent = 0;
    static uint32_t count_warning_LeakCurrent = 0;
    
    gLeakChecktime = (float)ModbusReg.residual_current_change_response_time;
    checkCntThreshold = gLeakChecktime * 20;// 1ms  1000 / 50 = 20
    _gInvSta = InverterGetState();

    gLeakCheckThreshold = (float)ModbusReg.residual_current_protection_threshold;

    if(leakcurrinitfinish == 1)
    { 
        valueCurrent = 0.99f * valueCurrent + 0.01f * abs(ADC_LEAK_CURRENT - valuenormal);
        NewLeakCurrent = (valueCurrent - 49.5) * 0.176 + 6.8;  //漏电流ADC采样值转换为实际电流值mA

        if(count_SlidingWindow < NewLeakCurrent_SlidingWindow_num && flag_SlidingWindow == 0) // 20260310，新增漏电流RMS有效值计算，使用滑动窗口算法
        {
            flag_SlidingWindow = 0;
            count_SlidingWindow++;
            NewLeakCurrent_SlidingWindow_sum = NewLeakCurrent_SlidingWindow_sum + (NewLeakCurrent*NewLeakCurrent);
            NewLeakCurrent_SlidingWindow_average = NewLeakCurrent_SlidingWindow_sum / count_SlidingWindow;
            NewLeakCurrent_rms = 0;
        }
        else
        {
            flag_SlidingWindow = 1;
            count_SlidingWindow = NewLeakCurrent_SlidingWindow_num;
            NewLeakCurrent_SlidingWindow_sum = (NewLeakCurrent_SlidingWindow_sum - NewLeakCurrent_SlidingWindow_average + (NewLeakCurrent*NewLeakCurrent));
            NewLeakCurrent_SlidingWindow_average = NewLeakCurrent_SlidingWindow_sum / NewLeakCurrent_SlidingWindow_num;
            NewLeakCurrent_rms = sqrt(NewLeakCurrent_SlidingWindow_average);
        }

        WATCH_LeakCurrent = NewLeakCurrent;
        WATCH_LeakCurrent_rms = NewLeakCurrent_rms;

        if(NewLeakCurrent_rms > gLeakCheckThreshold)
        {
            tempcnt++;
            if(tempcnt > checkCntThreshold)// 1ms  1000 / 50 = 20
            {
                tempcnt = checkCntThreshold;

                if((gLeakCheckThreshold != 0) && (gSysSetInfo.ignore_ct_err == 0))    //漏电流阈值设置成0时不报故障，测试模式忽略ct也不报故障 //ignore_ct_err
                {
                    gstFaultInfo.worning_code.b.leak_curr_high = 1;
                    flag_warning_LeakCurrent = 1;
                    count_warning_LeakCurrent = 0;
                }
            }
        }
        else
        {
            if(flag_warning_LeakCurrent == 1)
            {
                gstFaultInfo.worning_code.b.leak_curr_high = 1;
                count_warning_LeakCurrent++;
            }
            else
            {
                gstFaultInfo.worning_code.b.leak_curr_high = 0;
            }
            if(count_warning_LeakCurrent >= 600000) // 20260311，新增漏电流故障恢复，等待时间30s = 600000 / 20000
            {
                gstFaultInfo.worning_code.b.leak_curr_high = 0;
                flag_warning_LeakCurrent = 0;
                count_warning_LeakCurrent = 0;
            }
            tempcnt = 0;
        }
    }
    else
    {
        if(GPIO_ReadPin(29) == 1)   //GPIO2
        {
            tempcnt ++;
            valuemax = 0.95f * valuemax + 0.05f * ADC_LEAK_CURRENT;    //3mA的恒流源对应的值
            if(tempcnt > 40000)       //校准2秒 2000 000 / 50 = 40000
            {
                GPIO_WritePin(29,0);
                tempcnt = 0;
            }
            gLeak3ma = valuemax;
        }
        else
        {
            tempcnt ++;
            valuenormal  = 0.95f * valuenormal + 0.05f * ADC_LEAK_CURRENT;    //正常值
            if(tempcnt > 40000)       //校准2秒 2000 000 / 50 = 40000
            {
                leakcurrinitfinish = 1;
                tempcnt = 0;
            }
            gLeakCmp = valuecompare;
        }
    }
    valuecompare = abs(valuemax - valuenormal) * (gLeakCheckThreshold / 3.0f);
    gLeakCur = abs(ADC_LEAK_CURRENT - valuenormal);

    WATCH_flag_warning_LeakCurrent = flag_warning_LeakCurrent;
    WATCH_count_warning_LeakCurrent = count_warning_LeakCurrent;
}

/**
 * @brief
 * @param LeakCurrCheck
 * @param 拉弧绝缘检测函数
 * @return
 */

void ArcInsFaultDetect(void)
{
    static uint16_t trigger_count = 0;
    if(GPIO_ReadPin(GPIO1) == 0)//拉弧绝缘触发故障将该引脚拉低
    {
        if(!arc_fault_flag)//只需触发一次
        {
            trigger_count ++;
            if(trigger_count > 5)
            {
                gArcFaultStandbyFlag = 1;
//                gSysSetInfo.remote_standby_cmd = 1;
//                ModbusReg.remote_standby_cmd = 1;
                arc_fault_flag = 1;
            }
        }
    }
    else
    {
        trigger_count = 0;
        gArcFaultStandbyFlag = 0;
        arc_fault_flag = 0;
    }

}
