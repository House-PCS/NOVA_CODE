/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        fault_analysis.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-6-1
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2025-6-1    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "fault_analysis.h"
// 日志标签：用于打印该模块相关日志，便于调试定位
#define LOG_TAG     "fault_analysis"
/****************************************** 故障阈值宏定义 ******************************************/
// 放电过流阈值（1/2/3级，对应不同严重程度），关联BMS警告参数结构体
#define DISCHARGE_OVERCURR1     BMSWarningParam.discharge_overcurr1
#define DISCHARGE_OVERCURR2     BMSWarningParam.discharge_overcurr2
#define DISCHARGE_OVERCURR3     BMSWarningParam.discharge_overcurr3

// 充电过流阈值（1/2/3级），关联BMS警告参数结构体
#define CHARGE_OVERCURR1        BMSWarningParam.charge_overcurr1
#define CHARGE_OVERCURR2        BMSWarningParam.charge_overcurr2
#define CHARGE_OVERCURR3        BMSWarningParam.charge_overcurr3

// 电芯欠压阈值（1/2/3级），关联BMS警告参数结构体
#define CELL_UV1                BMSWarningParam.cell_uv1
#define CELL_UV2                BMSWarningParam.cell_uv2
#define CELL_UV3                BMSWarningParam.cell_uv3

// 电芯过压阈值（1/2/3级），关联BMS警告参数结构体
#define CELL_OV1                BMSWarningParam.cell_ov1
#define CELL_OV2                BMSWarningParam.cell_ov2
#define CELL_OV3                BMSWarningParam.cell_ov3

// 电池包总过压/欠压阈值，关联BMS警告参数结构体
#define PACK_OV                 BMSWarningParam.pack_ov
#define PACK_UV                 BMSWarningParam.pack_uv

// MOS管过温阈值（1/2/3级），关联BMS警告参数结构体
#define CMOS_OT1                BMSWarningParam.cmos_ot1
#define CMOS_OT2                BMSWarningParam.cmos_ot2
#define CMOS_OT3                BMSWarningParam.cmos_ot3

// 电芯过温阈值（1/2/3级）、电芯欠温阈值，关联BMS警告参数结构体
#define CELL_OT1                BMSWarningParam.cell_ot1
#define CELL_OT2                BMSWarningParam.cell_ot2
#define CELL_OT3                BMSWarningParam.cell_ot3

#define CELL_UT1                BMSWarningParam.cell_ut1
#define CELL_UT2                BMSWarningParam.cell_ut2
#define CELL_UT3                BMSWarningParam.cell_ut3

// 电芯温差阈值、电芯电压差阈值，关联BMS警告参数结构体
#define CELL_T_DIFF             BMSWarningParam.cell_temp_diff
#define CELL_V_DIFF             BMSWarningParam.cell_voltage_diff

// SOH（电池健康度）过低阈值（1/2/3级），关联BMS警告参数结构体
#define SOH_UNDER_LVL1          BMSWarningParam.soh_under_lvl1
#define SOH_UNDER_LVL2          BMSWarningParam.soh_under_lvl2
#define SOH_UNDER_LVL3          BMSWarningParam.soh_under_lvl3

// 温度传感器异常阈值（欠温/过温），关联BMS警告参数结构体
#define TSENSOR_UT              BMSWarningParam.tsensor_ut
#define TSENSOR_OT              BMSWarningParam.tsensor_ot
#define ZERODRIFT_CURRENT       BMSWarningParam.zerodrift_current    // 零漂电流阈值

/****************************************** 全局变量定义 ******************************************/
Fault_analysis_flag     gFaultAnalysisFlag;    // 故障分析标志结构体：存储各类故障的计数/状态标志
BMSErrorInfo_t          gBmsErrorInfo;         // BMS故障信息结构体：记录BMS层面的故障码
PackErrorInfo_t         gPackErrorInfo;        // 电池包故障信息结构体：记录电池包层面的故障码
HistoryErrorStatus_t    gHistoryErrorStatus;   // 历史故障状态结构体：存储故障历史记录
BMSWarningParam_t       BMSWarningParam;       // BMS警告参数结构体：存储各类故障判断的阈值参数
BMSErrorNumCode_t       gBMSErrorNumCode;      // BMS故障编号结构体：为每个故障分配唯一编号，便于定位

/***************************************************************************************************
 * 函数名称：_BMSWarningParamUpdate
 * 函数功能：更新BMS警告参数（故障判断阈值）
 * 函数说明：
 * 1. 根据SOX调试模式选择读取的参数源（调试模式/正常模式）
 * 2. 将BMS寄存器中的阈值参数赋值给BMSWarningParam结构体
 * 3. 电池包总电压（PACK_OV/PACK_UV）通过高低位拼接得到32位数值
 * 4. 温度传感器异常阈值、零漂电流为固定值（-40℃/120℃，2000mA）
 **************************************************************************************************/
static void _BMSWarningParamUpdate(void)
{
    // 若处于SOX调试模式，读取调试模式下的参数
    if(gSoxDebugParam.sox_debug_mode)
    {
        // 放电过流阈值（1/2/3级）：从BMS只读寄存器读取
        BMSWarningParam.discharge_overcurr1     = garrBMSReg.bms_only_read_para.dsg_oc_lvl1;
        BMSWarningParam.discharge_overcurr2     = garrBMSReg.bms_only_read_para.dsg_oc_lvl2;
        BMSWarningParam.discharge_overcurr3     = garrBMSReg.bms_only_read_para.dsg_oc_lvl3;

        // 充电过流阈值（1/2/3级）：从BMS只读寄存器读取
        BMSWarningParam.charge_overcurr1        = garrBMSReg.bms_only_read_para.chg_oc_lvl1;
        BMSWarningParam.charge_overcurr2        = garrBMSReg.bms_only_read_para.chg_oc_lvl2;
        BMSWarningParam.charge_overcurr3        = garrBMSReg.bms_only_read_para.chg_oc_lvl3;

        // 电芯欠压阈值（1/2/3级）：从BMS只读寄存器读取
        BMSWarningParam.cell_uv1                = garrBMSReg.bms_only_read_para.cell_uv_lvl1;
        BMSWarningParam.cell_uv2                = garrBMSReg.bms_only_read_para.cell_uv_lvl2;
        BMSWarningParam.cell_uv3                = garrBMSReg.bms_only_read_para.cell_uv_lvl3;

        // 电芯过压阈值（1/2/3级）：从BMS只读寄存器读取
        BMSWarningParam.cell_ov1                = garrBMSReg.bms_only_read_para.cell_ov_lvl1;
        BMSWarningParam.cell_ov2                = garrBMSReg.bms_only_read_para.cell_ov_lvl2;
        BMSWarningParam.cell_ov3                = garrBMSReg.bms_only_read_para.cell_ov_lvl3;

        // 电池包总过压阈值：高低位拼接（16位+16位=32位）
        BMSWarningParam.pack_ov                 = ((uint32_t)garrBMSReg.bms_only_read_para.pack_ov_lvl_high << 16) + garrBMSReg.bms_only_read_para.pack_ov_lvl_low;
        // 电池包总欠压阈值：高低位拼接（16位+16位=32位）
        BMSWarningParam.pack_uv                 = ((uint32_t)garrBMSReg.bms_only_read_para.pack_uv_lvl_high << 16) + garrBMSReg.bms_only_read_para.pack_uv_lvl_low;

        // MOS管过温阈值（1/2/3级）：从BMS只读寄存器读取
        BMSWarningParam.cmos_ot1                = garrBMSReg.bms_only_read_para.mos_ot_lvl1;
        BMSWarningParam.cmos_ot2                = garrBMSReg.bms_only_read_para.mos_ot_lvl2;
        BMSWarningParam.cmos_ot3                = garrBMSReg.bms_only_read_para.mos_ot_lvl3;

        // 电芯过温阈值（1/2/3级）：从BMS只读寄存器读取
        BMSWarningParam.cell_ot1                = garrBMSReg.bms_only_read_para.cell_ot_lvl1;
        BMSWarningParam.cell_ot2                = garrBMSReg.bms_only_read_para.cell_ot_lvl2;
        BMSWarningParam.cell_ot3                = garrBMSReg.bms_only_read_para.cell_ot_lvl3;

        // 电芯欠温阈值：从BMS只读寄存器读取（转换为16位有符号数）
        BMSWarningParam.cell_ut1                 = (int16_t)garrBMSReg.bms_only_read_para.cell_ut_lvl;
        BMSWarningParam.cell_ut2                 = (int16_t)garrBMSReg.bms_only_read_para.cell_ut_lv2;
        BMSWarningParam.cell_ut3                 = (int16_t)garrBMSReg.bms_only_read_para.cell_ut_lv3;        
        // 电芯温差、电压差阈值：从BMS只读寄存器读取
        BMSWarningParam.cell_temp_diff          = garrBMSReg.bms_only_read_para.cell_temp_diff;
        BMSWarningParam.cell_voltage_diff       = garrBMSReg.bms_only_read_para.cell_voltage_diff;

        // SOH过低阈值（1/2/3级）：从BMS只读寄存器读取
        BMSWarningParam.soh_under_lvl1          = garrBMSReg.bms_only_read_para.soh_under_lvl1;
        BMSWarningParam.soh_under_lvl2          = garrBMSReg.bms_only_read_para.soh_under_lvl2;
        BMSWarningParam.soh_under_lvl3          = garrBMSReg.bms_only_read_para.soh_under_lvl3;

        // 温度传感器异常阈值：固定值（-400 = -40℃，1200 = 120℃，单位：0.1℃）
        BMSWarningParam.tsensor_ut              = -400;
        BMSWarningParam.tsensor_ot              = 1200;

        // 零漂电流阈值：固定值2000mA
        BMSWarningParam.zerodrift_current       = 2000;
    }
    // 正常模式：参数读取逻辑与调试模式一致（寄存器源相同）
    else
    {
        BMSWarningParam.discharge_overcurr1     = garrBMSReg.bms_only_read_para.dsg_oc_lvl1;
        BMSWarningParam.discharge_overcurr2     = garrBMSReg.bms_only_read_para.dsg_oc_lvl2;
        BMSWarningParam.discharge_overcurr3     = garrBMSReg.bms_only_read_para.dsg_oc_lvl3;
        BMSWarningParam.charge_overcurr1        = garrBMSReg.bms_only_read_para.chg_oc_lvl1;
        BMSWarningParam.charge_overcurr2        = garrBMSReg.bms_only_read_para.chg_oc_lvl2;
        BMSWarningParam.charge_overcurr3        = garrBMSReg.bms_only_read_para.chg_oc_lvl3;
        BMSWarningParam.cell_uv1                = garrBMSReg.bms_only_read_para.cell_uv_lvl1;
        BMSWarningParam.cell_uv2                = garrBMSReg.bms_only_read_para.cell_uv_lvl2;
        BMSWarningParam.cell_uv3                = garrBMSReg.bms_only_read_para.cell_uv_lvl3;
        BMSWarningParam.cell_ov1                = garrBMSReg.bms_only_read_para.cell_ov_lvl1;
        BMSWarningParam.cell_ov2                = garrBMSReg.bms_only_read_para.cell_ov_lvl2;
        BMSWarningParam.cell_ov3                = garrBMSReg.bms_only_read_para.cell_ov_lvl3;
        BMSWarningParam.pack_ov                 = ((uint32_t)garrBMSReg.bms_only_read_para.pack_ov_lvl_high << 16) + garrBMSReg.bms_only_read_para.pack_ov_lvl_low;
        BMSWarningParam.pack_uv                 = ((uint32_t)garrBMSReg.bms_only_read_para.pack_uv_lvl_high << 16) + garrBMSReg.bms_only_read_para.pack_uv_lvl_low;
        BMSWarningParam.cmos_ot1                = garrBMSReg.bms_only_read_para.mos_ot_lvl1;
        BMSWarningParam.cmos_ot2                = garrBMSReg.bms_only_read_para.mos_ot_lvl2;
        BMSWarningParam.cmos_ot3                = garrBMSReg.bms_only_read_para.mos_ot_lvl3;
        BMSWarningParam.cell_ot1                = garrBMSReg.bms_only_read_para.cell_ot_lvl1;
        BMSWarningParam.cell_ot2                = garrBMSReg.bms_only_read_para.cell_ot_lvl2;
        BMSWarningParam.cell_ot3                = garrBMSReg.bms_only_read_para.cell_ot_lvl3;
        BMSWarningParam.cell_ut1                = (int16_t)garrBMSReg.bms_only_read_para.cell_ut_lvl;
        BMSWarningParam.cell_ut2                = (int16_t)garrBMSReg.bms_only_read_para.cell_ut_lv2;
        BMSWarningParam.cell_ut3                = (int16_t)garrBMSReg.bms_only_read_para.cell_ut_lv3; 
        BMSWarningParam.cell_temp_diff          = garrBMSReg.bms_only_read_para.cell_temp_diff;
        BMSWarningParam.cell_voltage_diff       = garrBMSReg.bms_only_read_para.cell_voltage_diff;
        BMSWarningParam.soh_under_lvl1          = garrBMSReg.bms_only_read_para.soh_under_lvl1;
        BMSWarningParam.soh_under_lvl2          = garrBMSReg.bms_only_read_para.soh_under_lvl2;
        BMSWarningParam.soh_under_lvl3          = garrBMSReg.bms_only_read_para.soh_under_lvl3;
        BMSWarningParam.tsensor_ut              = -400;
        BMSWarningParam.tsensor_ot              = 1200;
        BMSWarningParam.zerodrift_current       = 2000;
    }
}

/***************************************************************************************************
 * 函数名称：FaultAnalysisProcess
 * 函数功能：BMS核心故障分析主流程
 * 函数说明：
 * 1. 周期检测各类故障（电流、电压、温度、SOH、传感器、MOS管等）
 * 2. 采用"阈值判断+计数防抖"机制，避免误报（不同故障防抖计数不同）
 * 3. 故障分级：通知级（notice）、报警级（alarm）、错误级（error），对应不同处理策略
 * 4. 严重故障触发保护动作（如关闭充放电MOS管）
 **************************************************************************************************/
void FaultAnalysisProcess(void)
{
    /****************************************** 局部变量定义 ******************************************/
    // 各类故障检测的时间间隔计数器（单位：ms），控制检测周期
    static uint32_t current_time = 0;          // 电流故障检测计时
    static uint32_t voltage_time = 0;          // 电压故障检测计时
    static uint32_t temperature_time = 0;      // 温度故障检测计时
    static uint32_t cell_voltage_diff_time = 0;// 电芯电压差故障检测计时
    static uint32_t soh_under_time = 0;        // SOH过低故障检测计时
    static uint32_t vsensor_error_time = 0;    // 电压传感器故障检测计时
    static uint32_t tsensor_error_time = 0;    // 温度传感器故障检测计时
    static uint32_t mos_error_time = 0;     // MOS粘连故障检测计时
    static uint32_t heat_error_time = 0;       // 加热膜故障检测计时

    int32_t pack_current = 0;                  // 电池包电流（取绝对值，统一判断过流）
    PackStatus_t *pPackStatus = NULL;          // 电池包状态指针（指向调试/正常模式状态）

    /****************************************** 初始化处理 ******************************************/
    // 根据SOX调试模式选择电池包状态数据源
    if(gSoxDebugParam.sox_debug_mode)
    {
        pPackStatus = &gPackStatus_debug;      // 调试模式：使用调试状态结构体
    }
    else
    {
        pPackStatus = &gPackStatus;            // 正常模式：使用正常状态结构体
    }

    // 电池包电流取绝对值（放电为正，充电为负，统一转为正数值判断过流）
    if(pPackStatus->pack_current < 0)
    {
        pack_current = -pPackStatus->pack_current;
    }
    else
    {
        pack_current = pPackStatus->pack_current;
    }

    // 更新BMS故障判断阈值参数
    _BMSWarningParamUpdate();

    // 更新温度数据（读取传感器最新温度）
    TemperatureUpdate();
    gstCellInfo.cmos_temperature = 250; // MOS管温度初始化（250 = 25℃，单位：0.1℃）
    // AFE（模拟前端）未完成初始化（首次覆盖标志<2），直接返回，不进行故障检测
    if(gFirst_AFE_Coved_Flag != 3)
    {
        return;
    }

    /****************************************** MOS粘连故障检测 **************************************************/
    /****************************************** 充电MOS无法闭合故障检测 ******************************************/
    // 检测周期：1000ms（时间差超过1000ms执行一次检测）
    if(TimeDiffFromNow(mos_error_time) > 1000)
    {
        mos_error_time = TimGetSystime();    // 更新当前检测时间
        // 故障条件：充放电MOS都未打开，但存在放电电流>10000mA；或充电MOS未打开，但存在充电电流<-10000mA 放电或预充或充电MOS粘连故障（mos粘连故障）
        if(((!gFaultAnalysisFlag.dischargemos_enable_flag) && gPackStatus.pack_current > 10000) || \
                (!gFaultAnalysisFlag.chargemos_enable_flag && gPackStatus.pack_current < -10000))
        {
            gFaultAnalysisFlag.mos_adhesion_count++; // 故障计数+1（防抖）
        }
        else
        {
            gFaultAnalysisFlag.mos_adhesion_count = 0; // 无故障，计数清零
        }

        // 防抖计数超过3次（累计1000ms*3=3s），判定为MOS粘连故障
        if(gFaultAnalysisFlag.mos_adhesion_count > 10)
        {
            gBmsErrorInfo.mos_error_code.b.mos_adhesion = 1;
            gPackErrorInfo.mos_error_code.b.mos_adhesion = 1;
        }

        if(gFaultAnalysisFlag.chargemos_enable_flag)
        {
            //电芯总电压 - 电池包电压 > 100mV（MOS未闭合导致电压差）
            if(gstCellInfo.total_cell_voltage - PackVotageGet() > 10)
            {
                gFaultAnalysisFlag.mos_fail_close_count ++; // 故障计数+1
            }
            else
            {
                gFaultAnalysisFlag.mos_fail_close_count = 0; // 计数清零
            }
            // 防抖计数超过3次（累计3s），判定为充电MOS无法闭合故障
            if(gFaultAnalysisFlag.mos_fail_close_count > 10)
            {
                gBmsErrorInfo.mos_error_code.b.mos_fail_close = 0;
                gPackErrorInfo.mos_error_code.b.mos_fail_close = 0;
            }
        }
    }

    /****************************************** 预充电流反向、预充时间过长、预充短路（预充失败）(暂未涉及) ******************************************/
    /****************************************** 充电短路(暂未涉及)  ******************************************/
    /****************************************** 放电短路(暂未涉及)  ******************************************/


    /****************************************** 电流故障检测（充放电过流） ******************************************/
    if(TimeDiffFromNow(current_time) > 1000)  // 判断距离上次检测是否超过1秒（满足检测周期）
    {
        current_time = TimGetSystime();  // 更新当前检测时间戳，用于下次周期判断
        // --------------- 放电模式下过流检测（电池向外输出电流）---------------
        if(pPackStatus->mode == EN_PACK_CURRENT_DISCHARGE_MODE)
        {
            /********** 放电过流1级（通知级）**********/
            // 故障判定条件：
            // 1. 当前放电电流 > 阈值1 × 最大放电电流（阈值1为百分比，需×0.01转换）
            // 2. 电池包最大放电电流≥2000mA（避免小电流场景下误触发）
            if(pack_current > (DISCHARGE_OVERCURR1 * 0.01 * garrBMSReg.bms_only_read_para.pack_max_dsg_current * 100) &&
                    abs(garrBMSReg.bms_only_read_para.cell_current) >= 1880)
            {
                gFaultAnalysisFlag.dsg_over_current1_count ++;  // 满足条件，防抖计数+1
            }
            else
            {
                gFaultAnalysisFlag.dsg_over_current1_count = 0;  // 不满足条件，计数清零（故障恢复）
            }
            // 防抖逻辑：连续5次检测（5秒）满足故障条件，才判定为有效故障
            if(gFaultAnalysisFlag.dsg_over_current1_count > 60)
            {
                gBmsErrorInfo.curr_error_code.b.discharge_overcurr1 = 1;  // BMS内部标记故障
                gPackErrorInfo.curr_alarm_code.b.discharge_overcurr1 = 1;  // 对外输出「通知级」故障
                gBMSErrorNumCode.discharge_overcurr1_error_num = 30;  // 故障编号：30（通知级）
            }
            else
            {
                if((gBmsErrorInfo.curr_error_code.b.discharge_overcurr1 == 1) && (abs(garrBMSReg.bms_only_read_para.cell_current) < 1380))
                {
                    // 故障未满足/已恢复：清除所有相关标记
                    gBmsErrorInfo.curr_error_code.b.discharge_overcurr1 = 0;
                    gPackErrorInfo.curr_alarm_code.b.discharge_overcurr1 = 0;
                    gBMSErrorNumCode.discharge_overcurr1_error_num = 0;
                }
            }

            /********** 放电过流2级（报警级）**********/
            // 故障判定条件：
            // 1. 当前放电电流 > 阈值2 × 最大放电电流（阈值2 > 阈值1，过流更严重）
            // 2. 电池包最大放电电流≥2000mA（同1级条件，保证检测一致性）
            if(pack_current > (DISCHARGE_OVERCURR2 * 0.01 * garrBMSReg.bms_only_read_para.pack_max_dsg_current * 100) &&
                    abs(garrBMSReg.bms_only_read_para.cell_current) >= 1880)

            {
                gFaultAnalysisFlag.dsg_over_current2_count ++;  // 满足条件，防抖计数+1
            }
            else
            {
                gFaultAnalysisFlag.dsg_over_current2_count = 0;  // 不满足条件，计数清零
            }
            // 防抖逻辑：连续3次检测（3秒）满足故障条件（防抖时间短于1级，响应更灵敏）
            if(gFaultAnalysisFlag.dsg_over_current2_count > 300)
            {
                gBmsErrorInfo.curr_error_code.b.discharge_overcurr2 = 1;  // BMS内部标记故障
                gPackErrorInfo.curr_error_code.b.discharge_overcurr2 = 1;  // 对外输出「报警级」故障（需用户关注）
                gBMSErrorNumCode.discharge_overcurr2_error_num = 31;  // 故障编号：31（报警级）
            }

            /********** 放电过流3级（错误级）**********/
            // 故障判定条件：
            // 1. 当前放电电流 > 阈值3 × 最大放电电流（阈值3 > 阈值2，过流最严重）
            // 2. 电池包最大放电电流≥2000mA（同前两级条件）
            if(pack_current > (DISCHARGE_OVERCURR3 * 0.01 * garrBMSReg.bms_only_read_para.pack_max_dsg_current * 100) &&
                    abs(garrBMSReg.bms_only_read_para.cell_current) >= 1880)

            {
                gFaultAnalysisFlag.dsg_over_current3_count ++;  // 满足条件，防抖计数+1
            }
            else
            {
                gFaultAnalysisFlag.dsg_over_current3_count = 0;  // 不满足条件，计数清零
            }
            // 防抖逻辑：连续2次检测（2秒）满足故障条件（防抖时间最短，紧急响应）
            if(gFaultAnalysisFlag.dsg_over_current3_count > 300)
            {
                gBmsErrorInfo.curr_error_code.b.discharge_overcurr3 = 1;  // BMS内部标记故障
                gPackErrorInfo.curr_error_code.b.discharge_overcurr3 = 1;  // 对外输出「错误级」故障（需紧急处理）
                gBMSErrorNumCode.discharge_overcurr3_error_num = 32;  // 故障编号：32（错误级）
            }
        }
        // --------------- 充电模式下过流检测（外部向电池输入电流）---------------
        else if(pPackStatus->mode == EN_PACK_CURRENT_CHARGE_MODE)
        {
            /********** 充电过流1级（通知级）**********/
            // 故障判定条件：
            // 1. 当前充电电流 > 阈值1 × 最大充电电流（阈值1为百分比，×0.01转换）
            // 2. 电池包最大充电电流≥1800mA（充电最小电流阈值与放电不同，适配充电场景）
            if(pack_current > (CHARGE_OVERCURR1 * 0.01 * garrBMSReg.bms_only_read_para.pack_max_chg_current * 100) &&
                    abs(garrBMSReg.bms_only_read_para.cell_current) >= 1570)
            {
                gFaultAnalysisFlag.chg_over_current1_count ++;  // 满足条件，防抖计数+1
            }
            else
            {
                gFaultAnalysisFlag.chg_over_current1_count = 0;  // 不满足条件，计数清零
            }
            // 防抖逻辑：连续5次检测（5秒）满足条件，判定为有效故障
            if(gFaultAnalysisFlag.chg_over_current1_count > 60)
            {
                gBmsErrorInfo.curr_error_code.b.charge_overcurr1 = 1;  // BMS内部标记故障
                gPackErrorInfo.curr_alarm_code.b.charge_overcurr1 = 1;  // 对外输出「通知级」故障
                gBMSErrorNumCode.charge_overcurr1_error_num = 33;  // 故障编号：33（通知级）
            }
            else
            {
                if((gBmsErrorInfo.curr_error_code.b.charge_overcurr1 == 1) && (abs(garrBMSReg.bms_only_read_para.cell_current) < 1070))
                {
                    // 故障未满足/已恢复：清除所有相关标记
                    gBmsErrorInfo.curr_error_code.b.charge_overcurr1 = 0;
                    gPackErrorInfo.curr_alarm_code.b.charge_overcurr1 = 0;
                    gBMSErrorNumCode.charge_overcurr1_error_num = 0;
                }
            }

            /********** 充电过流2级（报警级）**********/
            // 故障判定条件：
            // 1. 当前充电电流 > 阈值2 × 最大充电电流（阈值2 > 阈值1）
            // 2. 电池包最大充电电流≥1800mA（同1级充电电流阈值条件）
            if(pack_current > (CHARGE_OVERCURR2 * 0.01 * garrBMSReg.bms_only_read_para.pack_max_chg_current * 100) &&
                    abs(garrBMSReg.bms_only_read_para.cell_current) >= 1570)
            {
                gFaultAnalysisFlag.chg_over_current2_count ++;  // 满足条件，防抖计数+1
            }
            else
            {
                gFaultAnalysisFlag.chg_over_current2_count = 0;  // 不满足条件，计数清零
            }
            // 防抖逻辑：连续3次检测（3秒）满足条件（响应优先级高于通知级）
            if(gFaultAnalysisFlag.chg_over_current2_count > 300)
            {
                gBmsErrorInfo.curr_error_code.b.charge_overcurr2 = 1;  // BMS内部标记故障
                gPackErrorInfo.curr_error_code.b.charge_overcurr2 = 1;  // 对外输出「报警级」故障
                gBMSErrorNumCode.charge_overcurr2_error_num = 34;  // 故障编号：34（报警级）
            }

            /********** 充电过流3级（错误级）**********/
            // 故障判定条件：
            // 1. 当前充电电流 > 阈值3 × 最大充电电流（阈值3 > 阈值2，过流最严重）
            // 2. 电池包最大充电电流≥1800mA（同前两级充电电流阈值条件）
            if(pack_current > (CHARGE_OVERCURR3 * 0.01 * garrBMSReg.bms_only_read_para.pack_max_chg_current * 100) &&
                    abs(garrBMSReg.bms_only_read_para.cell_current) >= 1570)
            {
                gFaultAnalysisFlag.chg_over_current3_count ++;  // 满足条件，防抖计数+1
            }
            else
            {
                gFaultAnalysisFlag.chg_over_current3_count = 0;  // 不满足条件，计数清零
            }
            // 防抖逻辑：连续2次检测（2秒）满足条件（紧急响应，避免电池损坏）
            if(gFaultAnalysisFlag.chg_over_current3_count > 300)
            {
                gBmsErrorInfo.curr_error_code.b.charge_overcurr3 = 1;  // BMS内部标记故障
                gPackErrorInfo.curr_error_code.b.charge_overcurr3 = 1;  // 对外输出「错误级」故障
                gBMSErrorNumCode.charge_overcurr3_error_num = 35;  // 故障编号：35（错误级）
            }
        }
    }

    /****************************************** 电压故障检测（电芯/电池包过压/欠压） ******************************************/
// 检测周期：1000ms（1s）
    if(TimeDiffFromNow(voltage_time) > 1000)
    {
        voltage_time = TimGetSystime(); // 更新当前检测时间

        /********** 单体电芯欠压1级（通知级）**********/
        if(gstCellInfo.cell_min_voltage < CELL_UV1)
        {
            gFaultAnalysisFlag.cell_under_votage1_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cell_under_votage1_count = 0;
        }
        if(gFaultAnalysisFlag.cell_under_votage1_count > 3) // 3s防抖
        {
            gBmsErrorInfo.cell1_error_code.b.cell_under_vol1 = 1;
            gPackErrorInfo.cell1_alarm_code.b.cell_under_vol1 = 1;
            gBMSErrorNumCode.cell_under_vol1_error_num = 40; // 故障编号40
        }
        else
        {
            if((gBmsErrorInfo.cell1_error_code.b.cell_under_vol1 == 1) && (gstCellInfo.cell_min_voltage > (CELL_UV1 + 50)))
            {
                gBmsErrorInfo.cell1_error_code.b.cell_under_vol1 = 0;
                gPackErrorInfo.cell1_alarm_code.b.cell_under_vol1 = 0;
                gBMSErrorNumCode.cell_under_vol1_error_num = 0;
            }
        }

        /********** 单体电芯欠压2级（报警级）**********/
        if(gstCellInfo.cell_min_voltage < CELL_UV2)
        {
            gFaultAnalysisFlag.cell_under_votage2_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cell_under_votage2_count = 0;
        }
        if(gFaultAnalysisFlag.cell_under_votage2_count > 3) // 3s防抖
        {
            gBmsErrorInfo.cell1_error_code.b.cell_under_vol2 = 1;
            gPackErrorInfo.cell1_error_code.b.cell_under_vol2 = 1;
            gBMSErrorNumCode.cell_under_vol2_error_num = 41; // 故障编号41
        }
        else
        {
            if((gBmsErrorInfo.cell1_error_code.b.cell_under_vol2 == 1) && (gstCellInfo.cell_min_voltage > (CELL_UV2 + 50)))
            {
                gBmsErrorInfo.cell1_error_code.b.cell_under_vol2 = 0;
                gPackErrorInfo.cell1_error_code.b.cell_under_vol2 = 0;
                gBMSErrorNumCode.cell_under_vol2_error_num = 0;
            }
        }        

        /********** 单体电芯欠压3级（错误级）**********/
        // 分温度场景判断：电芯温度>0℃时用阈值3，≤0℃时固定阈值2000mV

        if(gstCellInfo.cell_min_voltage < CELL_UV3)
        {
            gFaultAnalysisFlag.cell_under_votage3_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cell_under_votage3_count = 0;
        }
        if(gFaultAnalysisFlag.cell_under_votage3_count > 3) // 3s防抖
        {
            gBmsErrorInfo.cell1_error_code.b.cell_under_vol3 = 1;
            gPackErrorInfo.cell1_error_code.b.cell_under_vol3 = 1;
            gBMSErrorNumCode.cell_under_vol3_error_num = 42; // 故障编号42
        }


        /********** 单体电芯过压1级（通知级）**********/
        if(gstCellInfo.cell_max_voltage > CELL_OV1)
        {
            gFaultAnalysisFlag.cell_over_votage1_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cell_over_votage1_count = 0;
        }
        if(gFaultAnalysisFlag.cell_over_votage1_count > 3) // 3s防抖
        {
            gBmsErrorInfo.cell1_error_code.b.cell_over_vol1 = 1;
            gPackErrorInfo.cell1_alarm_code.b.cell_over_vol1 = 1;
            gBMSErrorNumCode.cell_over_vol1_error_num = 43; // 故障编号43
        }
        else
        {
            if((gBmsErrorInfo.cell1_error_code.b.cell_over_vol1 == 1) && (gstCellInfo.cell_max_voltage < (CELL_OV1 - 50)))
            {
                gBmsErrorInfo.cell1_error_code.b.cell_over_vol1 = 0;
                gPackErrorInfo.cell1_alarm_code.b.cell_over_vol1 = 0;
                gBMSErrorNumCode.cell_over_vol1_error_num = 0;
            }
        }

        /********** 单体电芯过压2级（报警级）**********/
        if(gstCellInfo.cell_max_voltage > CELL_OV2)
        {
            gFaultAnalysisFlag.cell_over_votage2_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cell_over_votage2_count = 0;
        }
        if(gFaultAnalysisFlag.cell_over_votage2_count > 3) // 3s防抖
        {
            gBmsErrorInfo.cell1_error_code.b.cell_over_vol2 = 1;
            gPackErrorInfo.cell1_error_code.b.cell_over_vol2 = 1;
            gBMSErrorNumCode.cell_over_vol2_error_num = 44; // 故障编号44
        }

        /********** 单体电芯过压3级（错误级）**********/
        if(gstCellInfo.cell_max_voltage > CELL_OV3)
        {
            gFaultAnalysisFlag.cell_over_votage3_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cell_over_votage3_count = 0;
        }
        if(gFaultAnalysisFlag.cell_over_votage3_count > 3) // 3s防抖
        {
            gBmsErrorInfo.cell1_error_code.b.cell_over_vol3 = 1;
            gPackErrorInfo.cell1_error_code.b.cell_over_vol3 = 1;
            gBMSErrorNumCode.cell_over_vol3_error_num = 45; // 故障编号45
        }

        /********** 电池包总欠压（错误级）**********/
        // 分温度场景：电芯温度>0℃时用阈值PACK_UV，≤0℃时固定阈值160mV
        if(gstCellInfo.cell_min_temperature > 0)
        {
            if(gstCellInfo.total_cell_voltage < PACK_UV)
            {
                gFaultAnalysisFlag.pack_under_votage_count ++;
            }
            else
            {
                gFaultAnalysisFlag.pack_under_votage_count = 0;
            }
        }
        else if(gstCellInfo.cell_min_temperature <= 0)
        {
            if(gstCellInfo.total_cell_voltage < 1600)
            {
                gFaultAnalysisFlag.pack_under_votage_count ++;
            }
            else
            {
                gFaultAnalysisFlag.pack_under_votage_count = 0;
            }
        }
        if(gFaultAnalysisFlag.pack_under_votage_count > 3) // 3s防抖
        {
            gBmsErrorInfo.cell1_error_code.b.pack_under_vol = 1;
            gPackErrorInfo.cell1_error_code.b.pack_under_vol3 = 1;
            gPackErrorInfo.cell1_error_code.b.pack_under_vol2 = 1;
            gBMSErrorNumCode.pack_under_votage_error_num = 47; // 故障编号47
        }

        /********** 电池包总过压（错误级）**********/
        if(gstCellInfo.total_cell_voltage > PACK_OV)
        {
            gFaultAnalysisFlag.pack_over_votage_count ++;
        }
        else
        {
            gFaultAnalysisFlag.pack_over_votage_count = 0;
        }
        if(gFaultAnalysisFlag.pack_over_votage_count > 3) // 3s防抖
        {
            gBmsErrorInfo.cell1_error_code.b.pack_over_vol = 1;
            gPackErrorInfo.cell1_error_code.b.pack_over_vol3 = 1;
            gPackErrorInfo.cell1_error_code.b.pack_over_vol2 = 1;
            gBMSErrorNumCode.pack_over_votage_error_num = 46; // 故障编号46
        }
    }

    /****************************************** 电芯电压不均衡故障检测 ******************************************/
// 检测周期：1000ms（1s）
    if(TimeDiffFromNow(cell_voltage_diff_time) > 1000)
    {
        cell_voltage_diff_time = TimGetSystime(); // 更新当前检测时间

        // 故障条件：电芯最大电压 - 最小电压 > 电压差阈值
        if((gstCellInfo.cell_max_voltage - gstCellInfo.cell_min_voltage) > CELL_V_DIFF)
        {
            gFaultAnalysisFlag.cell_voltage_diff_count ++; // 计数+1
        }
        else
        {
            gFaultAnalysisFlag.cell_voltage_diff_count = 0; // 计数清零
        }

        // 防抖计数>10次（10s）：置位故障码
        if(gFaultAnalysisFlag.cell_voltage_diff_count > 3)
        {
            gBmsErrorInfo.cell1_error_code.b.cell_voltage_diff = 1;
            gPackErrorInfo.cell1_error_code.b.cell_voltage_diff = 1;
        }
    }

    /****************************************** 电压传感器故障检测 ******************************************/
// 检测周期：1000ms（1s）
    if(TimeDiffFromNow(vsensor_error_time) > 1000)
    {
        vsensor_error_time = TimGetSystime(); // 更新当前检测时间

        // 故障条件：电芯最大电压>4000mV 或 最小电压<2000mV（超出合理范围，判定传感器异常）
        if(gstCellInfo.cell_max_voltage > 4000 || gstCellInfo.cell_min_voltage < 2000)
        {
            gFaultAnalysisFlag.cell_votage_sensor_error_count ++; // 计数+1
        }
        else
        {
            gFaultAnalysisFlag.cell_votage_sensor_error_count = 0; // 计数清零
        }

        // 防抖计数>10次（10s）：置位传感器故障码
        if(gFaultAnalysisFlag.cell_votage_sensor_error_count > 3)
        {
            gBmsErrorInfo.cellsample_error_code.b.vol_sensor_error = 1;
            gPackErrorInfo.cell1_error_code.b.vol_sensor_error = 1;
        }
    }

    /****************************************** 温度故障检测（过温/欠温/温差） ******************************************/
// 检测周期：1000ms（1s）
    if(TimeDiffFromNow(temperature_time) > 1000)
    {
        temperature_time = TimGetSystime(); // 更新当前检测时间

        /********** 电芯过温1级（通知级）**********/
        if(gstCellInfo.cell_max_temperature > CELL_OT1)
        {
            gFaultAnalysisFlag.cell_temperature_ot1_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cell_temperature_ot1_count = 0;
        }
        if(gFaultAnalysisFlag.cell_temperature_ot1_count > 30) // 30s防抖
        {
            gBmsErrorInfo.cell1_error_code.b.pack_high_temper1 = 1;
            gPackErrorInfo.temp_alarm_code.b.cell_high_temper1 = 1;
            gBMSErrorNumCode.pack_high_temper1_error_num = 18; // 故障编号18
        }
        else
        {
            if((gBmsErrorInfo.cell1_error_code.b.pack_high_temper1 == 1) && (gstCellInfo.cell_max_temperature < (CELL_OT1 - 30)))
            {
                gBmsErrorInfo.cell1_error_code.b.pack_high_temper1 = 0;
                gPackErrorInfo.temp_alarm_code.b.cell_high_temper1 = 0;
                gBMSErrorNumCode.pack_high_temper1_error_num = 0;
            }
        }

        /********** 电芯过温2级（报警级）**********/
        if(gstCellInfo.cell_max_temperature > CELL_OT2)
        {
            gFaultAnalysisFlag.cell_temperature_ot2_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cell_temperature_ot2_count = 0;
        }
        if(gFaultAnalysisFlag.cell_temperature_ot2_count > 100) // 3s防抖
        {
            gBmsErrorInfo.cell1_error_code.b.pack_high_temper2 = 1;
            gPackErrorInfo.temp_error_code.b.cell_high_temper2 = 1;
            gBMSErrorNumCode.pack_high_temper2_error_num = 19; // 故障编号19
        }

        /********** 电芯过温3级（错误级）**********/
        if(gstCellInfo.cell_max_temperature > CELL_OT3)
        {
            gFaultAnalysisFlag.cell_temperature_ot3_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cell_temperature_ot3_count = 0;
        }
        if(gFaultAnalysisFlag.cell_temperature_ot3_count > 100) // 3s防抖
        {
            gBmsErrorInfo.cell1_error_code.b.pack_high_temper3 = 1;
            gPackErrorInfo.temp_error_code.b.cell_high_temper3 = 1;
            gBMSErrorNumCode.pack_high_temper3_error_num = 20; // 故障编号20
        }

        /********** 电芯欠温（通知级）**********/
        if(gstCellInfo.cell_min_temperature < CELL_UT1)
        {
            gFaultAnalysisFlag.cell_temperature_ut1_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cell_temperature_ut1_count = 0;
        }
        /********** 电芯欠温（报警级）**********/
        if(gstCellInfo.cell_min_temperature < CELL_UT2)
        {
            gFaultAnalysisFlag.cell_temperature_ut2_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cell_temperature_ut2_count = 0;
        }
        /********** 电芯欠温（错误级）**********/
        if(gstCellInfo.cell_min_temperature < CELL_UT3)
        {
            gFaultAnalysisFlag.cell_temperature_ut3_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cell_temperature_ut3_count = 0;
        }        
        
        if(gFaultAnalysisFlag.cell_temperature_ut1_count > 30) // 30s防抖
        {
            gBmsErrorInfo.cell1_error_code.b.pack_low_temper1 = 1;
            gPackErrorInfo.temp_alarm_code.b.cell_low_temper1 = 1;
            gBMSErrorNumCode.pack_low_temper_error_num = 21; // 故障编号21
        }
        else
        {
            if((gBmsErrorInfo.cell1_error_code.b.pack_low_temper1 == 1) && (gstCellInfo.cell_min_temperature > (CELL_UT1 + 30)))
            {
                gBmsErrorInfo.cell1_error_code.b.pack_low_temper1 = 0;
                gPackErrorInfo.temp_alarm_code.b.cell_low_temper1 = 0;
                gBMSErrorNumCode.pack_low_temper_error_num = 0;
            }
        }        
        if(gFaultAnalysisFlag.cell_temperature_ut2_count > 100) // 100s防抖
        {
            gBmsErrorInfo.cell1_error_code.b.pack_low_temper2 = 1;
            gPackErrorInfo.temp_error_code.b.cell_low_temper2 = 1;
            gBMSErrorNumCode.pack_low_temper_error_num = 22; // 故障编号22
        }
        if(gFaultAnalysisFlag.cell_temperature_ut3_count > 100) // 100s防抖
        {
            gBmsErrorInfo.cell1_error_code.b.pack_low_temper3 = 1;
            gPackErrorInfo.temp_error_code.b.cell_low_temper3 = 1;
            gBMSErrorNumCode.pack_low_temper_error_num = 23; // 故障编号23
        }

        
        /********** 电芯温差过大（错误级）**********/
        if(((gstCellInfo.max_temperature_diff) >= CELL_T_DIFF))
        {
            gFaultAnalysisFlag.cell_temperature_ltd_count ++;
        }
        else
        {
            gFaultAnalysisFlag.cell_temperature_ltd_count = 0;
        }
        if(gFaultAnalysisFlag.cell_temperature_ltd_count > 3) // 3s防抖
        {
            gBmsErrorInfo.cell1_error_code.b.pack_temper_differ = 1;
            gPackErrorInfo.temp_error_code.b.cell_temper_differ = 1;
            gBMSErrorNumCode.pack_temper_differ_error_num = 24; // 故障编号24
        }
    }

    /****************************************** 温度传感器故障检测 ******************************************/
// 检测周期：1000ms（1s）
    if(TimeDiffFromNow(tsensor_error_time) >= 1000)
    {
        tsensor_error_time = TimGetSystime(); // 更新当前检测时间

        // 故障条件：温度>传感器过温阈值（120℃）或 <欠温阈值（-40℃）
        if(gstCellInfo.cell_max_temperature > TSENSOR_OT || gstCellInfo.cell_min_temperature < TSENSOR_UT)
        {
            gFaultAnalysisFlag.cell_temperature_sensor_error_count ++; // 计数+1
        }
        else
        {
            gFaultAnalysisFlag.cell_temperature_sensor_error_count = 0; // 计数清零
        }

        // 防抖计数>10次（10s）：置位温度传感器故障码
        if(gFaultAnalysisFlag.cell_temperature_sensor_error_count > 3)
        {
            gBmsErrorInfo.cellsample_error_code.b.cell_ntc_error = 1;
            gPackErrorInfo.temp_error_code.b.cell_ntc_error = 1;
        }
        /****************************************** 温度传感器严重故障检测 ******************************************/
        if(gBmsErrorInfo.cellsample_error_code.b.cell_ntc_error)
        {
            uint8_t count = 0;
            // 遍历9个电芯温度传感器，统计异常数量
            for(uint8_t i = 0; i < 9; i++)
            {
                if(gstCellInfo.cell_temperature[i] > TSENSOR_OT || gstCellInfo.cell_temperature[i] < TSENSOR_UT)
                {
                    count ++; // 异常传感器数量+1
                }
            }

            // 超过1个传感器异常，计数+1
            if(count > 1)
            {
                gFaultAnalysisFlag.cell_temperature_sensor_error_count1 ++;
            }
            else
            {
                gFaultAnalysisFlag.cell_temperature_sensor_error_count1 = 0;
            }

            // 防抖计数>3次：置位严重故障码
            if(gFaultAnalysisFlag.cell_temperature_sensor_error_count1 > 3)
            {
                gBmsErrorInfo.cellsample_error_code.b.cell_ntc_fatal_error = 1;
                gPackErrorInfo.temp_error_code.b.cell_ntc_fatal_error = 1;
            }
        }
    }
    /****************************************** SOH过低故障检测（电池健康度） ******************************************/
// 检测周期：1000ms（1s）
    if(TimeDiffFromNow(soh_under_time) > 1000)
    {
        soh_under_time = TimGetSystime(); // 更新当前检测时间

        // 调试模式下固定SOH为1000（100.0%）
        if(gSoxDebugParam.sox_debug_mode)
        {
            gPackStatus.soh_rpt = 1000;
        }

        // 分别检测SOH低于3个等级阈值的情况
        if(gPackStatus.soh_rpt < SOH_UNDER_LVL1)
        {
            gFaultAnalysisFlag.soh_under_lvl1_count ++;
        }
        else
        {
            gFaultAnalysisFlag.soh_under_lvl1_count = 0;
        }
        if(gPackStatus.soh_rpt < SOH_UNDER_LVL2)
        {
            gFaultAnalysisFlag.soh_under_lvl2_count ++;
        }
        else
        {
            gFaultAnalysisFlag.soh_under_lvl2_count = 0;
        }
        if(gPackStatus.soh_rpt < SOH_UNDER_LVL3)
        {
            gFaultAnalysisFlag.soh_under_lvl3_count ++;
        }
        else
        {
            gFaultAnalysisFlag.soh_under_lvl3_count = 0;
        }

        // SOH过低1级（通知级）：5s防抖
        if(gFaultAnalysisFlag.soh_under_lvl1_count > 3)
        {
            gBmsErrorInfo.cell1_error_code.b.pack_low_soh1 = 1;
            gPackErrorInfo.sox_alarm_code.b.pack_low_soh1 = 1;
        }
        // SOH过低2级（报警级）：5s防抖
        if(gFaultAnalysisFlag.soh_under_lvl2_count > 3)
        {
            gBmsErrorInfo.cell1_error_code.b.pack_low_soh2 = 1;
            gPackErrorInfo.sox_error_code.b.pack_low_soh2 = 1;
        }
        // SOH过低3级（错误级）：5s防抖
        if(gFaultAnalysisFlag.soh_under_lvl3_count > 3)
        {
            gBmsErrorInfo.cell1_error_code.b.pack_low_soh3 = 1;
            gPackErrorInfo.sox_error_code.b.pack_low_soh3 = 1;
        }
    }


    /****************************************** 加热膜故障检测 ******************************************/
    // 检测周期：1000ms（1s）
    if(TimeDiffFromNow(heat_error_time) >= 1000)
    {
        heat_error_time = TimGetSystime(); // 更新当前检测时间

        // 故障条件：加热使能（HeatEn）和加热检测（HeatCk）状态不一致（均为1或均为0）
        if(((GpioBitGet((&HeatEnHandle))) && (GpioBitGet((&HeatCkHandle)))) ||
                (!(GpioBitGet((&HeatEnHandle))) && (!(GpioBitGet((&HeatCkHandle))))))
        {
            gFaultAnalysisFlag.heat_error_count ++; // 计数+1
        }
        else
        {
            gFaultAnalysisFlag.heat_error_count = 0; // 计数清零
        }

        // 防抖计数>3次（3s）：置位加热膜故障码
        if(gFaultAnalysisFlag.heat_error_count > 3)
        {
            gBmsErrorInfo.other1_error_code.b.heat_error = 1;
            gPackErrorInfo.other_error_code.b.heat_error = 1;
        }
    }

    /****************************************** 其他故障检测 ******************************************/
// Flash故障：根据标志位直接置位故障码
    if(gFaultAnalysisFlag.flash_error_flag)
    {
        gBmsErrorInfo.other1_error_code.b.spi_flash_error = 1;
        gPackErrorInfo.other_error_code.b.spi_flash_error = 1;
    }

// E2PROM故障：根据标志位直接置位故障码
    if(gFaultAnalysisFlag.e2prom_error_flag)
    {
        gBmsErrorInfo.other1_error_code.b.e2prom_error = 1;
        gPackErrorInfo.other_error_code.b.e2prom_error = 1;
    }
}