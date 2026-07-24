# 11 算法层 algo

对应源码：`algo/cla_control.*`、`algo/inv_3ph_pwm.*`、`algo/cla_cpu_share.*`、`algo/pv_cfg.h`  
（`AlgoFw` 库内部实现不在本文展开）

---

## 1. CPU↔CLA 共享：`cla_cpu_share`

### 结构 `CpuTransforClaData_t`

CPU→CLA：逆变三相电压电流、母线正负压、角度、频率、参考幅值、状态等。

### 存储段

- `gstCpuTransforClaData` → `CpuToCla1MsgRAM`  
- `garrDebugDataCla` → `Cla1ToCpuMsgRAM`  

写入方：`algo_interface`（状态、theta、w0、参考电压等）。  
读取方：CLA Task1。

---

## 2. 三相 PWM：`inv_3ph_pwm`

### 用途

T 型三电平、分裂母线逆变桥 PWM 更新。

### 限制

- 占空比约 ±0.97  
- 零区约 ±0.04  

### `Inv3PhPwm_Update`（新接口）

失能：关 GPIO45/六路桥臂并清误差。  
使能：限幅 → 零区误差累积补偿 → 分相更新。  

正负半周策略：

- 正调制：正桥臂 PWM，负桥臂关断，中点开关保持导通  
- 负调制：相反  
- 零区：两桥臂进中点续流  

GPIO45：HC245/驱动使能，由 CLA 控制。

### `ThreePhasePwmUpDate`（旧接口）

按符号直接控 EPWM1/2、3/4、5/6。

**现状风险**：CLA 仍可能调用旧接口，则新接口的限幅与零区补偿未真正生效——联调时需核对 `.cla` 实际调用。

---

## 3. CLA 控制：`cla_control`

### 触发

EPWM10 → CLA Task1，**20 kHz**。

### Task1 流程

1. 读 ECAP1 计数器  
2. 首次运行初始化逆变控制环  
3. 从 ADC 共享结果读逆变三相 V/I、母线正负压  
4. 从 CPU→CLA Message RAM 读状态、theta、频率、参考电压  
5. 三相正负序解耦  
6. `Inv3Ph_Calc_Cla_Run()`  
7. 写 EPWM1~6  
8. 更新 16 路 CLA 调试数据  
9. 算运行耗时，清 EPWM10 中断  

### 状态与 PWM 使能

| 状态类 | PWM |
|--------|-----|
| IDLE / ERR / SELFCHECK | 关闭 |
| STARTING / DISCONNECTED / CONNECTING / CONNECTED / DISCONNECTING | 使能并做正负序计算 |

### 注意

有功/无功参考可能被硬编码为 0（相关赋值注释掉），需与 `algo_interface` 下发路径对照确认。

---

## 4. PV 配置：`pv_cfg.h`

| 项 | 典型值 |
|----|--------|
| 通道数 | 3 |
| 电感 | 1 mH |
| 开关频率 | 40 kHz |
| 电压范围 | 70~450 V |
| 电流 | 17 A |
| 占空比 | 0.05~0.85 |
| 母线最大 | 950 V |
| 单路 / 总功率 | 8.5 kW / 25 kW |
| MPPT | 5 Hz，参考变化 ≤20 V/s，内环 5 kHz，平均窗 1000 |

`PV_LOW_VOL_TEST` 切换低压测试参数；其中母线启动电压与最大母线电压可能矛盾，视为配置缺陷。

PV 运行本体在 `AlgoFw` + `algo_interface`；本文件提供编译期参数。

---

## 5. 与系统时序关系

```
EPWM7 80kHz ──► CPU ISR ──► 采样 / 许可 / 写 CpuToClaMsg
EPWM10 20kHz ──► CLA Task1 ──► Inv3Ph_Calc_Cla_Run ──► EPWM1~6
主循环 200ms ──► Pv_Mppt_Run（CPU 侧）
1ms 能量调度 ──► MpptSetPvPwr / Inv P·Q 设定
```

---

## 6. 阅读建议

1. 先看 `cla_cpu_share.h` 字段含义  
2. 再看 `cla_control.cla` Task1 主循环  
3. 对照 `inv_3ph_pwm.cla` 实际被调用的 PWM 更新函数  
4. PV 参数以 `pv_cfg.h` + `algo_interface` 为准  
5. 源码原文见 [附录索引](./附录/00_附录索引.md)

---

## 附录：本章源码

> 以下为本章涉及的全部自研源码原文（不含 TI `include/`、`source/` 库）。

### `algo/cla_control.cla`

```c
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        cla_control.cla
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-8-30
  * @brief       CLA control task and interrupt handlers
  ******************************************************************************
  * @attention
  * @relation

  * @par Revision history:
  * <table>
  * <tr><th>Date          <th>Version   <th>Author
  * <tr><td>2024-8-30     <td>V1.0.0    <td>jindongze
  * </table>
  ******************************************************************************
  */
#include "cla_control.h"

#pragma DATA_SECTION(gstClaAdcData,"Cla1DataRam");
CLAAdc_t gstClaAdcData;
#pragma DATA_SECTION(gstAfpInvVol,"Cla1DataRam");
#pragma DATA_SECTION(gstAfpInvCurrL1,"Cla1DataRam");
AFP_FilterState_Cla_t gstAfpInvVol;
AFP_FilterState_Cla_t gstAfpInvCurrL1;
#pragma DATA_SECTION(gstInvVolDecoupleData,"Cla1DataRam");
#pragma DATA_SECTION(gstInvCurrL1DecoupleData,"Cla1DataRam");
Inv3Ph_DecoupleData_Cla_t gstInvVolDecoupleData;
Inv3Ph_DecoupleData_Cla_t gstInvCurrL1DecoupleData;
#pragma DATA_SECTION(gstClaInv3PhState,"Cla1DataRam");
EN_INV3PH_STATE gstClaInv3PhState;
#pragma DATA_SECTION(gClaRunTime,"Cla1DataRam");
#pragma DATA_SECTION(gstClaFirstRunFlag,"Cla1DataRam");
#pragma DATA_SECTION(gstClaPwmEnFlag,"Cla1DataRam");
float32 gClaRunTime;
float32 gstClaFirstRunFlag;
float32 gstClaPwmEnFlag;
/**
 * @brief Set CLA debug buffer value
 * @param buff_num Debug buffer index
 * @param value Value to write
 */
void SetClaDebugData(uint16_t buff_num,float value)
{
    garrDebugDataCla[buff_num] = (float)value;
}
/**
* @brief Clear CLA/EPWM interrupt flags
* @param void
* @return Nothing
* @note Clears EPWM10 interrupt and PIE group 3 ACK
*/
void ClearClaInterrupt(void)
{
    EPwm10Regs.ETCLR.bit.INT = 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

/**
 * @brief INV control initialization
 */
void InvClaCtrl_InDataUpdate(void)
{
    float32 single_phase_active_pwr;
    float32 single_phase_reactive_pwr;
    /***************************ADC数据更新(直接从ADC计算后的采样数据快速拿值)***********************************/
    gstClaAdcData.inv_volt_phase_a = gstClaAdcResult.inv_volt_phase_a;
    gstClaAdcData.inv_volt_phase_b = gstClaAdcResult.inv_volt_phase_b;
    gstClaAdcData.inv_volt_phase_c = gstClaAdcResult.inv_volt_phase_c;

    gstClaAdcData.inv_curr_phase_a = gstClaAdcResult.inv_curr_phase_a;
    gstClaAdcData.inv_curr_phase_b = gstClaAdcResult.inv_curr_phase_b;
    gstClaAdcData.inv_curr_phase_c = gstClaAdcResult.inv_curr_phase_c;

    gstClaAdcData.bus_volt_neg = gstClaAdcResult.bus_volt_neg;
    gstClaAdcData.bus_volt_pos = gstClaAdcResult.bus_volt_pos;

    /**************************************从业务端进行数据获取*********************************************/
//    gstClaInv3PhState = gstClaInvRunState;
//    gstClaAdcData.inv_volt_phase_a = gstCpuTransforClaData.inv_vol_a;
//    gstClaAdcData.inv_volt_phase_b = gstCpuTransforClaData.inv_vol_b;
//    gstClaAdcData.inv_volt_phase_c = gstCpuTransforClaData.inv_vol_c;
//
//    gstClaAdcData.inv_curr_phase_a = gstCpuTransforClaData.inv_curr_L1_a;
//    gstClaAdcData.inv_curr_phase_b = gstCpuTransforClaData.inv_curr_L1_b;
//    gstClaAdcData.inv_curr_phase_c = gstCpuTransforClaData.inv_curr_L1_c;
//
//    gstClaAdcData.bus_volt_neg = gstCpuTransforClaData.bus_vol_neg;
//    gstClaAdcData.bus_volt_pos = gstCpuTransforClaData.bus_vol_pos;

    gInv3Ph_CalcData_Cla.bus_volt = gstClaAdcData.bus_volt_neg + gstClaAdcData.bus_volt_pos;
    gInv3Ph_CalcData_Cla.bus_volt_neg =     gstClaAdcData.bus_volt_neg;
    gInv3Ph_CalcData_Cla.bus_volt_pos =     gstClaAdcData.bus_volt_pos;
    gInv3Ph_CalcData_Cla.state = gstCpuTransforClaData.ClaInv3PhState;
    gInv3Ph_CalcData_Cla.theta = gstCpuTransforClaData.theta;
    gInv3Ph_CalcData_Cla.volt_ref = gstCpuTransforClaData.vol_amp_ref;
    gInv3Ph_CalcData_Cla.w = gstCpuTransforClaData.w0;
    gstClaInv3PhState = gstCpuTransforClaData.ClaInv3PhState;

    /******************************当前调试暂时不考虑防逆流分功率控制********************************/
    single_phase_active_pwr = gstPwrCtrlInterface.inv_active_pwr / 3.0f;
    single_phase_reactive_pwr = gstPwrCtrlInterface.inv_reactive_pwr / 3.0f;

//    gInv3Ph_CalcData_Cla.pwr_pa_ref = single_phase_active_pwr;
//    gInv3Ph_CalcData_Cla.pwr_pb_ref = single_phase_active_pwr;
//    gInv3Ph_CalcData_Cla.pwr_pc_ref = single_phase_active_pwr;
//
//    gInv3Ph_CalcData_Cla.pwr_qa_ref = single_phase_reactive_pwr;
//    gInv3Ph_CalcData_Cla.pwr_qb_ref = single_phase_reactive_pwr;
//    gInv3Ph_CalcData_Cla.pwr_qc_ref =  single_phase_reactive_pwr;
    gInv3Ph_CalcData_Cla.pwr_pa_ref = 0;
    gInv3Ph_CalcData_Cla.pwr_pb_ref = 0;
    gInv3Ph_CalcData_Cla.pwr_pc_ref = 0;

    gInv3Ph_CalcData_Cla.pwr_qa_ref = 0;
    gInv3Ph_CalcData_Cla.pwr_qb_ref = 0;
    gInv3Ph_CalcData_Cla.pwr_qc_ref =  0;
}

/**
 * @brief INV control initialization
 */
void InvClaCtrl_PosNegData(float32 theta)
{

    gstInvVolDecoupleData = Comp_3phDecouple_Run_Expanded_Cla(&gstAfpInvVol,theta,gstClaAdcData.inv_volt_phase_a,\
                                                              gstClaAdcData.inv_volt_phase_b,gstClaAdcData.inv_volt_phase_c);
    gInv3Ph_CalcData_Cla.vc_dp = gstInvVolDecoupleData.dp;
    gInv3Ph_CalcData_Cla.vc_dn = gstInvVolDecoupleData.dn;
    gInv3Ph_CalcData_Cla.vc_qp = gstInvVolDecoupleData.qp;
    gInv3Ph_CalcData_Cla.vc_qn = gstInvVolDecoupleData.qn;
    gstInvCurrL1DecoupleData = Comp_3phDecouple_Run_Expanded_Cla(&gstAfpInvCurrL1,theta,gstClaAdcData.inv_curr_phase_a,\
                                                              gstClaAdcData.inv_curr_phase_b,gstClaAdcData.inv_curr_phase_c);
    gInv3Ph_CalcData_Cla.iL1_dp = gstInvCurrL1DecoupleData.dp;
    gInv3Ph_CalcData_Cla.iL1_dn = gstInvCurrL1DecoupleData.dn;
    gInv3Ph_CalcData_Cla.iL1_qp = gstInvCurrL1DecoupleData.qp;
    gInv3Ph_CalcData_Cla.iL1_qn = gstInvCurrL1DecoupleData.qn;

    gInv3Ph_CalcData_Cla.vc_z = (gstClaAdcData.inv_volt_phase_a + gstClaAdcData.inv_volt_phase_b + \
                                gstClaAdcData.inv_volt_phase_c) / 3.0f;
}

/**
 * @brief INV control initialization
 */
void InvClaCtrl_Init(void)
{
    Inv3Ph_Calc_Cla_Init();
    AFP_FilterState_Init_Cla(&gstAfpInvVol);
    AFP_FilterState_Init_Cla(&gstAfpInvCurrL1);
    gstInvVolDecoupleData.dn = 0;
    gstInvVolDecoupleData.dp = 0;
    gstInvVolDecoupleData.qn = 0;
    gstInvVolDecoupleData.qp = 0;
    gstInvVolDecoupleData.zn = 0;
    gstInvVolDecoupleData.zp = 0;

    gstInvCurrL1DecoupleData.dn = 0;
    gstInvCurrL1DecoupleData.dp = 0;
    gstInvCurrL1DecoupleData.qn = 0;
    gstInvCurrL1DecoupleData.qp = 0;
    gstInvCurrL1DecoupleData.zn = 0;
    gstInvCurrL1DecoupleData.zp = 0;

    gstClaPwmEnFlag = 0;
}

/**
 * @brief Clear INV control loops
 */
void InvClaCtrl_ClearLoops(void)
{
    AFP_FilterState_Init_Cla(&gstAfpInvVol);
    AFP_FilterState_Init_Cla(&gstAfpInvCurrL1);
    gstInvVolDecoupleData.dn = 0;
    gstInvVolDecoupleData.dp = 0;
    gstInvVolDecoupleData.qn = 0;
    gstInvVolDecoupleData.qp = 0;
    gstInvVolDecoupleData.zn = 0;
    gstInvVolDecoupleData.zp = 0;

    gstInvCurrL1DecoupleData.dn = 0;
    gstInvCurrL1DecoupleData.dp = 0;
    gstInvCurrL1DecoupleData.qn = 0;
    gstInvCurrL1DecoupleData.qp = 0;
    gstInvCurrL1DecoupleData.zn = 0;
    gstInvCurrL1DecoupleData.zp = 0;

//    gInv3Ph_CalcData_Cla.duty_A = 0;
//    gInv3Ph_CalcData_Cla.duty_B = 0;
//    gInv3Ph_CalcData_Cla.duty_C = 0;
//
//    gInv3Ph_CalcData_Cla.volt_ref = 0;
}

void Inv3PhSwitchRun(void)
{
//    gInv3Ph_CalcData_Cla.theta += gInv3Ph_CalcData_Cla.w * gInv3Ph_CalcData_Cla.Ts;
//    if(gInv3Ph_CalcData_Cla.theta >= M_2PI)
//    {
//        gInv3Ph_CalcData_Cla.theta -= M_2PI;
//    }
//
    switch (gstClaInv3PhState)
    {
        case EN_INV3PH_STATE_IDLE:
        {
            InvClaCtrl_ClearLoops();
            gstClaPwmEnFlag = 0;
            break;
        }
        case EN_INV3PH_STATE_ERR:
        {
            InvClaCtrl_ClearLoops();
            gstClaPwmEnFlag = 0;
            break;
        }
        case EN_INV3PH_STATE_SELFCHECK:
        {
            gstClaPwmEnFlag = 0;
            break;
        }
        case EN_INV3PH_STATE_STARTING:
        {
            gstClaPwmEnFlag = 1;
            InvClaCtrl_PosNegData(gInv3Ph_CalcData_Cla.theta);
            break;
        }
        case EN_INV3PH_STATE_DISCONNECTED:
        {
            gstClaPwmEnFlag = 1;
            InvClaCtrl_PosNegData(gInv3Ph_CalcData_Cla.theta);
            break;
        }
        case EN_INV3PH_STATE_CONNECTING:
        {
            gstClaPwmEnFlag = 1;
            InvClaCtrl_PosNegData(gInv3Ph_CalcData_Cla.theta);
            break;
        }
        case EN_INV3PH_STATE_CONNECTED:
        {
            gstClaPwmEnFlag = 1;
            InvClaCtrl_PosNegData(gInv3Ph_CalcData_Cla.theta);
            break;
        }
        case EN_INV3PH_STATE_DISCONNECTING:
        {
            gstClaPwmEnFlag = 1;
            InvClaCtrl_PosNegData(gInv3Ph_CalcData_Cla.theta);
            break;
        }
        default:

            break;
    }
}

void InvClaCtrl_DebugDataUpdate(void)
{
    SetClaDebugData(0,gstClaInv3PhState);
    SetClaDebugData(1,gstClaAdcData.inv_volt_phase_a);
    SetClaDebugData(2,gstClaAdcData.inv_volt_phase_b);
    SetClaDebugData(3,gstClaAdcData.inv_volt_phase_c);
    SetClaDebugData(4,gstClaAdcData.inv_curr_phase_a);
    SetClaDebugData(5,gstClaAdcData.inv_curr_phase_b);
    SetClaDebugData(6,gstClaAdcData.inv_curr_phase_c);

    SetClaDebugData(7,gstClaAdcData.bus_volt_neg);
    SetClaDebugData(8,gstClaAdcData.bus_volt_pos);
    SetClaDebugData(9,gClaRunTime);
    SetClaDebugData(10,gInv3Ph_CalcData_Cla.theta);
    SetClaDebugData(11,gInv3Ph_CalcData_Cla.volt_ref);
    SetClaDebugData(12,gInv3Ph_CalcData_Cla.w);
    SetClaDebugData(13,gInv3Ph_CalcData_Cla.pwr_pa_ref);
    SetClaDebugData(14,gInv3Ph_CalcData_Cla.pwr_pb_ref);
    SetClaDebugData(15,gInv3Ph_CalcData_Cla.pwr_pc_ref);
}
/**
 * @brief INV control periodic run
 */
void InvClaCtrl_Run(void)
{
    if(gstClaFirstRunFlag == 0)
    {
        gstClaFirstRunFlag = 1;
        InvClaCtrl_Init();
    }
    //更新逆变数据
    InvClaCtrl_InDataUpdate();
    //逆变控制函数运行
    Inv3PhSwitchRun();
    //逆变代码运行
    Inv3Ph_Calc_Cla_Run();
    //占空比数据更新
//    Inv3PhPwm_Update(gInv3Ph_CalcData_Cla.duty_A,gInv3Ph_CalcData_Cla.duty_B,\
//                     gInv3Ph_CalcData_Cla.duty_C,gstClaPwmEnFlag);
    ThreePhasePwmUpDate(gInv3Ph_CalcData_Cla.duty_A,gInv3Ph_CalcData_Cla.duty_B,\
                        gInv3Ph_CalcData_Cla.duty_C,gstClaPwmEnFlag);
    //Debug数据更新
    InvClaCtrl_DebugDataUpdate();
}

/**
* @brief CLA task entry (timing and interrupt housekeeping only)
* @param
* @return Nothing
* @note Do not place real-time control algorithms here; implement them in CLA_TASK
*/
void CLA_Task(void)
{
    float32 cla_time;
    float32 cla_time_next;
    cla_time = ECap1Regs.TSCTR;
    InvClaCtrl_Run();
    cla_time_next = ECap1Regs.TSCTR;
    if(cla_time > cla_time_next)
    {
        gClaRunTime = cla_time_next + (0xffffffffUL - cla_time);
    }
    else
    {
        gClaRunTime = cla_time_next - cla_time;
    }
    ClearClaInterrupt();
}

__attribute__((interrupt)) void Cla1Task1 (void)
{
    CLA_Task();
}
__attribute__((interrupt)) void Cla1Task2 (void)
{

}
__attribute__((interrupt)) void Cla1Task3 (void)
{

}
__attribute__((interrupt)) void Cla1Task4 (void)
{

}
__attribute__((interrupt)) void Cla1Task5 (void)
{

}
__attribute__((interrupt)) void Cla1Task6 (void)
{

}
__attribute__((interrupt)) void Cla1Task7 (void)
{

}
__attribute__((interrupt)) void Cla1Task8 (void)
{

}
```

### `algo/cla_control.h`

```c

/*
 * cla_control.h
 *
 *  Created on: 2024-08-27
 *      Author: Nova007456
 */
#ifndef MIDDLEWARE_CLA_ALGO_H_
#define MIDDLEWARE_CLA_ALGO_H_
#endif

#include "F28x_Project.h"
#include "F2837xD_Cla_defines.h"
#include "stdint.h"
#include "CLAMath.h"
#include "system.h"
#include "fault.h"
#include <sample_update.h>
#include "interrupt.h"
#include "cla_cpu_share.h"
#include "AlgoFw/package/cntl/inverter_3ph/inverter_3ph_calc_cla.h"
#include "AlgoFw/component/Comp_3phDecouple_cla.h"
#include "AlgoFw/component/Comp_Clark_cla.h"
#include "AlgoFw/component/Comp_Park_cla.h"
#include "pwr_ctrl_interface.h"
#include "inv_3ph_pwm.h"

extern CLAAdc_t  gstClaAdcResult;
extern Inv3Ph_Vloop_t gVloop_p_Cla;
extern Inv3Ph_Vloop_t gVloop_n_Cla;
extern Inv3Ph_Iloop_t gIloop_p_Cla;
extern Inv3Ph_Iloop_t gIloop_n_Cla;
extern Inv3Ph_Iloop_t gIloop_z_Cla;
extern Inv3Ph_Calc_Data_t gInv3Ph_CalcData_Cla;
//extern PwrCtrlInteface_t gstPwrCtrlInterface;





void SetClaDebugData(uint16_t buff_num,float value);

void InvClaCtrl_Init(void);
void InvClaCtrl_Run(void);
void InvClaCtrl_ClearLoops(void);
```

### `algo/cla_cpu_share.c`

```c
/*
 * cla_cpu_share.c
 *
 *  Created on: 2025-03-25
 *      Author: Nova007456
 */
#include "cla_cpu_share.h"

volatile float32 garrDebugDataCla[16];
#pragma DATA_SECTION(garrDebugDataCla,"Cla1ToCpuMsgRAM");

#pragma DATA_SECTION(gstCpuTransforClaData,"CpuToCla1MsgRAM");
volatile CpuTransforClaData_t   gstCpuTransforClaData;
```

### `algo/cla_cpu_share.h`

```c
/*
 * cla_cpu_share.h
 *
 *  Created on: 2025-03-25
 *      Author: Nova007456
 */
#ifndef APP_CLA_CPU_SHARE_H_
#define APP_CLA_CPU_SHARE_H_

#include "F28x_Project.h"
#include "cap.h"
#include "AlgoFw/package/cntl/inverter_3ph/inverter_3ph_def.h"
typedef struct cpu_transfor_cla_data_t
{
    float32 inv_vol_a;
    float32 inv_vol_b;
    float32 inv_vol_c;

    float32 inv_curr_L1_a;
    float32 inv_curr_L1_b;
    float32 inv_curr_L1_c;

    float32 inv_curr_L2_a;
    float32 inv_curr_L2_b;
    float32 inv_curr_L2_c;

    float32 bus_vol;
    float32 bus_vol_pos;
    float32 bus_vol_neg;

    float32 theta;
    float32 theta_offset;

    float32 w0;
    float32 w_offset;

    float32 vol_amp_ref;
    float32 vol_amp_ref_offset;

    EN_INV3PH_STATE ClaInv3PhState;

} CpuTransforClaData_t;

extern volatile CpuTransforClaData_t   gstCpuTransforClaData;
extern volatile float garrDebugDataCla[16];

#endif /* APP_CLA_CPU_SHARE_H_ */
```

### `algo/inv_3ph_pwm.cla`

```c
/**
 * @file inv_3ph_pwm.c
 * @brief Three-phase T-type three-level split-bus EPWM driver
 * @note EPWMxA = bridge arm, EPWMxB = neutral-point complementary switch
 */

#include "inv_3ph_pwm.h"
#include "F28x_Project.h"

volatile float32 gInv3PhPwm_duty_err[3];
#pragma DATA_SECTION(gInv3PhPwm_duty_err,"Cla1DataRam");

#define INV_3PH_PWM_DUTY_CLAMP(x) \
    (((x) > INV_3PH_PWM_DUTY_MAX) ? INV_3PH_PWM_DUTY_MAX : (((x) < INV_3PH_PWM_DUTY_MIN) ? INV_3PH_PWM_DUTY_MIN : (x)))

#define INV_3PH_PWM_FABS(x) (((x) >= 0.0f) ? (x) : -(x))

void Inv3PhPwm_ErrClear(void)
{
    gInv3PhPwm_duty_err[0] = 0.0f;
    gInv3PhPwm_duty_err[1] = 0.0f;
    gInv3PhPwm_duty_err[2] = 0.0f;
}

static float Inv3PhPwm_DeadZoneWithErr(float duty_raw, volatile float *p_duty_err)
{
    float duty_temp;
    float duty_eff;

    duty_temp = duty_raw + (*p_duty_err);
    if((duty_temp > -INV_3PH_PWM_ZERO_BAND) && (duty_temp < INV_3PH_PWM_ZERO_BAND))
    {
        duty_eff = 0.0f;
    }
    else
    {
        duty_eff = duty_temp;
    }
    (*p_duty_err) = duty_temp - duty_eff;
    return duty_eff;
}

static Uint16 Inv3PhPwm_DutyToTicks(float duty, Uint16 tbprd)
{
    return (Uint16)((float)tbprd * (1.0f - INV_3PH_PWM_FABS(duty)));
}

/**
 * @brief Force both EPWM outputs low (full off)
 */
static void Inv3PhPwm_ForceOutputOff(volatile struct EPWM_REGS *epwm)
{
    epwm->AQCSFRC.bit.CSFA = 0x01;
    epwm->AQCSFRC.bit.CSFB = 0x01;
    epwm->CMPA.bit.CMPA = epwm->TBPRD;
    epwm->CMPB.bit.CMPB = 0;
}

static void Inv3PhPwm_ShutdownAll(void)
{
    Inv3PhPwm_ForceOutputOff(&EPwm1Regs);
    Inv3PhPwm_ForceOutputOff(&EPwm2Regs);
    Inv3PhPwm_ForceOutputOff(&EPwm3Regs);
    Inv3PhPwm_ForceOutputOff(&EPwm4Regs);
    Inv3PhPwm_ForceOutputOff(&EPwm5Regs);
    Inv3PhPwm_ForceOutputOff(&EPwm6Regs);
}

/**
 * @brief Force freewheel path: bridge arm off (A low) + neutral switch on (B high)
 */
static void Inv3PhPwm_ForceBridgeOffMiddleOn(volatile struct EPWM_REGS *epwm)
{
    epwm->AQCSFRC.bit.CSFA = 0x01;
    epwm->AQCSFRC.bit.CSFB = 0x02;
    epwm->CMPA.bit.CMPA = epwm->TBPRD;
    epwm->CMPB.bit.CMPB = epwm->TBPRD;
}

/**
 * @brief T-type three-level zero state: both pos/neg bridge off, neutral on (clamped near 0)
 */
static void Inv3PhPwm_PhaseZeroState(volatile struct EPWM_REGS *epwm_pos,
                                     volatile struct EPWM_REGS *epwm_neg)
{
    Inv3PhPwm_ForceBridgeOffMiddleOn(epwm_pos);
    Inv3PhPwm_ForceBridgeOffMiddleOn(epwm_neg);
}

static void Inv3PhPwm_PhaseUpdate(float duty, Uint16 duty_ticks,
                                  volatile struct EPWM_REGS *epwm_pos,
                                  volatile struct EPWM_REGS *epwm_neg)
{
    if(INV_3PH_PWM_FABS(duty) < INV_3PH_PWM_ZERO_BAND)
    {
        Inv3PhPwm_PhaseZeroState(epwm_pos, epwm_neg);
    }
    else if(duty >= 0.0f)
    {
        /** Positive half-cycle: PWM on positive bridge; negative bridge off + neutral on */
        Inv3PhPwm_ForceBridgeOffMiddleOn(epwm_neg);
        epwm_pos->CMPA.bit.CMPA = duty_ticks;
        epwm_pos->CMPB.bit.CMPB = duty_ticks;
    }
    else
    {
        /** Negative half-cycle: PWM on negative bridge; positive bridge off + neutral on */
        Inv3PhPwm_ForceBridgeOffMiddleOn(epwm_pos);
        epwm_neg->CMPA.bit.CMPA = duty_ticks;
        epwm_neg->CMPB.bit.CMPB = duty_ticks;
    }
}

static void Inv3PhPwm_ThreePhaseUpdate(float duty_a, float duty_b, float duty_c)
{
    Uint16 tbprd;
    Uint16 duty_a_ticks;
    Uint16 duty_b_ticks;
    Uint16 duty_c_ticks;

    tbprd = EPwm1Regs.TBPRD;
    duty_a_ticks = Inv3PhPwm_DutyToTicks(duty_a, tbprd);
    duty_b_ticks = Inv3PhPwm_DutyToTicks(duty_b, tbprd);
    duty_c_ticks = Inv3PhPwm_DutyToTicks(duty_c, tbprd);

    EPwm1Regs.AQCSFRC.all = 0;
    EPwm2Regs.AQCSFRC.all = 0;
    EPwm3Regs.AQCSFRC.all = 0;
    EPwm4Regs.AQCSFRC.all = 0;
    EPwm5Regs.AQCSFRC.all = 0;
    EPwm6Regs.AQCSFRC.all = 0;

    Inv3PhPwm_PhaseUpdate(duty_a, duty_a_ticks, &EPwm1Regs, &EPwm2Regs);
    Inv3PhPwm_PhaseUpdate(duty_b, duty_b_ticks, &EPwm3Regs, &EPwm4Regs);
    Inv3PhPwm_PhaseUpdate(duty_c, duty_c_ticks, &EPwm5Regs, &EPwm6Regs);
}

uint16_t Inv3PhPwm_Update(float duty_a, float duty_b, float duty_c, uint16_t en)
{
    float duty_a_eff;
    float duty_b_eff;
    float duty_c_eff;

    if(en == 0U)
    {
        GpioDataRegs.GPBCLEAR.bit.GPIO45 = 1;
        Inv3PhPwm_ShutdownAll();
        Inv3PhPwm_ErrClear();
        return 0U;
    }

    GpioDataRegs.GPBSET.bit.GPIO45 = 1;

    duty_a = INV_3PH_PWM_DUTY_CLAMP(duty_a);
    duty_b = INV_3PH_PWM_DUTY_CLAMP(duty_b);
    duty_c = INV_3PH_PWM_DUTY_CLAMP(duty_c);

    duty_a_eff = Inv3PhPwm_DeadZoneWithErr(duty_a, &gInv3PhPwm_duty_err[0]);
    duty_b_eff = Inv3PhPwm_DeadZoneWithErr(duty_b, &gInv3PhPwm_duty_err[1]);
    duty_c_eff = Inv3PhPwm_DeadZoneWithErr(duty_c, &gInv3PhPwm_duty_err[2]);

    Inv3PhPwm_ThreePhaseUpdate(duty_a_eff, duty_b_eff, duty_c_eff);
    return 1U;
}

/**
* @brief: ThreePhasePwmUpDate
* @param: static  void
* @return:Nothing
* @note:
*/
void ThreePhasePwmUpDate(float dutyA,float dutyB,float dutyC,float en_flag)
{
    Uint16  DutyA_ticks = 0;
    Uint16  DutyB_ticks = 0;
    Uint16  DutyC_ticks = 0;

    //待机模式就封波
    if(!en_flag)
    {
        GpioDataRegs.GPBCLEAR.bit.GPIO45 = 1;
    }
    else
    {
        GpioDataRegs.GPBSET.bit.GPIO45 = 1;
    }

    DutyA_ticks = (Uint16)((float)(EPwm1Regs.TBPRD)* (1.0f - fabsf(dutyA)));
    DutyB_ticks = (Uint16)((float)(EPwm1Regs.TBPRD)* (1.0f - fabsf(dutyB)));
    DutyC_ticks = (Uint16)((float)(EPwm1Regs.TBPRD)* (1.0f - fabsf(dutyC)));


    /*  说明：Q13为硬件原理图中的Q13,SA4为MATLAB仿真模型中的SA4
     * A组当调制波大于0时，Q13（SA4）强制为低  EPWM2A，Q8(SA2)强制为高电平  EPWM2B
     * A组当调制波小于零时，Q4(SA1)强制为低电平 EPWM1A,Q7(SA3)强制为高电平  EPWM1B
     * B组当调制波大于0时，Q14（SB4）强制为低  EPWM4A，Q10(SB2)强制为高电平 EPWM4B
     * B组当调制波小于零时，Q5(SB1)强制为低电平 EPWM3A,Q9(SB3)强制为高电平  EPWM3B
     * C组当调制波大于0时，Q15（SC4）强制为低  EPWM6A ，Q12(SC2)强制为高电平EPWM6B
     * C组当调制波小于零时，Q6(SC1)强制为低电平 EPWM5A,Q11(SC3)强制为高电平 EPWM5B*/
    EPwm1Regs.AQCSFRC.all = 0;
    EPwm2Regs.AQCSFRC.all = 0;
    EPwm3Regs.AQCSFRC.all = 0;
    EPwm4Regs.AQCSFRC.all = 0;
    EPwm5Regs.AQCSFRC.all = 0;
    EPwm6Regs.AQCSFRC.all = 0;

    if(dutyA >= 0.0f)
    {
        EPwm2Regs.AQCSFRC.bit.CSFA = 0x01;      //强制Q8的发波驱动为低
        EPwm2Regs.AQCSFRC.bit.CSFB = 0x02;      //强制Q13的对应的发波驱动为高(0x01代表低，0x02代表高电平)
        EPwm1Regs.CMPA.bit.CMPA = DutyA_ticks; //Q4发波
        EPwm1Regs.CMPB.bit.CMPB = DutyA_ticks; //Q7发波
    }
    else
    {
        EPwm1Regs.AQCSFRC.bit.CSFA = 0x01;      //强制Q4的对应的发波驱动为低(0x01代表低，0x02代表高电平)
        EPwm1Regs.AQCSFRC.bit.CSFB = 0x02;      //强制Q7的发波驱动为高
        EPwm2Regs.CMPA.bit.CMPA = DutyA_ticks; //Q8发波
        EPwm2Regs.CMPB.bit.CMPB = DutyA_ticks; //Q13发波
    }

    if(dutyB >= 0.0f)
    {
        EPwm4Regs.AQCSFRC.bit.CSFA = 0x01;      //强制Q10的发波驱动为低
        EPwm4Regs.AQCSFRC.bit.CSFB = 0x02;      //强制Q14的对应的发波驱动为高(0x01代表低，0x02代表高电平)
        EPwm3Regs.CMPA.bit.CMPA = DutyB_ticks; //Q5发波
        EPwm3Regs.CMPB.bit.CMPB = DutyB_ticks; //Q9发波
    }
    else
    {
        EPwm3Regs.AQCSFRC.bit.CSFA = 0x01;      //强制Q5的对应的发波驱动为低(0x01代表低，0x02代表高电平)
        EPwm3Regs.AQCSFRC.bit.CSFB = 0x02;      //强制Q9的发波驱动为高
        EPwm4Regs.CMPA.bit.CMPA = DutyB_ticks; //Q14发波
        EPwm4Regs.CMPB.bit.CMPB = DutyB_ticks; //Q10发波
    }

    if(dutyC >= 0.0f)
    {
        EPwm6Regs.AQCSFRC.bit.CSFA = 0x01;      //强制Q12的发波驱动为低
        EPwm6Regs.AQCSFRC.bit.CSFB = 0x02;      //强制Q15的对应的发波驱动为高(0x01代表低，0x02代表高电平)
        EPwm5Regs.CMPA.bit.CMPA = DutyC_ticks; //Q6发波
        EPwm5Regs.CMPB.bit.CMPB = DutyC_ticks; //Q11发波
    }
    else
    {
        EPwm5Regs.AQCSFRC.bit.CSFA = 0x01;      //强制Q6的对应的发波驱动为低(0x01代表低，0x02代表高电平)
        EPwm5Regs.AQCSFRC.bit.CSFB = 0x02;      //强制Q11的发波驱动为高
        EPwm6Regs.CMPA.bit.CMPA = DutyC_ticks; //Q15发波
        EPwm6Regs.CMPB.bit.CMPB = DutyC_ticks; //Q12发波
    }
}
```

### `algo/inv_3ph_pwm.h`

```c
/**
 * @file inv_3ph_pwm.h
 * @brief Three-phase T-type three-level split-bus EPWM driver
 * @note EPWMxA = bridge arm, EPWMxB = neutral-point switch; zero state = bridge off + middle on
 */

#ifndef DEVICE_INV_3PH_PWM_H_
#define DEVICE_INV_3PH_PWM_H_

#include "stdint.h"

#define INV_3PH_PWM_DUTY_MAX        (0.97f)
#define INV_3PH_PWM_DUTY_MIN        (-0.97f)
#define INV_3PH_PWM_ZERO_BAND       (0.04f)

void Inv3PhPwm_ErrClear(void);

uint16_t Inv3PhPwm_Update(float duty_a, float duty_b, float duty_c, uint16_t en);
void ThreePhasePwmUpDate(float dutyA,float dutyB,float dutyC,float en_flag);
#endif /* DEVICE_INV_3PH_PWM_H_ */
```

### `algo/pv_cfg.h`

```c
/**
 * @file pv_cfg.h
 * @brief 光伏(PV)控制系统配置头文件
 * @details 定义了光伏控制系统的所有配置参数，包括：
 *          - 通道数量配置
 *          - 电压/电流/功率限制参数
 *          - MPPT控制参数
 *          - 控制环路参数
 *          - 平均计算参数
 * @note 支持通过 PV_LOW_VOL_TEST 宏切换低压测试模式和正常模式
 */
#ifndef PV_CFG_H
#define PV_CFG_H

/**
 * @brief Boost 电感值（H），用于PV电压混合模式卡尔曼观测器
 */
#define PV_HW_INDUCTANCE_H         (1000e-6f)

/** 开关频率（Hz），物理硬件常数 */
#define PV_HW_SWITCH_FREQ_HZ       (40000.0f)

/* ========================================================================
 * 系统配置
 * ======================================================================== */

/**
 * @brief 光伏通道数量
 */
#define PV_CHANNELS          3

/* ========================================================================
 * 电压/电流/功率限制配置
 * ======================================================================== */

#ifdef PV_LOW_VOL_TEST
/* 低压测试模式参数 */

/** @brief 通道最低工作电压(V) */
#define PV_CHANNEL_MIN_VOLTAGE       50.0f

/** @brief 通道最高工作电压(V) */
#define PV_CHANNEL_MAX_VOLTAGE       120.0f

/** @brief 通道最大工作电流(A) */
#define PV_CHANNEL_MAX_CURRENT       16.0f

/** @brief 通道最大占空比(0~1) */
#define PV_CHANNEL_MAX_DUTY          0.95f

/** @brief 通道最小占空比(0~1) */
#define PV_CHANNEL_MIN_DUTY          0.05f

/** @brief 母线最大电压(V) */
#define PV_BUS_MAX_VOLTAGE           120.0f

/** @brief 母线启动电压阈值(V) */
#define PV_BUS_START_UP_VOLTAGE      650.0f

/** @brief 母线正常运行限制电压(V) */
#define PV_BUS_NOMAL_LIMIT_VOLTAGE   880.0f

/** @brief 单通道最大功率(W) */
#define PV_CHANNEL_MAX_POWER         1000.0f

/** @brief 系统总功率限制(W) */
#define PV_TOTAL_MAX_POWER           4000.0f



#define PV_ERR_CURR_OVER_THD          (16.0f) //过流告警阈值，单位A
#define PV_ERR_BUS_VOLT_OVER_THD      (125.0f) //母线过压告警阈值，单位V

#define PV_WARN_VOLT_LOW_THD          (50.0f) //电压过低告警阈值，单位V
#define PV_WARN_VOLT_OVER_THD         (120.0f) //电压过高告警阈值，单位V

#else
/* 正常工作模式参数 */

/**
 * @brief 通道最低工作电压(V)
 * @details 低于此值停止工作
 */
#define PV_CHANNEL_MIN_VOLTAGE       70.0f

/**
 * @brief 通道最高工作电压(V)
 * @details 超过此值限压保护
 */
#define PV_CHANNEL_MAX_VOLTAGE       450.0f

/**
 * @brief 通道最大工作电流(A)
 * @details 超过此值限流保护
 */
#define PV_CHANNEL_MAX_CURRENT       17.0f

/**
 * @brief 通道最大占空比(0~1)
 * @details PWM占空比上限
 */
#define PV_CHANNEL_MAX_DUTY          0.85f

/**
 * @brief 通道最小占空比(0~1)
 * @details PWM占空比下限，防止升压变换器完全关断
 */
#define PV_CHANNEL_MIN_DUTY          0.05f

/**
 * @brief 母线最大电压(V)
 * @details 母线电压上限
 */
#define PV_BUS_MAX_VOLTAGE           950.0f

/**
 * @brief 母线启动电压阈值(V)
 * @details 系统启动所需最低母线电压
 */
#define PV_BUS_START_UP_VOLTAGE      700.0f

/**
 * @brief 母线正常运行限制电压(V)
 * @details 正常运行时母线电压上限
 */
#define PV_BUS_NOMAL_LIMIT_VOLTAGE   950.0f

/**
 * @brief 单通道最大功率(W)
 * @details 单个PV通道最大输出功率
 */
#define PV_CHANNEL_MAX_POWER         8500.0f

/**
 * @brief 系统总功率限制(W)
 * @details 所有通道总功率上限
 */
#define PV_TOTAL_MAX_POWER           25000.0f


#define PV_ERR_CURR_OVER_THD          (18.0f) //过流告警阈值，单位A
#define PV_ERR_BUS_VOLT_OVER_THD      (900.0f) //母线过压告警阈值，单位V

#define PV_WARN_VOLT_LOW_THD          (150.0f) //电压过低告警阈值，单位V
#define PV_WARN_VOLT_OVER_THD         (950.0f) //电压过高告警阈值，单位V
#endif

/* ========================================================================
 * MPPT(最大功率点跟踪)控制参数
 * ======================================================================== */

/**
 * @brief MPPT电压参考值变化最大值(V/s)
 * @details 电压参考值每秒最大变化量
 */
#define PV_MPPT_D_VREF_MAX             (20.0f)
/**
 * @brief MPPT电压参考值变化比例系数
 * @details 调节电压参考值变化速度的增益
 */
#define PV_MPPT_D_VREF_K               (2.0f)

/**
 * @brief MPPT控制频率(Hz)
 * @details MPPT算法执行频率
 */
#define PV_MPPT_CNTL_FS              (5U)

/**
 * @brief MPPT控制周期(s)
 * @details 由控制频率计算得出
 */
#define PV_MPPT_CNTL_TS              (1.0f/PV_MPPT_CNTL_FS)

/**
 * @brief 电流限制范围(A)
 * @details 进入电流限制状态的容差范围,0-1
 */
#define PV_MPPT_CUR_LIMIT_ZONE       (0.1f)

/**
 * @brief 功率限制范围(W)
 * @details 进入功率限制状态的容差范围,0-1
 */
#define PV_MPPT_PWR_LIMIT_ZONE       (0.5f)

/**
 * @brief 功率标准差限制(W)
 * @details 用于判断功率稳定性
 * @note 暂未启用
 */
#define PV_MPPT_PWR_STD_LIMIT        (500.0f)

/**
 * @brief 是否启用分时控制 (0=禁用, 1=启用)
 * @details 多通道分时计算以降低负载，同时控制 MPPT 与内环（电流环/功率环）是否采用分时调度
 */
#define PV_CNTL_TIME_SLICE           (0)

/* ========================================================================
 * 控制环路参数
 * ======================================================================== */

/**
 * @brief 控制环执行频率(Hz)
 * @details 内环(电流环/功率环)的执行频率
 */
#define PV_LOOP_FS                  (5000U)

/**
 * @brief 控制环执行周期(s)
 * @details 由执行频率计算得出
 */
#define PV_LOOP_TS                  (1.0f/PV_LOOP_FS)

/**
 * @brief 功率调整限制(W)
 * @note 已弃用，保留用于参考
 */
#define PV_LOOP_PWR_ADJUST_LIMIT    (500.0f)

/**
 * @brief 功率调整比例系数
 * @details 功率调节速度调节参数
 */
#define PV_LOOP_PWR_ADJUST_K        (30.0f)//30

/**
 * @brief 多通道功率均衡比例系数
 * @details 用于平衡各通道功率输出，越大平衡力度越强
 */
#define PV_LOOP_PWR_BALANCE_K       (0.1f)

/**
 * @brief 功率调整阻尼系数
 * @details 抑制功率调节震荡
 * @note 暂未启用
 */
#define PV_LOOP_PWR_ADJUST_KD       (1.0f)

/* ========================================================================
 * 采样与平均计算参数
 * ======================================================================== */

/**
 * @brief 平均值计算采样点数
 * @details 在MPPT周期内采样次数，用于计算平均功率/电压/电流
 */
#define PV_AVG_SAMPLES              (PV_LOOP_FS / PV_MPPT_CNTL_FS)

#endif // PV_CFG_H
```

### `cmd/2837xD_FLASH_lnk_cpu1.cmd`

```text

MEMORY
{
PAGE 0 :  /* Program memory */
          /* RAM/FLASH blocks may also be allocated on PAGE 1 for data */
          /* BEGIN is used for "boot to Flash" bootloader mode */

   RAMLS0_4			: origin = 0x008000, length = 0x002800
 /*RAMLS0           : origin = 0x008000, length = 0x000800
   RAMLS1           : origin = 0x008800, length = 0x000800
   RAMLS2           : origin = 0x009000, length = 0x000800
   RAMLS3           : origin = 0x009800, length = 0x000800
   RAMLS4           : origin = 0x00A000, length = 0x000800*/
   RAMLS5			: origin = 0x00A800, length = 0x000800
   RAMD1           : origin = 0x00B800, length = 0x000800     /* D1: ramfunc RUN (PAGE 0) */

   RESET            : origin = 0x3FFFC0, length = 0x000002     /* Reset vector */
   IQTABLES         : origin = 0x3FE000, length = 0x000b50     /* IQ math lookup tables */
   IQTABLES2        : origin = 0x3FEB50, length = 0x00008c
   FPUTABLES        : origin = 0x3FEBDC, length = 0x0006A0     /* FPU math lookup tables */
   BOOTROM          : origin = 0x3FF27C, length = 0x000D44     /* Boot ROM */

   /* Flash sectors Flash0-FLASH4 reserved for boot */
   //FLASH 0          : origin = 0x080000, length = 0x002000  /* on-chip Flash */
   //FLASH 1          : origin = 0x082000, length = 0x002000  /* on-chip Flash */
   //FLASH 2          : origin = 0x084000, length = 0x002000  /* on-chip Flash */
   //FLASH 3          : origin = 0x086000, length = 0x002000  /* on-chip Flash */
   //FLASH 4          : origin = 0x088000, length = 0x008000  /* on-chip Flash */

//   BEGIN              : origin = 0x098000, length = 0x000002
   BEGIN           	  : origin = 0x080000, length = 0x000002       /* Code entry point */
   PROGRAM_SECTOR     : origin = 0x098010, length = 0x01FFF0  /* CPU application Flash */
   //FLASH 5          : origin = 0x090000, length = 0x008000  /* on-chip Flash */
   //FLASH 6          : origin = 0x098000, length = 0x008000  /* on-chip Flash */
   //FLASH 7          : origin = 0x0A0000, length = 0x008000  /* on-chip Flash */
   //FLASH 8          : origin = 0x0A8000, length = 0x008000  /* on-chip Flash */
   //FLASH 9          : origin = 0x0B0000, length = 0x008000  /* on-chip Flash */
   //FLASH 10         : origin = 0x0B8000, length = 0x002000  /* on-chip Flash */
   //FLASH 11         : origin = 0x0BA000, length = 0x002000  /* on-chip Flash */
   //FLASH 12         : origin = 0x0BC000, length = 0x002000  /* on-chip Flash */
   //FLASH 13         : origin = 0x0BE000, length = 0x002000  /* on-chip Flash */

PAGE 1 : /* Data memory */
         /* RAM/FLASH blocks may also be allocated on PAGE 0 for program code */

   BOOT_RSVD       : origin = 0x000002, length = 0x000120     /* M0 low region: Boot ROM stack */
   RAMM0           : origin = 0x000122, length = 0x0002DE     /* M0: .cio, Filter_RegsFile (PAGE 1) */
   RAMM1           : origin = 0x000400, length = 0x000400     /* M1: heap (.esysmem) */
   RAMD0           : origin = 0x00B000, length = 0x000800     /* D0: stack only (PAGE 1) */

//   RAMGS0_3    : origin = 0x00C000, length = 0x004000
   RAMGS0      : origin = 0x00C000, length = 0x001000
   RAMGS1      : origin = 0x00D000, length = 0x001000
   RAMGS2      : origin = 0x00E000, length = 0x001000
   RAMGS3      : origin = 0x00F000, length = 0x001000
   RAMGS4      : origin = 0x010000, length = 0x001000
   RAMGS5_15   : origin = 0x011000, length = 0x00B000   /* GS5-GS15, 11 x 4KB */
//   RAMGS5      : origin = 0x011000, length = 0x001000
//   RAMGS6      : origin = 0x012000, length = 0x001000
//   RAMGS7      : origin = 0x013000, length = 0x001000
//   RAMGS8      : origin = 0x014000, length = 0x001000
//   RAMGS9      : origin = 0x015000, length = 0x001000
//   RAMGS10     : origin = 0x016000, length = 0x001000
//   RAMGS11     : origin = 0x017000, length = 0x001000
//   RAMGS12     : origin = 0x018000, length = 0x001000
//   RAMGS13     : origin = 0x019000, length = 0x001000
//   RAMGS14     : origin = 0x01A000, length = 0x001000
//   RAMGS15     : origin = 0x01B000, length = 0x001000

   CPU2TOCPU1RAM   : origin = 0x03F800, length = 0x000400       /* CPU2 to CPU1 IPC message RAM */
   CPU1TOCPU2RAM   : origin = 0x03FC00, length = 0x000400

   CLA1_MSGRAMLOW   : origin = 0x001480,   length = 0x000080    /* CLA to CPU message RAM */
   CLA1_MSGRAMHIGH  : origin = 0x001500,   length = 0x000080    /* CPU to CLA message RAM */
}

SECTIONS
{
   /* Program sections */
   .cinit              : > PROGRAM_SECTOR      PAGE = 0, ALIGN(8)    /* C initialization tables */
   .pinit              : > PROGRAM_SECTOR,     PAGE = 0, ALIGN(8)    /* C++ constructor tables */
   .text               : >> PROGRAM_SECTOR      PAGE = 0, ALIGN(8)   /* Program code */
   codestart           : > BEGIN       PAGE = 0, ALIGN(8)            /* Code startup */

#ifdef __TI_COMPILER_VERSION__
    #if __TI_COMPILER_VERSION__ >= 15009000
    .TI.ramfunc : {} LOAD = PROGRAM_SECTOR,
                         RUN = RAMD1,                         /* Copy from Flash, run in RAM */
                         LOAD_START(_RamfuncsLoadStart),
                         LOAD_SIZE(_RamfuncsLoadSize),
                         LOAD_END(_RamfuncsLoadEnd),
                         RUN_START(_RamfuncsRunStart),
                         RUN_SIZE(_RamfuncsRunSize),
                         RUN_END(_RamfuncsRunEnd),
                         ALIGN(8)
    #else
   ramfuncs            : LOAD = PROGRAM_SECTOR,
                         RUN = RAMD1,
                         LOAD_START(_RamfuncsLoadStart),
                         LOAD_SIZE(_RamfuncsLoadSize),
                         LOAD_END(_RamfuncsLoadEnd),
                         RUN_START(_RamfuncsRunStart),
                         RUN_SIZE(_RamfuncsRunSize),
                         RUN_END(_RamfuncsRunEnd),
                         ALIGN(8)
    #endif
#endif

   /* Uninitialized data sections */
   .stack              : > RAMD0        PAGE = 1        /* Stack in D0 (2KB) */
   .ebss               : > RAMGS5_15	PAGE = 1		/* Global/static vars in GS5-GS15 */
   .cio				   : > RAMM0 	PAGE = 1        /* C I/O buffers */
   .esysmem            : > RAMM1        PAGE = 1        /* Heap in M1 (1KB) */

   /* Initialized constants in Flash (copied to RAM at startup if needed) */
   .econst             : >> PROGRAM_SECTOR     PAGE = 0, ALIGN(8)   /* const data */
   .switch             : > PROGRAM_SECTOR      PAGE = 0, ALIGN(8)   /* switch jump tables */

   .reset              : > RESET,     	PAGE = 0, TYPE = DSECT /* Placeholder only, not loaded */

   Filter_RegsFile     : > RAMM0,    PAGE = 1

/* Dual-core GS shared sections: GS3/GS4 are reserved for CPU1/CPU2 shared globals */
   SHARERAMGS0      : > RAMGS3,       PAGE = 1
   SHARERAMGS1      : > RAMGS4,       PAGE = 1
   ramgs0           : > RAMGS3,       PAGE = 1
   ramgs1           : > RAMGS4,       PAGE = 1

   /* Required for IPC API drivers */
    GROUP : > CPU1TOCPU2RAM, PAGE = 1
    {
        PUTBUFFER
        PUTWRITEIDX
        GETREADIDX
    }

    GROUP : > CPU2TOCPU1RAM, PAGE = 1
    {
        GETBUFFER :    TYPE = DSECT
        GETWRITEIDX :  TYPE = DSECT
        PUTREADIDX :   TYPE = DSECT
    }

   /* SDFM example sections (disabled) */
//   Filter1_RegsFile : > RAMGS15, PAGE = 1, fill=0x1111
//   Filter2_RegsFile : > RAMGS15, PAGE = 1, fill=0x2222
//   Filter3_RegsFile : > RAMGS15, PAGE = 1, fill=0x3333
//   Filter4_RegsFile : > RAMGS15, PAGE = 1, fill=0x4444
//   Difference_RegsFile : >RAMGS15,   PAGE = 1, fill=0x3333

//   SFRA_F_Data      : > RAMGS15, ALIGN = 64, PAGE = 1      /* SFRA float data */

//   SFRA_Data       : > RAMGS15, ALIGN = 64, PAGE = 1

   FPUmathTables   : > FPUTABLES, PAGE = 0, TYPE = NOLOAD   /* On-chip ROM, do not load */

   /* CLA sections */

   .scratchpad      : > RAMLS5,           PAGE = 0    /* CLA scratchpad */
   .bss_cla         : > RAMLS5,           PAGE = 0    /* CLA uninitialized data */

   Cla1DataRam      : > RAMLS5,           PAGE = 0    /* CLA data RAM */
   cla_shared       : > RAMLS5,           PAGE = 0    /* CPU/CLA shared data */
   CLADataLS1       : > RAMLS5,           PAGE = 0    /* CLA LS1 data */
   CLAFuncs			: > RAMLS5,			PAGE = 0      /* CLA functions */

   Cla1ToCpuMsgRAM  : > CLA1_MSGRAMLOW,   	PAGE = 1   /* CLA to CPU message RAM */
   CpuToCla1MsgRAM  : > CLA1_MSGRAMHIGH,  	PAGE = 1   /* CPU to CLA message RAM */


   .const_cla      :  LOAD = PROGRAM_SECTOR,
                      RUN = RAMLS5,
                      RUN_START(_Cla1ConstRunStart),
                      LOAD_START(_Cla1ConstLoadStart),
                      LOAD_SIZE(_Cla1ConstLoadSize)

	Cla1Prog       :  LOAD = PROGRAM_SECTOR,
                      RUN = RAMLS0_4,
                      LOAD_START(_Cla1funcsLoadStart),
                      LOAD_END(_Cla1funcsLoadEnd),
                      RUN_START(_Cla1funcsRunStart),
                      LOAD_SIZE(_Cla1funcsLoadSize)

   CLA1mathTables   : LOAD = PROGRAM_SECTOR,
                      RUN = RAMLS5,
                      RUN_START(_CLA1mathTablesRunStart),
                      LOAD_START(_CLA1mathTablesLoadStart),
                      LOAD_SIZE(_CLA1mathTablesLoadSize)
}
/*
//===========================================================================
// End of file.
//===========================================================================
*/
```

### `cmd/2837xD_RAM_lnk_cpu1.cmd`

```text

MEMORY
{
PAGE 0 :
   /* BEGIN is used for the "boot to SARAM" bootloader mode   */

   BEGIN            : origin = 0x000000, length = 0x000002
   RAMM0            : origin = 0x000123, length = 0x0002DD
   RAMD0            : origin = 0x00B000, length = 0x000800
   RAMLS0_4         : origin = 0x008000, length = 0x002800
//   RAMLS0           : origin = 0x008000, length = 0x000800
//   RAMLS1           : origin = 0x008800, length = 0x000800
//   RAMLS2           : origin = 0x009000, length = 0x000800
//   RAMLS3           : origin = 0x009800, length = 0x000800
//   RAMLS4           : origin = 0x00A000, length = 0x000800
   RESET            : origin = 0x3FFFC0, length = 0x000002

  /* Flash sectors */
   FLASHA           : origin = 0x080002, length = 0x001FFE  /* on-chip Flash */
   FLASHB           : origin = 0x082000, length = 0x002000  /* on-chip Flash */
   FLASHC           : origin = 0x084000, length = 0x002000  /* on-chip Flash */
   FLASHD           : origin = 0x086000, length = 0x002000  /* on-chip Flash */
   FLASHE           : origin = 0x088000, length = 0x008000  /* on-chip Flash */
   FLASHF           : origin = 0x090000, length = 0x008000  /* on-chip Flash */
   FLASHG           : origin = 0x098000, length = 0x008000  /* on-chip Flash */
   FLASHH           : origin = 0x0A0000, length = 0x008000  /* on-chip Flash */
   FLASHI           : origin = 0x0A8000, length = 0x008000  /* on-chip Flash */
   FLASHJ           : origin = 0x0B0000, length = 0x008000  /* on-chip Flash */
   FLASHK           : origin = 0x0B8000, length = 0x002000  /* on-chip Flash */
   FLASHL           : origin = 0x0BA000, length = 0x002000  /* on-chip Flash */
   FLASHM           : origin = 0x0BC000, length = 0x002000  /* on-chip Flash */
   FLASHN           : origin = 0x0BE000, length = 0x001FF0  /* on-chip Flash */

    RAMGS0_10   : origin = 0x00C000, length = 0x00B000
//   FLASHN_RSVD     : origin = 0x0BFFF0, length = 0x000010    /* Reserve and do not use for code as per the errata advisory "Memory: Prefetching Beyond Valid Memory" */

PAGE 1 :

   BOOT_RSVD       : origin = 0x000002, length = 0x000121     /* Part of M0, BOOT rom will use this for stack */
   RAMM1           : origin = 0x000400, length = 0x0003F8     /* on-chip RAM block M1 */
//   RAMM1_RSVD      : origin = 0x0007F8, length = 0x000008     /* Reserve and do not use for code as per the errata advisory "Memory: Prefetching Beyond Valid Memory" */
   RAMD1           : origin = 0x00B800, length = 0x000800

   RAMLS5      : origin = 0x00A800, length = 0x000800

//   RAMGS0_10   : origin = 0x00C000, length = 0x00B000
//   RAMGS0      : origin = 0x00C000, length = 0x001000
//   RAMGS1      : origin = 0x00D000, length = 0x001000
//   RAMGS2      : origin = 0x00E000, length = 0x001000
//   RAMGS3      : origin = 0x00F000, length = 0x001000
//   RAMGS4      : origin = 0x010000, length = 0x001000
//   RAMGS5      : origin = 0x011000, length = 0x001000
//   RAMGS6      : origin = 0x012000, length = 0x001000
//   RAMGS7      : origin = 0x013000, length = 0x001000
//   RAMGS8      : origin = 0x014000, length = 0x001000
//   RAMGS9      : origin = 0x015000, length = 0x001000
//   RAMGS10     : origin = 0x016000, length = 0x001000

//   RAMGS11     : origin = 0x017000, length = 0x000FF8   /* Uncomment for F28374D, F28376D devices */

//   RAMGS11_RSVD : origin = 0x017FF8, length = 0x000008    /* Reserve and do not use for code as per the errata advisory "Memory: Prefetching Beyond Valid Memory" */

   RAMGS11     : origin = 0x017000, length = 0x001000     /* Only Available on F28379D, F28377D, F28375D devices. Remove line on other devices. */
   RAMGS12     : origin = 0x018000, length = 0x001000     /* Only Available on F28379D, F28377D, F28375D devices. Remove line on other devices. */
   RAMGS13     : origin = 0x019000, length = 0x001000     /* Only Available on F28379D, F28377D, F28375D devices. Remove line on other devices. */
   RAMGS14     : origin = 0x01A000, length = 0x001000     /* Only Available on F28379D, F28377D, F28375D devices. Remove line on other devices. */
   RAMGS15     : origin = 0x01B000, length = 0x000FF8     /* Only Available on F28379D, F28377D, F28375D devices. Remove line on other devices. */

//   RAMGS15_RSVD : origin = 0x01BFF8, length = 0x000008    /* Reserve and do not use for code as per the errata advisory "Memory: Prefetching Beyond Valid Memory" */
                                                            /* Only on F28379D, F28377D, F28375D devices. Remove line on other devices. */

   CPU2TOCPU1RAM   : origin = 0x03F800, length = 0x000400
   CPU1TOCPU2RAM   : origin = 0x03FC00, length = 0x000400

   CANA_MSG_RAM     : origin = 0x049000, length = 0x000800
   CANB_MSG_RAM     : origin = 0x04B000, length = 0x000800
}


SECTIONS
{
   codestart        : > BEGIN,     PAGE = 0
   .text            : > RAMGS0_10,   PAGE = 0
   .cinit           : > RAMM0 | RAMLS0_4,     PAGE = 0
   .switch          : > RAMM0,     PAGE = 0
   .reset           : > RESET,     PAGE = 0, TYPE = DSECT /* not used, */
   .stack           : > RAMM1 | RAMD1,     PAGE = 1

#if defined(__TI_EABI__)
   .bss             : > RAMLS5,    PAGE = 1
   .bss:output      : > RAMLS3,    PAGE = 0
   .init_array      : > RAMM0,     PAGE = 0
   .const           : > RAMLS5,    PAGE = 1
   .data            : > RAMLS5,    PAGE = 1
   .sysmem          : > RAMLS5,    PAGE = 1
#else
   .pinit           : > RAMM0,     PAGE = 0
   .ebss            : >> RAMGS0_10,    PAGE = 0
   .econst          : > RAMGS0_10,    PAGE = 0
   .esysmem         : > RAMGS0_10,    PAGE = 0
#endif

   Filter_RegsFile  : > RAMGS0_10,    PAGE = 0


   ramgs0           : > RAMGS0_10,    PAGE = 0
   ramgs1           : > RAMGS0_10,    PAGE = 0

#ifdef __TI_COMPILER_VERSION__
   #if __TI_COMPILER_VERSION__ >= 15009000
    .TI.ramfunc : {} >  RAMM0 | RAMLS0_4,
                         LOAD_START(_RamfuncsLoadStart),
                         LOAD_SIZE(_RamfuncsLoadSize),
                         LOAD_END(_RamfuncsLoadEnd),
                         RUN_START(_RamfuncsRunStart),
                         RUN_SIZE(_RamfuncsRunSize),
                         RUN_END(_RamfuncsRunEnd), PAGE = 0
   #else
    ramfuncs    : > RAMM0      PAGE = 0
   #endif
#endif

   /* The following section definitions are required when using the IPC API Drivers */
    GROUP : > CPU1TOCPU2RAM, PAGE = 1
    {
        PUTBUFFER
        PUTWRITEIDX
        GETREADIDX
    }

    GROUP : > CPU2TOCPU1RAM, PAGE = 1
    {
        GETBUFFER :    TYPE = DSECT
        GETWRITEIDX :  TYPE = DSECT
        PUTREADIDX :   TYPE = DSECT
    }

    /* The following section definition are for SDFM examples */
   Filter1_RegsFile : > RAMGS0_10, PAGE = 0, fill=0x1111
   Filter2_RegsFile : > RAMGS0_10, PAGE = 0, fill=0x2222
   Filter3_RegsFile : > RAMGS0_10, PAGE = 0, fill=0x3333
   Filter4_RegsFile : > RAMGS0_10, PAGE = 0, fill=0x4444
   Difference_RegsFile : >RAMGS0_10,   PAGE = 0, fill=0x3333

   SFRA_F_Data      : > RAMGS0_10, ALIGN = 64, PAGE = 0

   SFRA_Data       : > RAMGS0_10, ALIGN = 64, PAGE = 0

   FPUmathTables   : > RAMGS0_10, PAGE = 0
}

/*
//===========================================================================
// End of file.
//===========================================================================
*/
```

### `cmd/F2837xD_Headers_nonBIOS_cpu1.cmd`

```text

MEMORY
{
 PAGE 0:    /* Program Memory */

 PAGE 1:    /* Data Memory */

   ADCA_RESULT   : origin = 0x000B00, length = 0x000020
   ADCB_RESULT   : origin = 0x000B20, length = 0x000020
   ADCC_RESULT   : origin = 0x000B40, length = 0x000020
   ADCD_RESULT   : origin = 0x000B60, length = 0x000020

   ADCA          : origin = 0x007400, length = 0x000080
   ADCB          : origin = 0x007480, length = 0x000080
   ADCC          : origin = 0x007500, length = 0x000080
   ADCD          : origin = 0x007580, length = 0x000080

   ANALOG_SUBSYS : origin = 0x05D180, length = 0x000080

   CANA          : origin = 0x048000, length = 0x000800
   CANB          : origin = 0x04A000, length = 0x000800

   CLA1          : origin = 0x001400, length = 0x000040     /* CLA registers */

   CLB_XBAR      : origin = 0x007A40, length = 0x000040

   CMPSS1        : origin = 0x005C80, length = 0x000020
   CMPSS2        : origin = 0x005CA0, length = 0x000020
   CMPSS3        : origin = 0x005CC0, length = 0x000020
   CMPSS4        : origin = 0x005CE0, length = 0x000020
   CMPSS5        : origin = 0x005D00, length = 0x000020
   CMPSS6        : origin = 0x005D20, length = 0x000020
   CMPSS7        : origin = 0x005D40, length = 0x000020
   CMPSS8        : origin = 0x005D60, length = 0x000020

   CPU_TIMER0    : origin = 0x000C00, length = 0x000008     /* CPU Timer0 registers */
   CPU_TIMER1    : origin = 0x000C08, length = 0x000008     /* CPU Timer1 registers */
   CPU_TIMER2    : origin = 0x000C10, length = 0x000008     /* CPU Timer2 registers */

   DACA          : origin = 0x005C00, length = 0x000010
   DACB          : origin = 0x005C10, length = 0x000010
   DACC          : origin = 0x005C20, length = 0x000010

   DMA          : origin = 0x001000, length = 0x000200
   DMACLASRCSEL : origin = 0x007980, length = 0x000040

   ECAP1        : origin = 0x005000, length = 0x000020     /* Enhanced Capture 1 registers */
   ECAP2        : origin = 0x005020, length = 0x000020     /* Enhanced Capture 2 registers */
   ECAP3        : origin = 0x005040, length = 0x000020     /* Enhanced Capture 3 registers */
   ECAP4        : origin = 0x005060, length = 0x000020     /* Enhanced Capture 4 registers */
   ECAP5        : origin = 0x005080, length = 0x000020     /* Enhanced Capture 5 registers */
   ECAP6        : origin = 0x0050A0, length = 0x000020     /* Enhanced Capture 6 registers */

   EMIF1        : origin = 0x047000, length = 0x000800
   EMIF2        : origin = 0x047800, length = 0x000800

   EQEP1        : origin = 0x005100, length = 0x000040     /* Enhanced QEP 1 registers */
   EQEP2        : origin = 0x005140, length = 0x000040     /* Enhanced QEP 2 registers */
   EQEP3        : origin = 0x005180, length = 0x000040     /* Enhanced QEP 3 registers */

   EPWM1        : origin = 0x004000, length = 0x000100     /* Enhanced PWM 1 registers */
   EPWM2        : origin = 0x004100, length = 0x000100     /* Enhanced PWM 2 registers */
   EPWM3        : origin = 0x004200, length = 0x000100     /* Enhanced PWM 3 registers */
   EPWM4        : origin = 0x004300, length = 0x000100     /* Enhanced PWM 4 registers */
   EPWM5        : origin = 0x004400, length = 0x000100     /* Enhanced PWM 5 registers */
   EPWM6        : origin = 0x004500, length = 0x000100     /* Enhanced PWM 6 registers */
   EPWM7        : origin = 0x004600, length = 0x000100     /* Enhanced PWM 7 registers */
   EPWM8        : origin = 0x004700, length = 0x000100     /* Enhanced PWM 8 registers */
   EPWM9        : origin = 0x004800, length = 0x000100     /* Enhanced PWM 9 registers */
   EPWM10       : origin = 0x004900, length = 0x000100     /* Enhanced PWM 10 registers */
   EPWM11       : origin = 0x004A00, length = 0x000100     /* Enhanced PWM 11 registers */
   EPWM12       : origin = 0x004B00, length = 0x000100     /* Enhanced PWM 12 registers */

   EPWM_XBAR    : origin = 0x007A00, length = 0x000040

   FLASH0_CTRL  : origin = 0x05F800, length = 0x000300
   FLASH0_ECC   : origin = 0x05FB00, length = 0x000040

   GPIOCTRL     : origin = 0x007C00, length = 0x000180     /* GPIO control registers */
   GPIODAT      : origin = 0x007F00, length = 0x000030     /* GPIO data registers */

   OUTPUT_XBAR  : origin = 0x007A80, length = 0x000040
   I2CA         : origin = 0x007300, length = 0x000040     /* I2C-A registers */
   I2CB         : origin = 0x007340, length = 0x000040     /* I2C-B registers */

   IPC          : origin = 0x050000, length = 0x000024

   FLASHPUMPSEMAPHORE   : origin = 0x050024, length = 0x000002

   ROMPREFETCH  : origin = 0x05E608, length = 0x000002

   MEMCFG       : origin = 0x05F400, length = 0x000080     /* Mem Config registers */
   EMIF1CONFIG  : origin = 0x05F480, length = 0x000020     /* Emif-1 Config registers */
   EMIF2CONFIG  : origin = 0x05F4A0, length = 0x000020     /* Emif-2 Config registers */
   ACCESSPROTECTION  : origin = 0x05F4C0, length = 0x000040     /* Access Protection registers */
   MEMORYERROR  : origin = 0x05F500, length = 0x000040     /* Access Protection registers */
   ROMWAITSTATE : origin = 0x05F540, length = 0x000002     /* ROM Config registers */


   MCBSPA       : origin = 0x006000, length = 0x000040     /* McBSP-A registers */
   MCBSPB       : origin = 0x006040, length = 0x000040     /* McBSP-A registers */

   NMIINTRUPT   : origin = 0x007060, length = 0x000010     /* NMI Watchdog Interrupt Registers */

   PIE_CTRL     : origin = 0x000CE0, length = 0x000020     /* PIE control registers */
   PIE_VECT     : origin = 0x000D00, length = 0x000200     /* PIE Vector Table */
   SCIA         : origin = 0x007200, length = 0x000010     /* SCI-A registers */
   SCIB         : origin = 0x007210, length = 0x000010     /* SCI-B registers */
   SCIC         : origin = 0x007220, length = 0x000010     /* SCI-C registers */
   SCID         : origin = 0x007230, length = 0x000010     /* SCI-D registers */

   SDFM1        : origin = 0x005E00, length = 0x000080     /* Sigma delta 1 registers */
   SDFM2        : origin = 0x005E80, length = 0x000080     /* Sigma delta 2 registers */

   SPIA         : origin = 0x006100, length = 0x000010
   SPIB         : origin = 0x006110, length = 0x000010
   SPIC         : origin = 0x006120, length = 0x000010
   SPID         : origin = 0x006130, length = 0x000010

   UPP          : origin = 0x006200, length = 0x000100     /* uPP registers */

   DEV_CFG     : origin = 0x05D000, length = 0x000180
   CLK_CFG     : origin = 0x05D200, length = 0x000100
   CPU_SYS     : origin = 0x05D300, length = 0x000100

   INPUT_XBAR   : origin = 0x007900, length = 0x000020
   XBAR         : origin = 0x007920, length = 0x000020
   SYNC_SOC     : origin = 0x007940, length = 0x000010
   WD           : origin = 0x007000, length = 0x000040

   XINT         : origin = 0x007070, length = 0x000010

   DCSM_Z1      : origin = 0x05F000, length = 0x000030     /* Zone 1 Dual code security module registers */
   DCSM_Z2      : origin = 0x05F040, length = 0x000030     /* Zone 2 Dual code security module registers */
   DCSM_COMMON  : origin = 0x05F070, length = 0x000010     /* Common Dual code security module registers */

   DCSM_Z1_OTP  : origin = 0x078000, length = 0x000020     /* Part of Z1 OTP.  LinkPointer/JTAG lock/ Boot Mode */
   DCSM_Z2_OTP  : origin = 0x078200, length = 0x000020     /* Part of Z2 OTP.  LinkPointer/JTAG lock */
}


SECTIONS
{
/*** PIE Vect Table and Boot ROM Variables Structures ***/
  UNION run = PIE_VECT, PAGE = 1
   {
      PieVectTableFile
      GROUP
      {
         EmuKeyVar
         EmuBModeVar
         FlashCallbackVar
         FlashScalingVar
      }
   }

   AdcaResultFile        : > ADCA_RESULT,  PAGE = 1
   AdcbResultFile        : > ADCB_RESULT,  PAGE = 1
   AdccResultFile        : > ADCC_RESULT,  PAGE = 1
   AdcdResultFile        : > ADCD_RESULT,  PAGE = 1

   AdcaRegsFile          : > ADCA,         PAGE = 1
   AdcbRegsFile          : > ADCB,         PAGE = 1
   AdccRegsFile          : > ADCC,         PAGE = 1
   AdcdRegsFile          : > ADCD,         PAGE = 1

   AnalogSubsysRegsFile  : > ANALOG_SUBSYS, PAGE = 1

   CanaRegsFile          : > CANA,         PAGE = 1
   CanbRegsFile          : > CANB,         PAGE = 1

   Cla1RegsFile          : > CLA1,         PAGE = 1
   Cla1SoftIntRegsFile   : > PIE_CTRL,     PAGE = 1, type=DSECT

   ClbXbarRegsFile       : > CLB_XBAR     PAGE = 1

   Cmpss1RegsFile        : > CMPSS1,      PAGE = 1
   Cmpss2RegsFile        : > CMPSS2,      PAGE = 1
   Cmpss3RegsFile        : > CMPSS3,      PAGE = 1
   Cmpss4RegsFile        : > CMPSS4,      PAGE = 1
   Cmpss5RegsFile        : > CMPSS5,      PAGE = 1
   Cmpss6RegsFile        : > CMPSS6,      PAGE = 1
   Cmpss7RegsFile        : > CMPSS7,      PAGE = 1
   Cmpss8RegsFile        : > CMPSS8,      PAGE = 1

   CpuTimer0RegsFile     : > CPU_TIMER0,    PAGE = 1
   CpuTimer1RegsFile     : > CPU_TIMER1,    PAGE = 1
   CpuTimer2RegsFile     : > CPU_TIMER2,    PAGE = 1

   DacaRegsFile          : > DACA          PAGE = 1
   DacbRegsFile          : > DACB          PAGE = 1
   DaccRegsFile          : > DACC          PAGE = 1

   DcsmZ1RegsFile        : > DCSM_Z1,          PAGE = 1
   DcsmZ2RegsFile        : > DCSM_Z2,          PAGE = 1
   DcsmCommonRegsFile    : > DCSM_COMMON,      PAGE = 1

   /*** Warning:  Only remove "Type = NOLOAD" to program OTP Locations ***/
   DcsmZ1OtpFile         : > DCSM_Z1_OTP,      PAGE = 1, type = NOLOAD
   DcsmZ2OtpFile         : > DCSM_Z2_OTP,      PAGE = 1, type = NOLOAD

   DmaRegsFile           : > DMA           PAGE = 1
   DmaClaSrcSelRegsFile  : > DMACLASRCSEL  PAGE = 1

   ECap1RegsFile         : > ECAP1,        PAGE = 1
   ECap2RegsFile         : > ECAP2,        PAGE = 1
   ECap3RegsFile         : > ECAP3,        PAGE = 1
   ECap4RegsFile         : > ECAP4,        PAGE = 1
   ECap5RegsFile         : > ECAP5,        PAGE = 1
   ECap6RegsFile         : > ECAP6,        PAGE = 1

   Emif1RegsFile         : > EMIF1         PAGE = 1
   Emif2RegsFile         : > EMIF2         PAGE = 1

   EPwm1RegsFile         : > EPWM1,        PAGE = 1
   EPwm2RegsFile         : > EPWM2,        PAGE = 1
   EPwm3RegsFile         : > EPWM3,        PAGE = 1
   EPwm4RegsFile         : > EPWM4,        PAGE = 1
   EPwm5RegsFile         : > EPWM5,        PAGE = 1
   EPwm6RegsFile         : > EPWM6,        PAGE = 1
   EPwm7RegsFile         : > EPWM7,        PAGE = 1
   EPwm8RegsFile         : > EPWM8,        PAGE = 1
   EPwm9RegsFile         : > EPWM9,        PAGE = 1
   EPwm10RegsFile        : > EPWM10,       PAGE = 1
   EPwm11RegsFile        : > EPWM11,       PAGE = 1
   EPwm12RegsFile        : > EPWM12,       PAGE = 1

   EPwmXbarRegsFile      : > EPWM_XBAR     PAGE = 1

   EQep1RegsFile         : > EQEP1,        PAGE = 1
   EQep2RegsFile         : > EQEP2,        PAGE = 1
   EQep3RegsFile         : > EQEP3,        PAGE = 1

   Flash0CtrlRegsFile     : > FLASH0_CTRL    PAGE = 1
   Flash0EccRegsFile      : > FLASH0_ECC     PAGE = 1

   GpioCtrlRegsFile      : > GPIOCTRL,     PAGE = 1
   GpioDataRegsFile      : > GPIODAT,      PAGE = 1

   OutputXbarRegsFile    : > OUTPUT_XBAR    PAGE = 1
   I2caRegsFile          : > I2CA,          PAGE = 1
   I2cbRegsFile          : > I2CB,          PAGE = 1
   InputXbarRegsFile     : > INPUT_XBAR     PAGE = 1
   XbarRegsFile          : > XBAR           PAGE = 1
   IpcRegsFile           : > IPC,           PAGE = 1

   FlashPumpSemaphoreRegsFile   : > FLASHPUMPSEMAPHORE,    PAGE = 1

   RomPrefetchRegsFile       : > ROMPREFETCH,       PAGE = 1
   MemCfgRegsFile            : > MEMCFG,            PAGE = 1
   Emif1ConfigRegsFile       : > EMIF1CONFIG,       PAGE = 1
   Emif2ConfigRegsFile       : > EMIF2CONFIG,       PAGE = 1
   AccessProtectionRegsFile  : > ACCESSPROTECTION,  PAGE = 1
   MemoryErrorRegsFile       : > MEMORYERROR,       PAGE = 1
   RomWaitStateRegsFile      : > ROMWAITSTATE,      PAGE = 1

   McbspaRegsFile        : > MCBSPA,       PAGE = 1
   McbspbRegsFile        : > MCBSPB,       PAGE = 1

   UppRegsFile           : > UPP,          PAGE = 1

   NmiIntruptRegsFile    : > NMIINTRUPT,   PAGE = 1
   PieCtrlRegsFile       : > PIE_CTRL,     PAGE = 1

   SciaRegsFile          : > SCIA,         PAGE = 1
   ScibRegsFile          : > SCIB,         PAGE = 1
   ScicRegsFile          : > SCIC,         PAGE = 1
   ScidRegsFile          : > SCID,         PAGE = 1

   Sdfm1RegsFile         : > SDFM1,        PAGE = 1
   Sdfm2RegsFile         : > SDFM2,        PAGE = 1

   SpiaRegsFile          : > SPIA,        PAGE = 1
   SpibRegsFile          : > SPIB,        PAGE = 1
   SpicRegsFile          : > SPIC,        PAGE = 1
   SpidRegsFile          : > SPID,        PAGE = 1

   DevCfgRegsFile        : > DEV_CFG,     PAGE = 1
   ClkCfgRegsFile        : > CLK_CFG,     PAGE = 1
   CpuSysRegsFile        : > CPU_SYS,     PAGE = 1

   SyncSocRegsFile       : > SYNC_SOC,    PAGE = 1

   WdRegsFile            : > WD,           PAGE = 1

   XintRegsFile          : > XINT          PAGE = 1
   MemCfgRegs            : > MEMCFG        PAGE = 1

}

/*
//===========================================================================
// End of file.
//===========================================================================
*/
```

