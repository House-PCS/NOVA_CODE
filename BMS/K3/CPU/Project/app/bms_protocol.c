/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        bms_protocol.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-6-1
  * @brief       BMS（电池管理系统）核心协议处理模块，包含系统错误检测、运行状态管理、
  *              温度采集更新、加热控制、休眠检测等核心功能实现
  ******************************************************************************
  * @attention
  * 1. 本模块依赖bms_protocol.h头文件定义的结构体和接口
  * 2. 温度数据单位为0.1℃（例如250表示25.0℃），温差计算需注意单位转换
  * 3. 错误码采用位掩码方式定义，需结合具体硬件故障定义表解读
  * 4. 功率百分比范围为0~1，对应0%~100%充放电功率
  * @relation
  * @par :
  * <table>
  * <tr><<th>Date          <<th>Version   <<th>Author      <<th>Description
  * <tr><td>2025-6-1    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "bms_protocol.h"
#define LOG_TAG     "bms_protocol"  // 日志标签，用于调试输出
#include "math.h"                   // 数学库，用于绝对值计算等

#define NTC_NUM  9                   // NTC温度传感器数量（实际采集10路电芯温度，预留冗余）

uint8_t error_level = 0;             // 系统错误等级（0-无错误，1-轻微错误，2-严重错误，3-紧急错误）
BMSSysInfo_t   gBMSSys_info;         // BMS系统信息结构体，存储系统故障状态等信息

// 判断是否仅存在欠压一/二级故障（无其他故障）
static bool IsOnlyCellUndervoltError(void)
{
    // 提取电芯欠压相关的故障位（一/二级）
    uint32_t cell_undervolt_bits = gBmsErrorInfo.cell1_error_code.value & (0x0001 | 0x0002); // 欠压1级=bit0，欠压2级=bit1
    
    // 其他故障位（排除欠压一/二级）
    uint32_t other_errors = (gBmsErrorInfo.mos_error_code.value) | 
                            (gBmsErrorInfo.curr_error_code.value) | 
                            (gBmsErrorInfo.cell1_error_code.value & ~(0x0001 | 0x0002)) | // 排除欠压一/二级
                            (gBmsErrorInfo.cellsample_error_code.value) | 
                            (gBmsErrorInfo.other1_error_code.value);
    
    // 条件：存在欠压一/二级，且无其他故障
    return (cell_undervolt_bits != 0) && (other_errors == 0);
}

/**
  * @brief   获取系统错误等级
  * @details 根据各类故障码（MOS管、电流、电芯、采样等）的位掩码组合，判定系统错误等级
  *          错误等级优先级：3级（最高）>2级>1级>0级（无错误）
  * @param   无
  * @retval  uint8_t - 系统错误等级（0~3）
  */
uint8_t GetSystemErrorLevel(void)
{
    // 先判断是否仅存在欠压一/二级故障
    if (IsOnlyCellUndervoltError())
    {
        // 仅欠压一/二级：二级优先于一级
        if(gBmsErrorInfo.cell1_error_code.b.cell_under_vol2)
        {
            gBMSSys_info.bms_sys_fail_sta = 2;
        }
        else if(gBmsErrorInfo.cell1_error_code.b.cell_under_vol1)
        {
            gBMSSys_info.bms_sys_fail_sta = 1;
        }
        return gBMSSys_info.bms_sys_fail_sta;
    }
    // 3级错误：严重故障组合（MOS管/电流/电芯1/精度/采样/其他1故障）
    if((gBmsErrorInfo.mos_error_code.value & 0x0003) ||             // MOS管故障码掩码（特定故障位）
            (gBmsErrorInfo.curr_error_code.value & 0x0024) ||       // 电流故障码掩码
            (gBmsErrorInfo.cell1_error_code.value & 0x4E4E4) ||      // 电芯1故障码掩码
            (gBmsErrorInfo.cellsample_error_code.value & 0x000F) || // 电芯采样故障码掩码
            (gBmsErrorInfo.other1_error_code.value & 0x001F))       // 其他1故障码掩码
    {
        gBMSSys_info.bms_sys_fail_sta = 3;  // 标记为3级错误
    }
    // 2级错误：较严重故障组合
    else if((gBmsErrorInfo.curr_error_code.value & 0x0012) ||
            (gBmsErrorInfo.cell1_error_code.value & 0x21210))
    {
        gBMSSys_info.bms_sys_fail_sta = 2;  // 标记为2级错误
    }
    // 1级错误：轻微故障组合
    else if((gBmsErrorInfo.curr_error_code.value & 0x0009) ||
            (gBmsErrorInfo.cell1_error_code.value & 0x10908))
    {
        gBMSSys_info.bms_sys_fail_sta = 1;  // 标记为1级错误
    }
    // 无错误
    else
    {
        gBMSSys_info.bms_sys_fail_sta = 0;  // 标记为0级（无错误）
    }

    return gBMSSys_info.bms_sys_fail_sta;  // 返回系统错误等级
}

/**
  * @brief   紧急错误检测（静态内部函数，仅本文件调用）
  * @details 结合系统错误等级和温度状态，判定是否需要紧急下电，并设置充放电功率百分比
  * @param   无
  * @retval  bool - 紧急下电标志：true=需要紧急下电，false=无需紧急下电
  */
static bool _EmergencyErrorCheck(void)
{
    // 获取当前系统错误等级（外部接口：返回0=正常/1=轻微/2=严重/3=紧急）
    error_level = GetSystemErrorLevel();
    bool is_only_undervolt = IsOnlyCellUndervoltError();
    bool mos_fault = GetMosCheck();   // 独立采样MOS故障，避免短路导致漏检
    float power = gPackStatus.current_power_percentage;
    bool power_is_0 = (power < 0.1f);
    bool power_is_08 = (power > 0.7f) && (power < 0.9f);
    bool power_is_1 = (power > 0.9f);
    bool power_is_05 = (power > 0.4f) && (power < 0.6f);
    //系统错误等级判断（最高优先级：满足则直接触发紧急下电或功率限制）
    // 紧急下电条件：严重错误（2级）、紧急错误（3级），或MOS管故障检测触发 注意：欠压不关mos 只有电压低于2v才关闭mos
    if ((!is_only_undervolt) && ((error_level == 3) || (error_level == 2) || mos_fault))
    {
        return true;  // 需要紧急下电，退出函数
    }
    // 轻微错误（1级）：充放电功率限制为50%（无需下电）
    else if (error_level == 1)
    {
        gPackStatus.current_power_percentage = 0.5;  // 功率限制：50%
        return false;  // 无需紧急下电
    }

    // 电芯温差判断（系统无错误时生效，包含滞回机制避免频繁切换）
    // 极端温差（>8.0℃）：禁止充放电（功率限制0%）
    if (gstCellInfo.max_temperature_diff > 80)
    {
        gPackStatus.current_power_percentage = 0;  
        return false;
    }
    // 中度温差（6.0℃~8.0℃）：功率限制80%（滞回上限触发条件）
    else if ((gstCellInfo.max_temperature_diff <= 60) && power_is_0)
    {
        gPackStatus.current_power_percentage = 0.8f;  // 功率限制：80%
        return false;
    }
    // 中度温差（6.0℃~8.0℃）：功率限制80%（滞回上限触发条件）
    else if ((gstCellInfo.max_temperature_diff >= 60) && (power_is_1 || power_is_05))
    {
        gPackStatus.current_power_percentage = 0.8f;  // 功率限制：80%
        return false;
    }
    // 低温差（≤4.0℃）+ 当前功率为80%：恢复满功率（100%，滞回下限触发条件）
    // 注：仅当之前因温差限制到80%时，才恢复满功率（避免无限制反复切换）
    else if ((gstCellInfo.max_temperature_diff <= 40) && (power_is_08 || power_is_05))
    {
        gPackStatus.current_power_percentage = 1.0f;  // 功率恢复：100%
        return false;
    }

    // 4. 其他正常情况（温差4.0℃~6.0℃，或功率已为满功率/50%等）
    // 保持当前功率状态（不修改current_power_percentage）
    return false;  // 无需紧急下电
}

/**
  * @brief   BMS运行状态机处理
  * @details 实现BMS三种运行状态（空闲/正常/下电）的切换逻辑，包含充放电MOS管控制
  *          状态切换依据：紧急错误检测结果
  * @param   无
  * @retval  无
  */
void BMSRunStateProcess(void)
{
    BMSSleepCheck();       // 休眠检测（先判断是否需要休眠）
    HeatCtrlProcess();     // 加热控制（根据温度和SOC控制加热使能）
    static EN_BMS_RUNSTATE _geUpLoadStatus = EN_BMS_RUNSTATE_IDLE;  // 运行状态机变量（初始为空闲态）

    switch(_geUpLoadStatus)
    {
        // 空闲状态：系统初始化后的初始状态
        case EN_BMS_RUNSTATE_IDLE:
        {
            // 检测到紧急错误：执行下电流程
            if(_EmergencyErrorCheck())
            {
                // 若充放电MOS管处于打开状态，关闭MOS管
                if((ChargeMosIsOpen() == true) || (DisChargeMosIsOpen() == true))
                {
                    CloseChargeMos();   // 关闭充电MOS管
                    CloseDisChargeMos();// 关闭放电MOS管
                }
                _geUpLoadStatus = EN_BMS_RUNSTATE_OFF;  // 切换到下电状态
                break;
            }
            // 无紧急错误：切换到正常运行状态
            else
            {
                _geUpLoadStatus = EN_BMS_RUNSTATE_NORMAL;
            }
            break;
        }

        // 正常运行状态：充放电正常工作
        case EN_BMS_RUNSTATE_NORMAL:
        {
            // 运行中检测到紧急错误：执行下电流程
            if(_EmergencyErrorCheck())
            {
                // 若充放电MOS管处于打开状态，关闭MOS管
                if((ChargeMosIsOpen() == true) || (DisChargeMosIsOpen() == true))
                {
                    CloseChargeMos();   // 关闭充电MOS管
                    CloseDisChargeMos();// 关闭放电MOS管
                }
                _geUpLoadStatus = EN_BMS_RUNSTATE_OFF;  // 切换到下电状态
                break;
            }
            break;
        }
        // 下电状态：系统紧急下电后的状态
        case EN_BMS_RUNSTATE_OFF:
        {
            // 仍检测到紧急错误：保持下电状态
            if(_EmergencyErrorCheck())
            {
                break;
            }
            break;
        }
        // 默认状态：异常状态保护，无操作
        default:
            break;
    }
}

/**
  * @brief   加热控制处理
  * @details 根据电芯最低温度和SOC（剩余电量）状态，控制加热使能开关
  *          加热使能条件：电芯最低温度<5℃ 且 SOC>50%
  *          加热关闭条件：电芯最低温度>8℃ 或 SOC<50%
  * @param   无
  * @retval  无
  */
void HeatCtrlProcess()
{
    // 加热使能条件：电芯最低温度<5℃（50=5.0℃）且 SOC>50%
    if((gstCellInfo.cell_min_temperature < 50) && (garrBMSReg.bms_only_read_para.soc_rpt > 50))
    {
        GpioBitSet((&HeatEnHandle));  // 置位加热使能GPIO（开启加热）
        garrBMSReg.bms_only_read_para.heat_status = 1;  // 标记加热状态为开启（1=开启）
    }

    // 加热关闭条件：已开启加热，且（电芯最低温度>8℃ 或 SOC<50%）
    if((GpioBitGet((&HeatEnHandle))) && ((gstCellInfo.cell_min_temperature > 80) || (garrBMSReg.bms_only_read_para.soc_rpt < 50)))
    {
        GpioBitReset((&HeatEnHandle));  // 复位加热使能GPIO（关闭加热）
        garrBMSReg.bms_only_read_para.heat_status = 0;  // 标记加热状态为关闭（0=关闭）
    }
}

uint32_t ADC_BMS_current1 = 0;  // ADC采集的电流值1（对应ADC_CHANNEL_1）
uint32_t ADC_BMS_current2 = 0;  // ADC采集的电流值2（对应ADC_CHANNEL_0）

/**
  * @brief   温度数据更新
  * @details 1. 调试模式下：设置固定温度值（25.0℃）
  *          2. 正常模式下：通过ADC采集10路电芯温度（9个NTC传感器+1路AFE网络温度）
  *          3. 计算电芯平均温度、最高温度、最低温度、最大温差
  *          4. 采集两路电流ADC数据
  * @param   无
  * @retval  无
  */
void TemperatureUpdate()
{
    uint8_t i = 0;          // 循环变量
    int16_t ave_temp = 0;   // 温度总和临时变量（用于计算平均值）

    /* 温度读取逻辑 */
    if(gSoxDebugParam.sox_debug_mode)  // 调试模式：使用固定温度值
    {
        for(uint8_t i = 0; i < 10; i++)
        {
            gstCellInfo.cell_temperature[i] = 250;  // 所有电芯温度设为25.0℃（250=25.0℃）
            gstCellInfo.max_temperature_diff = 0;   // 最大温差设为0
            gstCellInfo.afe_net_temperature = 250;  // AFE网络温度设为25.0℃
        }
    }
    else  // 正常模式：通过ADC采集实际温度
    {
        // 测量电芯8/7的极柱温度（通过GPIO引脚切换通道）
        ResetBMSTempS2();  // 复位温度采集控制引脚S2
        ResetBMSTempS1();  // 复位温度采集控制引脚S1
        ResetBMSTempS0();  // 复位温度采集控制引脚S0
        gstCellInfo.cell_temperature[8] = (TemperatureGet(ADC_CHANNEL_10) * 10);  // （ADC通道10）
        gstCellInfo.cell_temperature[7] = (TemperatureGet(ADC_CHANNEL_11) * 10);  // （ADC通道11）

        // 测量电芯6/5的极柱温度
        ResetBMSTempS2();
        ResetBMSTempS1();
        SetBMSTempS0();    // 置位温度采集控制引脚S0（切换通道）
        gstCellInfo.cell_temperature[6] = (TemperatureGet(ADC_CHANNEL_10) * 10);
        gstCellInfo.cell_temperature[5] = (TemperatureGet(ADC_CHANNEL_11) * 10);

        // 测量电芯4/3的极柱温度
        ResetBMSTempS2();
        SetBMSTempS1();    // 置位温度采集控制引脚S1（切换通道）
        ResetBMSTempS0();
        gstCellInfo.cell_temperature[4] = (TemperatureGet(ADC_CHANNEL_10) * 10);
        gstCellInfo.cell_temperature[3] = (TemperatureGet(ADC_CHANNEL_11) * 10);

        // 测量电芯2/1的极柱温度
        ResetBMSTempS2();
        SetBMSTempS1();
        SetBMSTempS0();
        gstCellInfo.cell_temperature[2] = (TemperatureGet(ADC_CHANNEL_10) * 10);
        gstCellInfo.cell_temperature[1] = (TemperatureGet(ADC_CHANNEL_11) * 10);

        // 测量电芯0的极柱温度 + AFE网络温度
        SetBMSTempS2();    // 置位温度采集控制引脚S2（切换通道）
        ResetBMSTempS1();
        ResetBMSTempS0();
        gstCellInfo.cell_temperature[0] = (TemperatureGet(ADC_CHANNEL_10) * 10);  
        gstCellInfo.afe_net_temperature = (TemperatureGet(ADC_CHANNEL_10) * 10);  // AFE网络温度（复用ADC通道10）
    }

    // 初始化温度统计参数（默认以平均温度为初始值）
    gstCellInfo.cell_min_temperature = gstCellInfo.cell_ave_temperature;
    gstCellInfo.cell_max_temperature = gstCellInfo.cell_ave_temperature;
    gstCellInfo.max_temperature_diff = 0;

    // 计算电芯间最大温差（相邻电芯温差对比）
    for(i = 1; i < NTC_NUM - 2; i ++)
    {
        // 计算当前电芯与下一个电芯的温度差绝对值
        if(abs(gstCellInfo.cell_temperature[i] - gstCellInfo.cell_temperature[i + 1]) >= gstCellInfo.max_temperature_diff)
        {
            gstCellInfo.max_temperature_diff = abs(gstCellInfo.cell_temperature[i] - gstCellInfo.cell_temperature[i + 1]);
        }
    }

    // 计算最高温度、最低温度
    for(i = 0; i < NTC_NUM; i ++)
    {
        // 更新最高温度
        if(gstCellInfo.cell_temperature[i] >= gstCellInfo.cell_max_temperature)
        {
            gstCellInfo.cell_max_temperature = gstCellInfo.cell_temperature[i];
        }
        // 更新最低温度
        else if(gstCellInfo.cell_temperature[i] <= gstCellInfo.cell_min_temperature)
        {
            gstCellInfo.cell_min_temperature = gstCellInfo.cell_temperature[i];
        }
    }

    for(i = 1; i < NTC_NUM - 1; i ++)
    {
        ave_temp += gstCellInfo.cell_temperature[i];  // 累加所有电芯温度
    }
    gstCellInfo.cell_ave_temperature = ave_temp / (NTC_NUM - 2);  // 平均温度 = 总温度 / 传感器数量

    // 采集两路电流ADC数据（通道1和通道0）
    ADC_BMS_current1 = (AdcBMSCurrentGet(ADC_CHANNEL_1));
    ADC_BMS_current2 = (AdcBMSCurrentGet(ADC_CHANNEL_0));
}

/**
  * @brief   BMS休眠检测
  * @details 当系统无唤醒信号时，累计休眠时间，达到阈值后执行休眠下电流程
  *          休眠条件：无唤醒标志（gBMSSleepFlag=1）且持续时间≥120秒，最多尝试2次下电
  * @param   无
  * @retval  无
  */
void BMSSleepCheck()
{
    static uint8_t  poweroff_count = 0;  // 下电尝试次数计数器（最多2次）
    static uint32_t sleep_time = 0;      // 休眠计时起始时间

    // 首次进入：设置休眠标志和起始时间
    if(!gBMSSleepFlag)
    {
        gBMSSleepFlag = 1;               // 置位休眠标志（开始计时）
        sleep_time = TimGetSystime();    // 获取当前系统时间（作为计时起点）
    }
    else  // 已置位休眠标志：判断是否达到休眠阈值
    {
        // 休眠时间≥120秒（120000ms）且下电次数<2次
        if((TimeDiffFromNow(sleep_time) > 120000) && (poweroff_count < 2))
        {
            garrBMSReg.bms_hold_para.bms_shutdown = 0x0002;  // 设置BMS下电控制字（特定值触发下电）
            sleep_time = TimGetSystime();    // 更新计时起点（避免唤醒后立即再次休眠）
            SoxFlashInfoSave();              // 保存SOX相关Flash数据（防止数据丢失）
            SoxEepromInfoSave();             // 保存SOX相关EEPROM数据
            CloseAllMOS();                   // 关闭所有MOS管（停止充放电）            
            BMSBoardPowerOff();              // 执行BMS主板下电操作
            poweroff_count ++;               // 下电次数+1
        }
    }
}

/**
  * @brief   MOS管故障检测
  * @details 通过GPIO引脚检测MOS管故障状态，连续3次检测到故障则判定为MOS管故障
  * @param   无
  * @retval  bool - MOS管故障标志：true=故障，false=正常
  */
bool GetMosCheck(void)
{
    static uint8_t  mos_error_count = 0;  // MOS管故障计数（连续检测）

    // 检测到MOS管故障GPIO引脚为高电平（故障状态）
    if(GpioBitGet((&ErrorCkHandle)))
    {
        mos_error_count++;  // 故障计数+1
    }
    else  // 故障解除：计数清零
    {
        mos_error_count = 0;
    }

    // 连续3次检测到故障：判定为MOS管故障
    if(mos_error_count >= 3)
    {
        return true;
    }
    else  // 未达到连续故障次数：判定为正常
    {
        return false;
    }
}