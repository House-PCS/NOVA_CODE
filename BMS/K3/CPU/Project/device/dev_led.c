/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_led.c
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
#include "dev_led.h"

/**************************************************************************
 * 宏定义：LED相关时间常量
 **************************************************************************/
#define  LED_TIMES         1000    // 普通LED默认时间基数（单位：ms，根据系统时钟配置）
#define  TEST_LED_TIMES    3000    // 测试模式下LED时间基数（单位：ms，预留测试场景使用）

/**************************************************************************
 * 全局变量定义
 **************************************************************************/
// LED设备信息数组：存储每个LED的硬件模式、激活电平、GPIO句柄、工作状态等配置
// LED_SRC_NUM为LED源数量枚举（来自dev_led.h），每个元素对应一个物理LED
LedInfo_t   gLedInfo[LED_SRC_NUM];

// GPIO句柄数组：存储每个LED对应的GPIO驱动句柄（静态变量，仅本文件可见）
static Gpio_HandleType          *pGpioHandle[LED_SRC_NUM];

// 定时器句柄数组：存储每个LED对应的定时器驱动句柄（静态变量，仅本文件可见，PWM模式预留）
static Timer_HandleType         *pTimerHandle[LED_SRC_NUM];

/**************************************************************************
 * 函数名称：LedOFF
 * 函数功能：关闭指定LED
 * 输入参数：index - LED源索引（EN_LED_SRC枚举类型，如LED_SRC_SYS、LED_SRC_PWR）
 * 逻辑说明：根据LED的硬件模式（GPIO/PWM）和激活电平（高/低）执行对应关闭操作
 **************************************************************************/
void LedOFF(EN_LED_SRC index)
{
    // 判断LED硬件模式是否为GPIO模式（当前仅实现GPIO模式，PWM模式预留）
    if(gLedInfo[index].led_hw_mode == LED_HW_GPIO)
    {
        // 若LED为高电平激活：关闭时需拉低GPIO引脚
        if(gLedInfo[index].led_active_level == LED_HW_ACTIVE_HIGH)
        {
            GpioBitReset(gLedInfo[index].p_gpio_Handle); // GPIO引脚拉低
        }
        // 若LED为低电平激活：关闭时需拉高GPIO引脚（反向逻辑）
        else
        {
            GpioBitSet(gLedInfo[index].p_gpio_Handle); // GPIO引脚拉高
        }
    }
    // PWM模式关闭逻辑预留（如需PWM调光，此处需添加停止PWM输出代码）
    else if(gLedInfo[index].led_hw_mode == LED_HW_PWM)
    {
    }
}

/**************************************************************************
 * 函数名称：LedON
 * 函数功能：开启指定LED
 * 输入参数：index - LED源索引（EN_LED_SRC枚举类型）
 * 逻辑说明：根据LED的硬件模式和激活电平执行对应开启操作，与LedOFF逻辑相反
 **************************************************************************/
void LedON(EN_LED_SRC index)
{
    // 判断LED硬件模式是否为GPIO模式
    if(gLedInfo[index].led_hw_mode == LED_HW_GPIO)
    {
        // 若LED为高电平激活：开启时需拉高GPIO引脚
        if(gLedInfo[index].led_active_level == LED_HW_ACTIVE_HIGH)
        {
            GpioBitSet(gLedInfo[index].p_gpio_Handle); // GPIO引脚拉高
        }
        // 若LED为低电平激活：开启时需拉低GPIO引脚（反向逻辑）
        else
        {
            GpioBitReset(gLedInfo[index].p_gpio_Handle); // GPIO引脚拉低
        }
    }
    // PWM模式开启逻辑预留（如需PWM调光，此处需添加PWM占空比配置代码）
    else if(gLedInfo[index].led_hw_mode == LED_HW_PWM)
    {
    }
}

/**************************************************************************
 * 函数名称：LedBlink
 * 函数功能：LED闪烁控制（需周期性调用，建议在主循环或定时器中断中执行）
 * 输入参数：index - LED源索引；first_flag - 初始化标志（1=首次调用，需初始化计数）
 * 局部变量：
 *   led_flag[] - 闪烁状态标志（0=熄灭状态，1=点亮状态，静态变量保持状态）
 *   timecount[] - 时间戳记录（记录上一次状态切换时间，静态变量不重置）
 *   period - 闪烁周期（取自LED配置，测试模式下强制为100ms）
 * 逻辑说明：通过时间差判断是否切换LED状态，实现固定周期闪烁
 **************************************************************************/
void LedBlink(EN_LED_SRC index, uint8_t first_flag)
{
    static uint32_t led_flag[LED_SRC_NUM];    // 每个LED的闪烁状态标志（静态变量，跨调用保持）
    static uint32_t timecount[LED_SRC_NUM];   // 每个LED的时间戳记录（静态变量，记录上次切换时间）
    uint32_t period = gLedInfo[index].led_status_period; // 读取配置的闪烁周期

    // 测试模式判断（gtest为全局测试标志，需外部定义）：测试模式下强制闪烁周期为100ms
    if(gtest)
    {
        period = 100;
    }

    // first_flag=1时初始化：重置状态标志和时间戳，初始状态为熄灭
    if(first_flag == 1)/*计数初始化*/
    {
        led_flag[index] = 0;                      // 初始状态：熄灭
        timecount[index] = TimGetSystime();       // 记录当前系统时间（TimGetSystime返回系统运行时间ms）
        LedOFF(index);                            // 初始化时关闭LED
        return;
    }

    // 时间差 >= 周期的1/2：从熄灭状态切换到点亮状态
    if((uint32_t)(TimGetSystime() - timecount[index]) >= period / 2)
    {
        if(led_flag[index] == 0)
        {
            LedON(index);         // 点亮LED
            led_flag[index] = 1;  // 更新状态标志为点亮
        }
    }

    // 时间差 >= 完整周期：从点亮状态切换到熄灭状态，重置时间戳
    if((uint32_t)(TimGetSystime() - timecount[index]) >= period)
    {
        timecount[index] = TimGetSystime();       // 重置时间戳，开始下一个周期
        if(led_flag[index] == 1)
        {
            LedOFF(index);        // 熄灭LED
            led_flag[index] = 0;  // 更新状态标志为熄灭
        }
    }
}

/**************************************************************************
 * 函数名称：LedCtlBlink
 * 函数功能：配置指定LED为闪烁模式，并设置闪烁周期
 * 输入参数：src - LED源索引；period - 闪烁周期（单位：ms）
 * 逻辑说明：通过修改全局LED信息数组的状态和周期参数，间接控制LedBlink函数行为
 **************************************************************************/
void LedCtlBlink(EN_LED_SRC src, uint32_t period)
{
    gLedInfo[src].led_status = LED_STATUS_BLINK;    // 设置LED工作状态为闪烁模式
    gLedInfo[src].led_status_period = period;       // 配置闪烁周期
}

/**************************************************************************
 * 函数名称：LedInit
 * 函数功能：LED模块初始化（系统启动时调用，初始化硬件配置和工作模式）
 * 初始化内容：
 *   1. 系统LED（LED_SRC_SYS）：GPIO模式、低电平激活、1000ms闪烁
 *   2. 电源LED（LED_SRC_PWR）：GPIO模式、高电平激活、常亮模式
 *   3. 硬件底层初始化：GPIO引脚配置、初始电平设置
 **************************************************************************/
void LedInit(void)
{
    // -------------------------- 系统LED（MCU状态指示灯）配置 --------------------------
    gLedInfo[LED_SRC_SYS].led_hw_mode = LED_HW_GPIO;         // 硬件模式：GPIO控制
    gLedInfo[LED_SRC_SYS].led_active_level = LED_HW_ACTIVE_LOW; // 激活电平：低电平有效（拉低点亮）
    gLedInfo[LED_SRC_SYS].p_gpio_Handle = &MCU_LED_Handle;   // 绑定MCU LED的GPIO句柄（硬件抽象层定义）
    LedCtlBlink(LED_SRC_SYS, 1000);                          // 配置为1000ms周期闪烁（系统运行状态指示）

    // -------------------------- 电源LED（供电状态指示灯）配置 --------------------------
    gLedInfo[LED_SRC_PWR].led_hw_mode = LED_HW_GPIO;         // 硬件模式：GPIO控制
    gLedInfo[LED_SRC_PWR].led_status = LED_STATUS_ON;        // 工作状态：常亮（电源正常指示）
    gLedInfo[LED_SRC_PWR].led_active_level = LED_HW_ACTIVE_HIGH; // 激活电平：高电平有效（拉高点亮）
    gLedInfo[LED_SRC_PWR].p_gpio_Handle = &KEY_LED_EN_Handle;// 绑定电源LED的GPIO句柄（硬件抽象层定义）

    // -------------------------- 硬件底层初始化 --------------------------
    KeyLEDBspInit();       // 按键LED底层初始化（配置GPIO引脚方向、复用功能等）
    MCULEDBspInit();       // MCU LED底层初始化（配置GPIO引脚电气属性）
    GpioBitSet((&KEY_LED_EN_Handle)); // 电源LED初始电平：拉高（因高电平激活，初始常亮）
}