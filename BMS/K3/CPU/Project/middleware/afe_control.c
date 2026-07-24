/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        afe_control.c  // 修正原文件引用冲突，实际应为afe_control.c（因包含afe_control相关逻辑）
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-6-1
  * @brief       BMS AFE（模拟前端）控制模块核心代码，基于BQ769x2芯片
  *              实现电池电压/电流/温度采集、电芯均衡控制、断线检测等功能
  ******************************************************************************
  * @attention   1. 依赖I2C通信驱动、GPIO驱动、延时函数
  *              2. 编译时需根据硬件配置开启对应宏定义（如温度采样、高压采样）
  *              3. 状态机驱动核心逻辑，禁止随意修改执行顺序
  * @relation    依赖文件：afe_control.h, bsp.h, 设备驱动文件（如bq769x2驱动）
  * @par :
  * <table>
  * <tr><<th>Date          <<th>Version   <<th>Author      <<th>Description
  * <tr><td>2025-6-1    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "afe_control.h"
#include "bsp.h"
// 日志标签，用于调试输出
#define LOG_TAG "afe_control"
// 温度传感器数量（最大支持4个）
#define TSENSOR_NUM 4
/************************* 均衡参数配置（可根据硬件修改） *************************/
#define CELL_STRINGS_MAX            8      // 最大电芯串数
#define CELL_BAL_DIFF_THRESH        20      // 均衡触发压差(mV)
#define CELL_BAL_MIN_VOLT           3100    // 均衡最低保护电压(mV)
#define CELL_CAPACITY               314     // 电芯容量(Ah)
#define BAL_CURRENT                 100     // 均衡电流(mA)
#define CELL_BAL_BAT_PERCENT        0.003f  // 单次均衡电量比例 0.3%
#define CURRENT_WINDOW_SIZE         25      // 滑窗长度：60个点
#define LPF_SHIFT                   15      // 定点放大系数 2^15
static EN_AFE_STATE gAfeState = EN_AFE_IDLE;
// 8节电芯 → AFE均衡BIT映射（100%匹配硬件：VC1→bit0, VC2→bit1, VC3→bit2, VC4→bit3, VC7→bit6, VC11→bit10, VC15→bit14, VC16→bit15）
const uint8_t CELL_TO_AFE_BIT[CELL_STRINGS_MAX] = {0, 1, 2, 3, 6, 10, 14, 15};
const uint8_t BAL_GROUP[2][4] = {{0, 2, 4, 6}, {1, 3, 5, 7}};
// INA240电流传感器参考值（初始化后校准使用）
static uint16_t gINA240Ref = 0;
// 中断处理函数声明（内部使用）
static void _irq_process(void);
// 所有AFE芯片（BQ769x2）的设备信息结构体数组（TOTAL_IC为芯片数量，此处配置为1）
AFECellAsic_t gIC[TOTAL_IC];
// 电芯全局信息结构体（存储电压、电流、温度、均衡状态等）
CellInfo_t    gstCellInfo;
// AFE控制驱动结构体（状态机、请求队列管理）
AfeCtrolDrv_t gAfeCtrolDrv;
// AFE工作状态枚举（IDLE/采集/均衡等）

// AFE首次采集完成标志（0-未开始，1-电压采集完成，2-电流采集完成，3-就绪，4-停止）
uint8_t gFirst_AFE_Coved_Flag = 0;
// 上一次电源键状态（用于故障复位判断）
static bool _gPrePowKeySts;
// 电芯均衡分组表（分2组，每组4个电芯，轮流均衡避免功耗过高）

/* const value ------------------------------------------------------------------*/
// AFE芯片I2C设备地址（BQ769x2默认7位地址0x08，此处为8位地址（包含读写位）0x10）
const uint8_t gDeviceAddress[TOTAL_IC] = {0x10};

/* set cfg value ----------------------------------------------------------------*/
// AFE控制配置结构体（单次请求队列+循环请求队列）
const AfeCtrolCfg_t gAfeCtrolCfg =
{
    /* 初始化请求队列（仅执行一次） */
    {
        EN_REQUEST_INITIALIZE,  // 初始化请求
    },
    /* 单次请求队列长度 */
    AFE_CONTROL_MAX_REQUEST_SINGLE_LEN,
    /* 循环请求队列（持续循环执行） */
    {
        EN_REQUEST_CONT_MEASURE,        // 连续测量
        EN_REQUEST_READ_CELL_VOLTAGES,  // 读取电芯电压
#if TEMPERATURE_SAMPLE_EN
        EN_REQUEST_READ_GPIO_VOLTAGES,  // 读取GPIO电压（温度采样）
#endif
#if HIGH_VOLTAGE_SAMPLE_EN
        EN_REQUEST_READ_STACK_VOLTAGES, // 读取堆叠电压（总压）
#endif
        EN_REQUEST_READ_CURR,           // 读取电流
        EN_REQUEST_SET_CELL_BALANCING,  // 设置电芯均衡
        EN_REQUEST_DETECT_CELL_OPEN_WIRE,// 检测电芯断线
        EN_REQUEST_READ_CELL_BALANCE_STATE,// 读取均衡状态
        EN_REQUEST_LIVE_CONT_MEASURE,   // 存活计数（状态机监控）
        EN_REQUEST_FAULT_HANDLE,        // 故障处理
    },
    /* 循环请求队列长度 */
    AFE_CONTROL_MAX_REQUEST_LOOP_LEN,
};

/* static function --------------------------------------------------------------*/
// 内部函数声明（仅本文件可见）
static void _AfeCtrolDetectOpenWire(void);    // 检测电芯断线
static void _AfeCtrolStartRequest(void);      // 启动状态机请求
static void _AfeCtrolDrvInit(void);           // 初始化AFE控制驱动
static void _AfeCtrolDataInit(void);          // 初始化数据缓冲区
static void _AfeCtrolRegInit(void);           // 初始化AFE芯片寄存器
static void _AfeCtrolContinousMeas(void);     // 连续测量（电压/温度/电流）
static void _AfeCtrolReadCellVol(void);       // 读取并处理电芯电压
static void _AfeCtrolReadGPIOVol(void);       // 读取GPIO电压（转换为温度）
static void _AfeCtrolReadStackVol(void);      // 读取堆叠电压（总压）
static void _AfeCtrolReadCurr(void);          // 读取并处理电流
static void _AfeCtrolWriteCellBal(void);      // 设置电芯均衡状态
static void _AfeCtrolReadCellBal(void);       // 读取电芯均衡状态
static void _AfeCtrolCompleteRequest(void);   // 完成当前请求（状态机复位）
static void _AfeCtrolCalLiveCnt(void);        // 计算状态机存活计数
static void _AfeCtrolFltHndle(void);          // 故障处理（通信故障等）

/**
*******************************************************************************
* Function: TwosComponentS16
* @brief:    16位二进制补码转换为有符号整数
*
* @details:  处理AFE芯片采集的16位无符号补码数据，转换为实际有符号值（如负电压/电流）
*
* Parameters:
* @param [in]  input:  16位无符号补码数据（AFE采集原始值）
* @param [out] output: 转换后的32位有符号整数
*
* @return: None
*
*******************************************************************************
*/
void TwosComponentS16(uint16_t input, int32_t* output)
{
    // 判断符号位（最高位为1表示负数）
    if (input & 0x8000)
    {
        // 负数：按补码规则转换（取反+1后取负）
        *output = -(int32_t)(~(input - 1) & 0x7FFF);
    }
    else
    {
        // 正数：直接赋值
        *output = input;
    }
}

/**
*******************************************************************************
* Function: TwosComponentS32
* @brief:    32位二进制补码转换为有符号整数
*
* @details:  处理AFE芯片采集的32位无符号补码数据（如高精度电流/电压）
*
* Parameters:
* @param [in]  input:  32位无符号补码数据（AFE采集原始值）
* @param [out] output: 转换后的32位有符号整数
*
* @return: None
*
*******************************************************************************
*/
void TwosComponentS32(uint32_t input, int32_t* output)
{
    // 判断符号位（最高位为1表示负数）
    if (input & 0x80000000)
    {
        // 负数：按补码规则转换
        *output = -(int32_t)(~(input - 1) & 0x7FFFFFFF);
    }
    else
    {
        // 正数：直接赋值
        *output = input;
    }
}

/**
*******************************************************************************
* Function: _AfeCtrolDrvInit
* @brief:    初始化AFE控制驱动结构体
*
* @details:  重置状态机状态、请求队列计数器、故障标志等，为AFE工作做准备
*
* Parameters: None
*
* @return: None
*
*******************************************************************************
*/
static void _AfeCtrolDrvInit(void)
{
    /* 初始化状态机相关参数 */
    gAfeCtrolDrv.eAfeModule = EN_MODULE_IDLE;          // 模块状态：空闲
    gAfeCtrolDrv.eAfeRequest = EN_REQUEST_NO_REQ;      // 当前无请求
    gAfeCtrolDrv.eStsHandleStmcErr = STATUS_HANDLE_NO_ERR; // 无状态机错误
    gAfeCtrolDrv.FullScanSts = false;                  // 全扫描未完成
    gAfeCtrolDrv.RequestQueueLoopCounter = 0;          // 循环请求队列计数器清零
    gAfeCtrolDrv.RequestQueueStartupCounter = 0;       // 启动请求队列计数器清零
    gAfeCtrolDrv.StartupSequenceComplete = false;      // 启动序列未完成

    /* 初始化通信故障标志 */
    gIC[0].FltSts.CommFlt = false;                     // 通信故障标志清零
}

/**
*******************************************************************************
* Function:  _AfeCtrolRegInit
* @brief:    初始化AFE芯片（BQ769x2）寄存器
*
* @details:  1. 唤醒AFE芯片（TS2引脚复位）
*            2. 重置芯片寄存器（RST_SHUT引脚）
*            3. 配置寄存器参数（采样率、保护阈值等）
*            4. 禁用睡眠模式（确保持续采样）
*
* Parameters: None
*
* @return: None
*
*******************************************************************************
*/
static void _AfeCtrolRegInit(void)
{
    // 1. TS2引脚置高300ms，唤醒AFE芯片
    DevFeedExtWdg();                      // 喂外部看门狗
    GpioBitSet((&AfeTs2Handle));          // TS2引脚置高
    soft_delayms(300);                    // 延时300ms（确保芯片唤醒）
    DevFeedExtWdg();                      // 喂外部看门狗
    GpioBitReset((&AfeTs2Handle));        // TS2引脚拉低

    // 2. RST_SHUT引脚置高300ms，复位芯片寄存器
    GpioBitSet((&Bq76952WpHandle));       // RST_SHUT引脚置高
    soft_delayms(300);                    // 延时300ms（确保寄存器复位）
    DevFeedExtWdg();                      // 喂外部看门狗
    GpioBitReset((&Bq76952WpHandle));     // RST_SHUT引脚拉低

#if AFE_OTP_EN
    // 若开启OTP编程，写入校准参数（生产阶段使用）
    DevAfeOTPWrite(gDeviceAddress[0], &gIC[0]);
#endif

    soft_delayms(30);                     // 等待寄存器稳定
    DevFeedExtWdg();                      // 喂狗

    // 3. 发送RESET命令，重置BQ769x2寄存器
    gIC[0].FltSts.CommFlt = DevAfeCommandSubcommands(gDeviceAddress[0], DevAfeRESET);
    soft_delayms(30);                     // 等待复位完成

    // 4. 配置AFE芯片所有寄存器（采样模式、保护阈值等）
    DevAfeRegInit(gDeviceAddress[0], &gIC[0]);
    soft_delayms(10);                     // 等待配置生效
    DevFeedExtWdg();                      // 喂狗

    // 5. 禁用睡眠模式（Normal模式下全速采样）
    gIC[0].FltSts.CommFlt = DevAfeCommandSubcommands(gDeviceAddress[0], SLEEP_DISABLE);
    DevFeedExtWdg();                      // 喂狗
}

/**
*******************************************************************************
* Function:  _AfeCtrolCompleteRequest
* @brief:    完成当前状态机请求（复位状态机状态）
*
* @details:  将模块状态重置为空闲，当前请求重置为无请求，允许下一次请求触发
*
* Parameters: None
*
* @return: None
*
*******************************************************************************
*/
static void _AfeCtrolCompleteRequest(void)
{
    gAfeCtrolDrv.eAfeModule = EN_MODULE_IDLE;      // 模块状态：空闲
    gAfeCtrolDrv.eAfeRequest = EN_REQUEST_NO_REQ;  // 当前无请求
}

LPF2_Float_T lpf_float;
// 初始化：wn(rad/s), fs(Hz)
void LPF2_Float_Init(float wn, float fs)
{
    const float xi = 0.707f;
    float Ts = 1.0f / fs;
    float wnTs = wn * Ts;
    float wnTs2 = wnTs * wnTs;

    // 按PPT公式计算系数
    lpf_float.b0 = wnTs2;
    lpf_float.b1 = 2.0f * wnTs2;
    lpf_float.b2 = wnTs2; // 修正：PPT中b2=0为笔误，正确应与b0相同

    lpf_float.a0 = 4.0f + 4.0f * xi * wnTs + wnTs2;
    lpf_float.a1 = 2.0f * wnTs2 - 8.0f;
    lpf_float.a2 = 4.0f - 4.0f * xi * wnTs + wnTs2;

    // 清零历史值
    lpf_float.x[0] = lpf_float.x[1] = lpf_float.x[2] = 0.0f;
    lpf_float.y[0] = lpf_float.y[1] = lpf_float.y[2] = 0.0f;
}

// 滤波运行
int32_t LPF2_Float_Run(int32_t raw_mA)
{
    // 输入移位
    lpf_float.x[2] = lpf_float.x[1];
    lpf_float.x[1] = lpf_float.x[0];
    lpf_float.x[0] = (float)raw_mA;

    // 输出移位
    lpf_float.y[2] = lpf_float.y[1];
    lpf_float.y[1] = lpf_float.y[0];

    // 递推公式：y(n) = 1/a0 * [b0x(n)+b1x(n-1)+b2x(n-2) -a1y(n-1)-a2y(n-2)]
    lpf_float.y[0] = (lpf_float.b0 * lpf_float.x[0]
                      + lpf_float.b1 * lpf_float.x[1]
                      + lpf_float.b2 * lpf_float.x[2]
                      - lpf_float.a1 * lpf_float.y[1]
                      - lpf_float.a2 * lpf_float.y[2]) / lpf_float.a0;

    return (int32_t)lpf_float.y[0];
}

/**
*******************************************************************************
* Function:  AFEInit
* @brief:    AFE模块初始化（对外接口）
*
* @details:  1. 初始化I2C通信（AFE芯片通信接口）
*            2. 初始化AFE芯片寄存器
*            3. 初始化AFE控制驱动（状态机）
*
* Parameters: None
*
* @return: None
*
*******************************************************************************
*/
void AFEInit(void)
{
    float fc = 0.1f;                  // 截止频率 0.1Hz
    float wn = 2 * 3.14f * fc; // 截止角频率 rad/s
    float fs = 200.0f;                // 采样率 200Hz
    LPF2_Float_Init(wn, fs); // 浮点版
    /* 初始化I2C通信 */
    AFESampleTimeHandle.irq_process = _irq_process;  // 绑定中断处理函数
    AFETimerInit();                                   // 初始化AFE采样定时器
    DevAfeI2CInit(gDeviceAddress[0]);                 // 初始化AFE I2C驱动
    I2CBq76952BspInit();                              // 初始化BQ769x2 I2C硬件

    /* 初始化AFE芯片寄存器 */
    _AfeCtrolRegInit();

    /* 初始化AFE控制驱动（状态机） */
    _AfeCtrolDrvInit();
    gstCellInfo.cell_bal_max_time = (uint32_t)(CELL_BAL_BAT_PERCENT * CELL_CAPACITY * 1000 * 3600) * 1000 / BAL_CURRENT;
    for(uint8_t i = 0; i < CELL_STRINGS_MAX; i++)
    {
        gstCellInfo.cell_bal_flag[i] = 0;
        gstCellInfo.cell_bal_timer[i] = 0;
    }
}

/**
*******************************************************************************
* Function:  AFEProcess
* @brief:    AFE状态机处理函数（方式2：状态驱动，对外接口）
*
* @details:  根据gAfeState状态，执行对应的采集/均衡/检测操作
*            支持单独触发电压/电流/温度采集、均衡控制、断线检测
*            需在主循环中定期调用
*
* Parameters: None
*
* @return: None
*
*******************************************************************************
*/
void AFEProcess()
{
    static uint32_t voltagesample_time = 0;
    if(gFirst_AFE_Coved_Flag != 3)
    {
        return;
    }
    if(TimeDiffFromNow(voltagesample_time) > 100)
    {
        voltagesample_time = TimGetSystime();  // 更新当前时间戳
        VoltageSample();  //电压采集
        OpenWireCheck();  //断线检测
    }
    BalanceProcess();
    switch(gAfeState)
    {
        case EN_AFE_IDLE:
        {
            break;
        }
        case EN_AFE_SET_B:
        {
            // 设置电芯均衡：按分组开启均衡
            static uint8_t balance_index = 0;  //均衡分组索引（循环切换）
            uint16_t cell_bal_cmd = 0;         //均衡控制字
            //uint8_t _gBalanceTab[2][4] = {{0, 2, 4, 6}, {1, 3, 5, 7}};
            // 遍历当前分组的4个电芯
            for(uint8_t i = 0; i < 4; i ++)
            {
                uint8_t cell_idx = BAL_GROUP[balance_index][i];  // 当前电芯索引（0-7）
                uint8_t afe_bit = CELL_TO_AFE_BIT[cell_idx];    // 映射到AFE的bit位

                // 仅当电芯标记为均衡中，且压差满足条件时，置位对应bit
                if(gstCellInfo.cell_bal_flag[cell_idx] == 1)
                {
                    cell_bal_cmd |= (1 << afe_bit);
                }
            }
            // 切换均衡分组（0→1→0循环）
            balance_index = (balance_index + 1) % 2;
            // 写入均衡控制字到AFE芯片
            DevAfeHostWriteCBL(gDeviceAddress[0], cell_bal_cmd, &gIC[0]);
            gAfeState = EN_AFE_READ_B;  // 切换到读取均衡状态
            break;
        }

        case EN_AFE_READ_B:
        {
            // 读取均衡状态：更新全局均衡状态
            bool cellbalststemp = false;
            // 读取AFE芯片均衡状态
            DevAfeHostReadCBL(gDeviceAddress[0], &gIC[0]);

            // 解析均衡状态（8节电芯）
            for(uint8_t i = 0; i < CELL_STRINGS_MAX; i++)
            {
                uint8_t afe_bit = CELL_TO_AFE_BIT[i];
                cellbalststemp = (bool)((gIC[0].SampInfo.CBActiveCells >> afe_bit) & 0x01);
                if(cellbalststemp)
                {
                    gstCellInfo.arrCellBalSts[i] = true;    // 均衡中
                    gstCellInfo.balance_open_status[i] = 1;    // 均衡开启标志
                }
                else
                {
                    gstCellInfo.arrCellBalSts[i] = false;   // 未均衡
                    gstCellInfo.balance_open_status[i] = 0;    // 均衡关闭标志
                }
            }
            // 16串中其余无效通道置0（避免干扰）
            for(uint8_t i = CELL_STRINGS_MAX; i < 16; i++)
            {
                gstCellInfo.arrCellBalSts[i] = false;
                gstCellInfo.balance_open_status[i] = 0;
            }
            gAfeState = EN_AFE_IDLE;  // 回到空闲状态
            break;
        }

        case EN_AFE_GET_C:
        {
            int32_t curr = 0;
            // 触发电流采集：读取电流原始值
            gAfeCtrolDrv.FullScanSts = false;
            gIC[0].FltSts.CommFlt = false;
            // 读取告警状态，判断全扫描是否完成
            gIC[0].AlmSts.AlarmBits = DevAfeReadAlarmStatus(gDeviceAddress[0], &gIC[0].FltSts.CommFlt);
            if(gIC[0].AlmSts.AlarmBits & 0x80)  // FULLSCAN位置1
            {
                // 读取电流（若开启CC2电流采样）
#if AFE_CC2_CURRENT_EN
                DevAfeReadCurrent(gDeviceAddress[0], &gIC[0]);
#endif
                /* 清除FULLSCAN标志 */
                gIC[0].FltSts.CommFlt = DevAfeDirectCommands(gDeviceAddress[0], AlarmStatus, 0x0080, W);
                gAfeCtrolDrv.FullScanSts = true;  // 标记电流采集完成
            }
            /* 步骤1：补码转换，10mA→1mA */
            TwosComponentS16(gIC[0].SampInfo.cell_current, &curr);
            /* 步骤2：单位转换为mA（×10），并校准 */
            gstCellInfo.cell_current = curr * 10;
            // 电流校准（充电方向：负电流）
            if((curr * 10) < 0)
            {
                gstCellInfo.cell_current = -(0.91 * abs(curr * 10) + 230);
                gstCellInfo.cell_current = -(0.92 * abs(curr * 10));
            }
            // 电流校准（放电方向：正电流）
            else if((curr * 10) > 0)
            {
                gstCellInfo.cell_current = (0.91 * abs(curr * 10) - 230);
                gstCellInfo.cell_current = (0.87 * abs(curr * 10));
            }
            gAfeState = EN_AFE_IDLE;  // 回到空闲状态
            break;
        }
        default:
            break;
    }
}

// 滑窗滤波：去掉最大、最小值后取平均（最常用、最稳的滑窗算法）
int32_t sliding_window_filter(int32_t *buf, uint16_t size)
{
    int32_t sum = 0;
    int32_t max, min;

    // 找最大、最小值
    max = min = buf[0];
    for(int i = 0; i < size; i++)
    {
        sum += buf[i];
        if(buf[i] > max) max = buf[i];
        if(buf[i] < min) min = buf[i];
    }

    // 平均剩下 28 个点 → 滑窗输出
    return sum / (size);
}

int32_t RC_Current_Filter(int32_t raw_mA)
{
    static int64_t filter_buf = 0;  // 滤波“记忆”缓存（64位绝对不溢出）
    const uint16_t RC_COEF = 15;    // 平滑系数：15 = 最强平滑 + 响应快

    // 第一步：旧值衰减（保留 14/15，丢掉 1/15）
    filter_buf = filter_buf - (filter_buf / RC_COEF);

    // 第二步：加入新采样值（只加入 1/15 的影响力）
    filter_buf = filter_buf + raw_mA;

    // 第三步：输出平滑后的电流（mA）
    return (int32_t)(filter_buf / RC_COEF);
}
/**
*******************************************************************************
* Function: _irq_process
* @brief:    AFE采样中断处理函数（5ms中断一次）
*
* @details:  1. 定期触发电压/电流/温度采集
*            2. 每1000ms执行一次安时积分（SOC计算）
*            3. 喂狗防止系统复位
*
* Parameters: None
*
* @return: None
*
*******************************************************************************
*/
static void _irq_process(void)
{
    static uint16_t countup_5ms = 0;        // 5ms计数
    static uint8_t soc_ampere_flag = 0;     // 安时积分标志
    static int32_t current_window_buf[CURRENT_WINDOW_SIZE] = {0}; // 滑窗缓冲区
    static uint16_t current_window_index = 0;  // 当前写入位置索引
    static uint8_t current_window_full_flag = 0; // 窗口是否已满标记
    DevFeedExtWdg();  // 喂外部看门狗
    // ===================== 【核心】均衡计时器更新 =====================
    for(uint8_t i = 0; i < CELL_STRINGS_MAX; i++)
    {
        // 仅当均衡标记=1 且 实际开启均衡 → 计时
        if(gstCellInfo.cell_bal_flag[i] == 1 && gstCellInfo.arrCellBalSts[i] == true)
        {
            if(gstCellInfo.cell_bal_timer[i] < gstCellInfo.cell_bal_max_time)
            {
                gstCellInfo.cell_bal_timer[i] += 5;
            }
            else
            {
                // 达到均衡时间 → 停止
                gstCellInfo.cell_bal_flag[i] = 0;
                gstCellInfo.cell_bal_timer[i] = 0;
            }
        }
    }

    // AFE首次采集完成（就绪状态）
    if(gFirst_AFE_Coved_Flag == 3)
    {
        /* 触发电流采集 */
        if(countup_5ms % 6 == 0)
        {
            if(countup_5ms == 180)
            {
                countup_5ms = 0;
                soc_ampere_flag = 1;  // 标记安时积分就绪
            }
            CurrentGet();  // 触发电流采集
            current_window_buf[current_window_index] = gstCellInfo.cell_current;
            current_window_index++;
            if(current_window_index >= CURRENT_WINDOW_SIZE)
            {
                current_window_index = 0;
                current_window_full_flag = 1; // 标记窗口已满，后续可以开始用滑窗数据
            }
            if(current_window_full_flag)
            {
                // 输出：滑窗滤波结果
                gPackStatus.pack_current_uncalibrated = sliding_window_filter(current_window_buf, CURRENT_WINDOW_SIZE);
                gPackStatus.pack_current_filter = RC_Current_Filter(gPackStatus.pack_current_uncalibrated);
//                gPackStatus.pack_current = gPackStatus.pack_current_calibration_factor * gPackStatus.pack_current_filter;
                gPackStatus.pack_current = LPF2_Float_Run(gPackStatus.pack_current_filter);
            }
            else
            {
                // 未满25点，先用原始值
                gPackStatus.pack_current = gstCellInfo.cell_current;
            }
        }
        /* 安时积分就绪：计算SOC */
        if(soc_ampere_flag)
        {
            soc_ampere_flag = 0;
            if(gSoxDebugParam.sox_debug_mode == 0)
            {
                // 正常模式：计算平均电流，应用校准系数
                SOCAmpereHourIntegral(900, &gPackStatus);
            }
            else if(gSoxDebugParam.sox_debug_mode == 1)
            {
                // 调试模式1：使用原始电流，调试SOC
                gPackStatus_debug.pack_current = gSoxDebugParam.cell_current_debug;
                SOCAmpereHourIntegral(900, &gPackStatus_debug);
            }
            else if(gSoxDebugParam.sox_debug_mode == 2)
            {
                // 调试模式2：双SOC计算（正常+调试）
                gPackStatus_debug.pack_current = -gSoxDebugParam.cell_current_debug;
                SOCAmpereHourIntegral(900, &gPackStatus_debug);
            }
            countup_5ms = 0;
        }
        // 4ms计数器累加
        if(countup_5ms < 180)
        {
            countup_5ms++;
        }
        else
        {
            countup_5ms = 0;
        }
    }
}

void OpenWireCheck()
{
    if(gSoxDebugParam.sox_debug_mode == 0)
    {
        bool open_wire = false;
        gIC[0].AlmSts.AlarmBits = DevAfeReadAlarmStatus(gDeviceAddress[0], &gIC[0].FltSts.CommFlt);
        // 安全状态标志置1：存在OV/UV故障（可能为断线）
        if(gIC[0].AlmSts.AlarmBits & 0xC000)
        {
            // 读取安全状态（确认是否触发了UV/OV硬件故障）
            DevAfeReadSafetyStatus(gDeviceAddress[0], &gIC[0]);
            for(uint8_t i = 0; i < 8; i++)
            {
                if(((gstCellInfo.cell_voltage[i] < 1000) || (gstCellInfo.cell_voltage[i] > 4000)) && ((gIC[0].FltSts.UVFault) || (gIC[0].FltSts.OVFault)))
                {
                    gstCellInfo.CellOpenWire[i] = true;
                    open_wire = true;                  // 标记整体有断线
                }
                else
                {
                    gstCellInfo.CellOpenWire[i] = false; // 无断线则清除标志
                }
            }
            // 如果检测到任意一节断线，更新错误码
            if(open_wire)
            {
                gBmsErrorInfo.cellsample_error_code.b.cell_vol_sample_miss = 1;
                gPackErrorInfo.cell1_error_code.b.cell_vol_sample_miss = 1;
            }
        }
    }
}

void VoltageSample()
{
    // 触发电压采集：读取电芯电压和总压
    gAfeCtrolDrv.FullScanSts = false;
    gIC[0].FltSts.CommFlt = false;

    // 读取告警状态，判断全扫描是否完成
    gIC[0].AlmSts.AlarmBits = DevAfeReadAlarmStatus(gDeviceAddress[0], &gIC[0].FltSts.CommFlt);
    if(gIC[0].AlmSts.AlarmBits & 0x80)  // FULLSCAN位置1
    {
        /* 读取所有电芯电压 */
        DevAfeReadCellVoltages(gDeviceAddress[0], &gIC[0]);
        /* 清除FULLSCAN标志 */
        gIC[0].FltSts.CommFlt = DevAfeDirectCommands(gDeviceAddress[0], AlarmStatus, 0x0080, W);
        gAfeCtrolDrv.FullScanSts = true;  // 标记电压采集完成
    }

    // 电压处理状态：计算电压参数，判断是否需要均衡
    if(gAfeCtrolDrv.FullScanSts == true)
    {
        uint8_t j = 0;
        uint16_t cell_index = VCELL_MODE_CFG;
        uint8_t cell_num = 8;

        /* 步骤1：补码转换 */
        for(uint8_t i = 0; i < CELL; i++)
        {
            if(gIC[0].SampInfo.cell_voltage[i] & 0x8000)
            {
                gIC[0].SampInfo.cell_voltage[i] = ((~gIC[0].SampInfo.cell_voltage[i]) & 0x7FFF) * -1U;
            }
            else
            {
                gIC[0].SampInfo.cell_voltage[i] = gIC[0].SampInfo.cell_voltage[i];
            }
        }

        /* 步骤2：计算电压参数 */
        gstCellInfo.total_cell_voltage = 0;
        gstCellInfo.cell_max_voltage = gIC[0].SampInfo.cell_voltage[0];
        gstCellInfo.cell_min_voltage = gIC[0].SampInfo.cell_voltage[0];

        for(uint8_t i = 0; i < CELL; i++)
        {
            if(cell_index & 0x01)
            {
                gstCellInfo.cell_voltage[j] = gIC[0].SampInfo.cell_voltage[i];
                gstCellInfo.total_cell_voltage += gIC[0].SampInfo.cell_voltage[i];

                // 更新最大/最小电压
                if(gstCellInfo.cell_voltage[j] > gstCellInfo.cell_max_voltage)
                {
                    gstCellInfo.cell_max_voltage = gstCellInfo.cell_voltage[j];
                }
                if(gstCellInfo.cell_voltage[j] < gstCellInfo.cell_min_voltage)
                {
                    gstCellInfo.cell_min_voltage = gstCellInfo.cell_voltage[j];
                }
                j++;
            }
            cell_index >>= 1;
        }
        // 单位转换：总电压/100（原始单位：0.01mV → mV）
        gstCellInfo.total_cell_voltage = gstCellInfo.total_cell_voltage / 10;
        gstCellInfo.cell_diff_voltage = gstCellInfo.cell_max_voltage - gstCellInfo.cell_min_voltage;
        gstCellInfo.cell_ave_voltage = (gstCellInfo.total_cell_voltage * 10) / cell_num;  // 平均电压（单位：0.01mV）
    }

    // 调试模式：强制所有电芯电压为3300mV（0.01mV单位）
    if(gSoxDebugParam.sox_debug_mode)
    {
        for(uint8_t i = 0; i < 16; i++)
        {
            gstCellInfo.cell_voltage[i] = 3300;
        }
        gstCellInfo.cell_max_voltage = 3300;
        gstCellInfo.cell_min_voltage = 3300;
        gstCellInfo.cell_ave_voltage = 3300;
        gstCellInfo.cell_diff_voltage = 0;
        gstCellInfo.total_cell_voltage = 2660;  // 3300 * 8 / 100 = 264 → 此处为调试值2660
    }
}

void BalanceProcess()
{
    static uint32_t balance_time = 0;
    if(TimeDiffFromNow(balance_time) > 5000)
    {
        balance_time = TimGetSystime();
        // ===================== 【流程图核心】弱势电芯复位 =====================
        for(uint8_t i = 0; i < CELL_STRINGS_MAX; i++)
        {
            if((gstCellInfo.cell_bal_flag[i] == 1) && (gstCellInfo.cell_voltage[i] == gstCellInfo.cell_min_voltage))
            {
                gstCellInfo.cell_bal_flag[i] = 0;
                gstCellInfo.cell_bal_timer[i] = 0;
            }
        }

        // 充电或者待机模式下，压差>20mV且无电芯错误、最小电压>3100mV：触发均衡
        if(((gPackStatus.mode == EN_PACK_CURRENT_CHARGE_MODE) || ((gPackStatus.mode == EN_PACK_CURRENT_IDLE_MODE))) && (gstCellInfo.cell_diff_voltage > CELL_BAL_DIFF_THRESH) && \
                (gBmsErrorInfo.cell1_error_code.value == 0) && (gstCellInfo.cell_min_voltage >= 3100))
        {
            // 标记需要均衡的电芯
            for(uint8_t i = 0; i < CELL_STRINGS_MAX; i++)
            {
                if(gstCellInfo.cell_voltage[i] - gstCellInfo.cell_min_voltage > CELL_BAL_DIFF_THRESH)
                {
                    if(gstCellInfo.cell_bal_flag[i] == 0)
                    {
                        gstCellInfo.cell_bal_flag[i] = 1;
                    }
                }
                else
                {
                    gstCellInfo.cell_bal_flag[i] = 0;
                    gstCellInfo.cell_bal_timer[i] = 0;
                }
            }
            SetBalance();
        }
    }
}
bool AFEFirstSample(void)
{
    static uint16_t filter_count = 0;
    static uint32_t max_vol = 0;
    static uint32_t min_vol = 0;
    static uint32_t ave_vol = 0;
    static uint16_t max_cur_ref = 0;
    static uint16_t min_cur_ref = 0;
    static uint16_t ave_cur_ref = 0;

    /* 确保AFE第一次采集完数据后再进入故障解析 */
    if(gFirst_AFE_Coved_Flag > 2)
    {
        return true;
    }
    if(gFirst_AFE_Coved_Flag == 0)
    {
        VoltageSample();
        if(filter_count == 0)
        {
            max_vol = gstCellInfo.total_cell_voltage;
            min_vol = gstCellInfo.total_cell_voltage;
        }
        if(gstCellInfo.total_cell_voltage > max_vol)
        {
            max_vol = gstCellInfo.total_cell_voltage;
        }
        if(gstCellInfo.total_cell_voltage < min_vol)
        {
            min_vol = gstCellInfo.total_cell_voltage;
        }
        ave_vol += gstCellInfo.total_cell_voltage;
        filter_count ++;
        if(filter_count > 100)
        {
            gFirst_AFE_Coved_Flag = 1;
            filter_count = 0;
            gstCellInfo.bat_init_vol = (ave_vol - min_vol - max_vol) / 4;
        }
    }
    if(gFirst_AFE_Coved_Flag == 1)
    {

        CurrentGet();
        if(filter_count == 0)
        {
            max_cur_ref = gstCellInfo.cur_ref;
            min_cur_ref = gstCellInfo.cur_ref;
        }

        if(gstCellInfo.cur_ref > max_cur_ref)
        {
            max_cur_ref = gstCellInfo.cur_ref;
        }

        if(gstCellInfo.cur_ref < min_cur_ref)
        {
            min_cur_ref = gstCellInfo.cur_ref;
        }
        ave_cur_ref += gstCellInfo.cur_ref;
        filter_count ++;

        if(filter_count > 100)
        {
            gFirst_AFE_Coved_Flag = 2;
        }
    }
    if(gFirst_AFE_Coved_Flag == 2)
    {
        return true;
    }
    return false;
}

bool SetBalance(void)
{
    if(gAfeState != EN_AFE_IDLE)
    {
        LOG_DEBUG(LOG_TAG, "AFE is Busy!!! SetBalance miss!!!\r\n");
        return false;
    }
    else
    {
        gAfeState = EN_AFE_SET_B;
    }
    return true;
}

bool CurrentGet(void)
{
    if(gAfeState != EN_AFE_IDLE)
    {
        LOG_DEBUG(LOG_TAG, "AFE is Busy!!! CurrentGet miss!!!\r\n");
        return false;
    }
    else
    {
        gAfeState = EN_AFE_GET_C;
    }
    return true;
}

void StopAFESample(void)
{
    gFirst_AFE_Coved_Flag = 4;
}

void StartAFESample(void)
{
    gFirst_AFE_Coved_Flag = 3;
}


