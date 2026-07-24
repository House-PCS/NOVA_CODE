/*
 * generator.c
 *
 *  Created on: 2024年11月18日
 *      Author: Nova007456
 */
#include "generator.h"
#include "F28x_Project.h"
#include "algo_info.h"
#include "energy_management.h"
#include "adc.h"
#include "sys_info.h"
#include "inverter_loop_cntl.h"
#include "algo_err_check.h"
#include "hard_cmpss.h"
#include "modbus_analysis.h"

GenInfoGet_t Gen;

/**
 * @brief
 * @param _GeneratorInit
 * @param 发电机初始化状态函数
 * @return
 */
static void _GeneratorInit(void)
{
    if(Gen.State.InitFinish == 0)
    {
        Gen.WorkMode = EN_GRID_MODE;
        Gen.State.InitFinish = 1;
    }
}
/**
 * @brief
 * @param _GridToGenConfig
 * @param 市电切发电机配置采样
 * @return
 */
static void _GridToGenConfig(void)
{
    //配置发电机电压和电流采样给市电的通道
    EALLOW;
    //市电电压采样 实际采发电机电压
    AdcdRegs.ADCSOC0CTL.bit.CHSEL = 1;  //SOC0 will convert Inv-1
    AdcdRegs.ADCSOC0CTL.bit.ACQPS = ACQPS_SYS_CLKS; //sample window in SYSCLK cycles
    AdcdRegs.ADCSOC0CTL.bit.TRIGSEL = ADC_TRIG_SOURCE; //trigger on ePWM1 SOCA/C

    //市电峰值电流采样 实际采发电机峰值电流
    AdcaRegs.ADCSOC2CTL.bit.CHSEL = 1;  //SOC2 will convert Inv-3
    AdcaRegs.ADCSOC2CTL.bit.ACQPS = ACQPS_SYS_CLKS; //sample window in SYSCLK cycles
    AdcaRegs.ADCSOC2CTL.bit.TRIGSEL = ADC_TRIG_SOURCE; //trigger

    //市电零点电流采样 实际采发电机零点电流
    AdcaRegs.ADCSOC10CTL.bit.CHSEL = 1;//SOC10 will convert
    AdcaRegs.ADCSOC10CTL.bit.ACQPS = ACQPS_SYS_CLKS; //sample window in SYSCLK cycles
    AdcaRegs.ADCSOC10CTL.bit.TRIGSEL = ADC_TRIG_SOURCE_SOCB; //trigger
    EDIS;
}
/**
 * @brief
 * @param _GenToGridConfig
 * @param 发电机切市电配置采样
 * @return
 */
static void _GenToGridConfig(void)
{
    EALLOW;
    //市电电压采样 恢复
    AdcdRegs.ADCSOC0CTL.bit.CHSEL = 0;  //SOC0 will convert Inv-1
    AdcdRegs.ADCSOC0CTL.bit.ACQPS = ACQPS_SYS_CLKS; //sample window in SYSCLK cycles
    AdcdRegs.ADCSOC0CTL.bit.TRIGSEL = ADC_TRIG_SOURCE; //trigger on ePWM1 SOCA/C

    //市电峰值电流采样 恢复
    AdcaRegs.ADCSOC2CTL.bit.CHSEL = 2;  //SOC2 will convert Inv-3
    AdcaRegs.ADCSOC2CTL.bit.ACQPS = ACQPS_SYS_CLKS; //sample window in SYSCLK cycles
    AdcaRegs.ADCSOC2CTL.bit.TRIGSEL = ADC_TRIG_SOURCE; //trigger

    //市电零点电流采样 恢复
    AdcaRegs.ADCSOC10CTL.bit.CHSEL = 2;//SOC10 will convert
    AdcaRegs.ADCSOC10CTL.bit.ACQPS = ACQPS_SYS_CLKS; //sample window in SYSCLK cycles
    AdcaRegs.ADCSOC10CTL.bit.TRIGSEL = ADC_TRIG_SOURCE_SOCB; //trigger
    EDIS;
}
/**
 * @brief
 * @param _GeneratorState
 * @param 发电机状态函数
 * @return
 */
static void _GeneratorState(void)
{
    static uint16_t delaycnt = 0;
    static uint16_t type_last = 0xFFFF;

    if(_gState_algo.inv_state == EN_INV_STATE_DEF_DISCONNECTED)
    {

        if(type_last==0xFFFF)
        {
            type_last = gSysSetInfo.charging_input_typesetting;
        }
        else
        {
            if(gSysSetInfo.charging_input_typesetting != type_last)
            {
                RelaySetValue(EN_RELAY_DEF_ENGINE, 0);
                RelaySetValue(EN_RELAY_DEF_GRID, 0);
            }
        }

        type_last = gSysSetInfo.charging_input_typesetting;
    }

    switch(Gen.WorkMode)
    {
        case EN_GRID_MODE://发电机待机模式（市电模式）
        {
            Gen.State.AdcGenToGridChangeFinish = 1;     //发电机切换至市电已完成
            Gen.State.AdcGridToGenChangeFinish = 0;

            if((gSysSetInfo.charging_input_typesetting == 1)   //强制发电机模式
                    &&(GetGridRlyState() == 0)                     //市电与发电机继电器已均断开
                    &&(GetEngineRlyState() == 0))
            {
                delaycnt++;
                if(delaycnt > 5)//给继电器和时序预留500ms动作时间
                {
                    delaycnt = 0;
                    Gen.WorkMode = EN_GRID_TO_GEN;
                    RelaySetValue(EN_RELAY_DEF_MAIN, 1);
                }
            }
            else
            {
                delaycnt = 0;
            }
            break;
        }
        case EN_GRID_TO_GEN://市电切换至发电机
        {
            _GridToGenConfig();
            Gen.WorkMode = EN_GEN_MODE;
            break;
        }
        case EN_GEN_MODE:
        {
            Gen.State.AdcGridToGenChangeFinish = 1; ///市电切换至发电机已完成
            Gen.State.AdcGenToGridChangeFinish = 0;

            if((gSysSetInfo.charging_input_typesetting == 0)   //强制发电机模式
                    &&(GetGridRlyState() == 0)                     //市电与发电机继电器已均断开
                    &&(GetEngineRlyState() == 0))
            {
                delaycnt++;
                if(delaycnt > 5)//给继电器和时序预留500ms动作时间
                {
                    delaycnt = 0;
                    Clear_cmpss_ost();
                    gAlgoErrDefSetCmpssCurOver(0);
                    gAlgoErrDefSetInvVoltOver(0);
                    gAlgoClearErr();
                    Gen.State.voltcheck = 0;
                    Gen.State.VoltNormal = 0;
                    Gen.State.LoadFault = 0;
                    Gen.WorkMode = EN_GEN_TO_GRID;
                    RelaySetValue(EN_RELAY_DEF_MAIN, 1);
                }
            }
            else
            {
                delaycnt = 0;
            }
            break;
        }
        case EN_GEN_TO_GRID:
        {
            _GenToGridConfig();
            Gen.WorkMode = EN_GRID_MODE;
            break;
        }
        default:
        {
            break;
        }
    }
}
/**
 * @brief
 * @param _GenVoltCheck
 * @param 发电机模式输入电压检查
 * @return
 */
static void _GenVoltCheck(void)
{
    static int overLoadCnt = 0;
    if((gSysSetInfo.charging_input_typesetting == 1) && (Gen.State.AdcGridToGenChangeFinish == 1)) //市电切换至发电机已完成
    {
        if((fabsf(_gState_algo.grid_vol_rms - Gen.State.VoltTemp) < 10) &&
           (_gState_algo.grid_vol_rms > 176.0f) &&
           (_gState_algo.grid_vol_rms < 264.0f) &&
           (fabsf(_gState_algo.grid_freq - 50.0f) < 10.0f))
        {
            Gen.State.voltcheck += 1;
            if(Gen.State.voltcheck > 50)
            {
                Gen.State.VoltNormal = 1;
                InverterSetGridConnect(1);
            }
        }
        else
        {
            Gen.State.VoltNormal = 0;
            Gen.State.voltcheck = 0;
        }

        Gen.State.VoltTemp = _gState_algo.grid_vol_rms;

        if(Gen.State.LoadFault == 0)//未过载
        {
            if((_gState_algo.load_pwr > 5500.0f) || (_gState_algo.load_cur_rms > 45.0f))
            {
                overLoadCnt++;
            }
            else
            {
                overLoadCnt--;
            }
            if(overLoadCnt > 10)//1秒
            {
                Gen.State.LoadFault = 1;
            }
            if(overLoadCnt < 0)
            {
                overLoadCnt = 0;
            }
        }
        else
        {
            Gen.State.LoadCheck += 1;
            if(Gen.State.LoadCheck > 300)//30秒
            {
                Gen.State.LoadFault = 0;
                Gen.State.LoadCheck = 0;
            }
        }
    }
    else
    {
        Gen.State.voltcheck = 0;
        Gen.State.VoltNormal = 0;
        Gen.State.LoadFault = 0;
        Gen.State.LoadCheck = 0;
    }
}
/**
 * @brief
 * @param _GenInvVoltCurrRecover
 * @param 发电机模式逆变电压电流恢复逻辑
 * @return
 */
static void _GenInvVoltCurrRecover(void)
{
    static uint16_t recovercnt = 0;

    if((Gen.State.AdcGridToGenChangeFinish == 1) && ((gAlgoErrDefGetCmpssCurOver() == 1) || (gAlgoErrDefGetInvVoltOver() == 1)))
    {
        recovercnt++;
        if(recovercnt > 50)//5S恢复
        {
            Clear_cmpss_ost();
            gAlgoErrDefSetCmpssCurOver(0);
            gAlgoErrDefSetInvVoltOver(0);
            gAlgoClearErr();
            recovercnt = 0;
        }
    }
    else
    {
        recovercnt = 0;
    }
}
/**
 * @brief
 * @param _GetGenModeLoadRlyState
 * @param 发电机模式负载继电器状态
 * @return
 */
static void _GetGenModeLoadRlyState(void)
{
    if(Gen.State.AdcGridToGenChangeFinish == 1)
    {
        if(GetLoadRlyState() == 1)
        {
            gSysSetInfo.backup_output_enable = 1;
            ModbusReg.backup_output_enable = 1;
            gRelayCloseFlag = 0;
        }
        else
        {
            gSysSetInfo.backup_output_enable = 0;
            ModbusReg.backup_output_enable = 0;
        }
    }
}
/**
 * @brief
 * @param GetGenToGridFinish
 * @param 获取发电机切换至市电完成标志位 置1有效
 * @return
 */
uint16_t GetGenToGridFinish(void)
{
    return (Gen.State.AdcGenToGridChangeFinish);
}
/**
 * @brief
 * @param Getvoltcheck
 * @param
 * @return
 */
uint16_t Getvoltcheck(void)
{
    return (Gen.State.voltcheck);
}
/**
 * @brief
 * @param GetGridToGenFinish
 * @param 获取市电切换至发电机完成标志位 置1有效
 * @return
 */
uint16_t GetGridToGenFinish(void)
{
    return (Gen.State.AdcGridToGenChangeFinish);
}
/**
 * @brief
 * @param GetGenVoltNormal
 * @param 获取发电机电压正常标志位
 * @return
 */
uint16_t GetGenVoltNormal(void)
{
    return (Gen.State.VoltNormal);
}
/**
 * @brief
 * @param GetGenLoadFault
 * @param 获取发电机负载异常,包含过载过流的标志位
 * @return
 */
uint16_t GetGenLoadFault(void)
{
    return (Gen.State.LoadFault);
}
/**
 * @param SetForceGenFlag
 * @param 设置强制发电机标志位 置1 强制发电机模式 置0 强制市电模式
 */
void SetForceGenFlag(uint16_t Value)
{
    if((Value == 0) || (Value == 1))
    {
        Gen.State.SysForceGenFlag = Value;
    }
}
/**
 * @param SetGenRlyOpenFinish
 * @param 发电机与市电切换的继电器已断开可以修改底层采样标志位 置1有效
 */
void SetGenRlyOpenFinish(uint16_t Value)
{
    if((Value == 0) || (Value == 1))
    {
        Gen.State.RlyOpenFinish = Value;
    }
}
/**
 * @brief
 * @param InputModeChange
 * @param 发电机与市电切换总函数(100ms)
 * @return
 */
void InputModeChange(void)
{
    _GeneratorInit();
    _GenVoltCheck();
    _GeneratorState();
    // _GenInvVoltCurrRecover(); //取消定时复位
//    _GetGenModeLoadRlyState();
}
