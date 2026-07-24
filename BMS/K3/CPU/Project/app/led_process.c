/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        led_process.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-6-1
  * @brief       LED设备状态处理核心文件，负责解析LED状态指令并调用底层驱动执行对应操作
  *              支持LED的关闭、开启、闪烁三种工作模式，通过循环扫描状态数组实现多LED设备管理
  ******************************************************************************
  * @attention
  * 1. 依赖底层驱动文件 dev_led.h 提供的硬件操作接口（LedON/LedOFF/LedBlink）
  * 2. 依赖系统信息文件 sys_info.h 定义的全局LED状态结构体 gLedInfo
  * 3. 需确保 LED_SRC_NUM 宏定义与实际硬件LED数量一致，避免数组越界
  * @relation
  * @par :
  * <table>
  * <tr><<th>Date          <<th>Version   <<th>Author      <<th>Description
  * <tr><td>2025-6-1    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "led_process.h"
//#include "sys_info.h"    // 包含全局系统信息定义（如gLedInfo结构体）

/**
 * @brief  LED闪烁模式下的基础时间参数（单位：ms）
 * @note   实际闪烁周期需结合底层驱动实现，此宏为预留配置接口
 */
#define  LED_TIMES      1000

/**
 * @brief  日志打印标签，用于区分LED模块的调试信息
 */
#define LOG_TAG     "led_process"

/**
 * @brief  全局LED状态变量（当前未使用，预留用于整体LED状态管理）
 * @note   初始值设为LED_STATUS_OFF（LED关闭状态）
 */
EN_LED_STATUS   led_status = LED_STATUS_OFF;

/**
 * @brief  LED状态处理主函数，循环扫描所有LED设备并执行对应操作
 * @details 函数逻辑：
 *          1. 遍历所有LED设备（索引0~LED_SRC_NUM-1）
 *          2. 根据每个LED设备的当前状态（gLedInfo[i].led_status）执行对应操作
 *          3. 处理完成后更新状态为IDLE，避免重复执行；闪烁模式特殊处理状态记忆
 * @param  无
 * @return 无
 */
void LedProcess(void)
{
    uint8_t i;  // 循环索引，用于遍历所有LED设备
    
    /**
     * @brief  静态数组，存储每个LED设备上一次的闪烁状态
     * @note   静态变量初始化后持续保留值，用于闪烁模式的状态切换判断
     *         数组长度与LED设备数量（LED_SRC_NUM）一致
     */
    static EN_LED_STATUS pre_status[LED_SRC_NUM];
    
    // 遍历所有LED设备，依次处理每个设备的状态
    for(i = 0; i < LED_SRC_NUM; i++)
    {
        // 根据当前LED设备的状态执行对应分支逻辑
        switch(gLedInfo[i].led_status)
        {
            case LED_STATUS_IDLE:  // LED空闲状态（无操作）
            {
                // 无需执行硬件操作，直接跳出分支
                break;
            }
            case LED_STATUS_OFF:  // LED关闭状态指令
            {
                LedOFF((EN_LED_SRC)i);  // 调用底层驱动关闭第i个LED
                gLedInfo[i].led_status = LED_STATUS_IDLE;  // 更新状态为空闲，避免重复执行
                break;
            }
            case LED_STATUS_ON:  // LED开启状态指令
            {
                LedON((EN_LED_SRC)i);  // 调用底层驱动开启第i个LED
                gLedInfo[i].led_status = LED_STATUS_IDLE;  // 更新状态为空闲，避免重复执行
                break;
            }
            case LED_STATUS_BLINK:  // LED闪烁状态指令
            {
                // 首次进入闪烁模式（上一次状态不是闪烁）
                if(pre_status[i] != LED_STATUS_BLINK)
                {
                    // 调用底层驱动启动闪烁（第二个参数1表示启动闪烁）
                    LedBlink((EN_LED_SRC)i, 1);
                    pre_status[i] = LED_STATUS_BLINK;  // 记录当前状态为闪烁，用于下次判断
                }
                else  // 已处于闪烁模式（上一次状态是闪烁）
                {
                    // 调用底层驱动维持/切换闪烁状态（第二个参数0表示持续闪烁）
                    LedBlink((EN_LED_SRC)i, 0);
                }
                break;
            }
            default:  // 未知状态（异常处理）
                break;
        }
    }
}