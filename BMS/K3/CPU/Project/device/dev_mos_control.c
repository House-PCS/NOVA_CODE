/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_mos_control.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-6-1
  * @brief       电源管理系统MOS管控制驱动模块
  *              负责充电MOS、放电MOS、预充电MOS的开关控制及状态查询
  *              包含单MOS操作和全MOS批量操作接口，同时更新故障分析标志位
  ******************************************************************************
  * @attention
  * 1. 依赖GPIO驱动模块（提供GpioBitSet/GpioBitReset等底层接口）
  * 2. 依赖故障分析模块（gFaultAnalysisFlag全局标志位定义）
  * 3. MOS管控制引脚通过xxxEnHandle句柄关联（需在dev_mos_control.h中配置）
  * 4. 所有操作均包含状态判断，避免重复开关导致硬件异常
  * @relation
  * @par :
  * <table>
  * <tr><<th>Date          <<th>Version   <<th>Author      <<th>Description
  * <tr><td>2025-6-1    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "dev_mos_control.h"  // MOS管硬件配置头文件（包含GPIO句柄定义）
#include "fault_analysis.h"  // 故障分析模块头文件（包含故障标志位结构体定义）
#define LOG_TAG "dev_charge_control"  // 日志标签，用于区分模块日志输出

/**
 * @brief  全局测试变量
 * @note   仅用于调试，在关闭放电MOS时置1（实际项目建议移除或改用调试宏）
 */
uint8_t gtest = 0;

/**
 * @brief  打开充电MOS管
 * @para   None
 * @note   1. 先判断MOS管当前状态，避免重复打开
 *         2. 通过GPIO置位操作控制MOS管导通
 *         3. 更新故障分析标志位（充电MOS使能状态）
 * @return None
 */
void OpenChargeMos(void)
{
    if(ChargeMosIsOpen())
    {
        LOG_DEBUG(LOG_TAG, "ChargeMos Was Opened\r\n");  // 已打开，输出日志提示
    }
    else
    {
        GpioBitSet((&CmosEnHandle));  // 置位充电MOS控制引脚（导通MOS管）
        LOG_DEBUG(LOG_TAG, "OpenChargeMos\r\n");  // 输出打开成功日志
    }
    gFaultAnalysisFlag.chargemos_enable_flag = 1;  // 标记充电MOS已使能
}

/**
 * @brief  关闭充电MOS管
 * @para   None
 * @note   1. 先判断MOS管当前状态，避免重复关闭
 *         2. 通过GPIO复位操作控制MOS管关断
 *         3. 更新故障分析标志位（充电MOS禁用状态）
 * @return None
 */
void CloseChargeMos(void)
{
    if(ChargeMosIsOpen())
    {
        GpioBitReset((&CmosEnHandle));  // 复位充电MOS控制引脚（关断MOS管）
        LOG_DEBUG(LOG_TAG, "CloseChargeMos\r\n");  // 输出关闭成功日志
    }
    else
    {
        LOG_DEBUG(LOG_TAG, "ChargeMos Was Closed\r\n");  // 已关闭，输出日志提示
    }
    gFaultAnalysisFlag.chargemos_enable_flag = 0;  // 标记充电MOS已禁用
}

/**
 * @brief  查询充电MOS管当前状态
 * @para   None
 * @note   通过读取GPIO输出电平判断MOS管状态（SET=导通，RESET=关断）
 * @return SET: 充电MOS已打开（导通）   RESET: 充电MOS已关闭（关断）
 */
bool ChargeMosIsOpen(void)
{
    if(GpioOutputBitGet((&CmosEnHandle)) == SET)  // 读取充电MOS控制引脚电平
    {
        return true;  // 电平为SET，返回已打开
    }
    else
    {
        return false;  // 电平为RESET，返回已关闭
    }
}

/**
 * @brief  查询放电MOS管当前状态
 * @para   None
 * @note   通过读取GPIO输出电平判断MOS管状态（SET=导通，RESET=关断）
 * @return SET: 放电MOS已打开（导通）   RESET: 放电MOS已关闭（关断）
 */
bool DisChargeMosIsOpen(void)
{
    if(GpioOutputBitGet((&DmosEnHandle)) == SET)  // 读取放电MOS控制引脚电平
    {
        return true;  // 电平为SET，返回已打开
    }
    else
    {
        return false;  // 电平为RESET，返回已关闭
    }
}

/**
 * @brief  打开放电MOS管
 * @para   None
 * @note   1. 先判断MOS管当前状态，避免重复打开
 *         2. 通过GPIO置位操作控制MOS管导通
 *         3. 更新故障分析标志位（放电MOS使能状态）
 * @return None
 */
void OpenDisChargeMos(void)
{
    if(DisChargeMosIsOpen())
    {
        LOG_DEBUG(LOG_TAG, "DisChargeMos Was Opened\r\n");  // 已打开，输出日志提示
    }
    else
    {
        GpioBitSet((&DmosEnHandle));  // 置位放电MOS控制引脚（导通MOS管）
        LOG_DEBUG(LOG_TAG, "OpenDisChargeMos\r\n");  // 输出打开成功日志
    }
    gFaultAnalysisFlag.dischargemos_enable_flag = 1;  // 标记放电MOS已使能
}

/**
 * @brief  关闭放电MOS管
 * @para   None
 * @note   1. 先判断MOS管当前状态，避免重复关闭
 *         2. 通过GPIO复位操作控制MOS管关断
 *         3. 更新故障分析标志位（放电MOS禁用状态）
 *         4. 调试用：设置全局测试变量gtest为1（实际项目建议移除）
 * @return None
 */
void CloseDisChargeMos(void)
{
    if(DisChargeMosIsOpen())
    {
        GpioBitReset((&DmosEnHandle));  // 复位放电MOS控制引脚（关断MOS管）
        LOG_DEBUG(LOG_TAG, "CloseDisChargeMos\r\n");  // 输出关闭成功日志
    }
    else
    {
        LOG_DEBUG(LOG_TAG, "DisChargeMos Was Closed\r\n");  // 已关闭，输出日志提示
    }
    gtest = 1;  // 调试标记：放电MOS关闭时置1
    gFaultAnalysisFlag.dischargemos_enable_flag = 0;  // 标记放电MOS已禁用
}

/**
 * @brief  查询预充电MOS管当前状态
 * @para   None
 * @note   1. 预充电MOS用于电源启动时的缓启动（避免大电流冲击）
 *         2. 通过读取GPIO输出电平判断MOS管状态（SET=导通，RESET=关断）
 * @return SET: 预充电MOS已打开（导通）   RESET: 预充电MOS已关闭（关断）
 */
bool PreChargeMosIsOpen(void)
{
    if(GpioOutputBitGet((&PmosEnHandle)) == SET)  // 读取预充电MOS控制引脚电平
    {
        return true;  // 电平为SET，返回已打开
    }
    else
    {
        return false;  // 电平为RESET，返回已关闭
    }
}

/**
 * @brief  打开预充电MOS管
 * @para   None
 * @note   1. 预充电MOS用于电源启动阶段，通常需配合充电MOS使用
 *         2. 先判断MOS管当前状态，避免重复打开
 *         3. 通过GPIO置位操作控制MOS管导通
 *         4. 更新故障分析标志位（预充电MOS使能状态）
 * @return None
 */
void OpenPreChargeMos(void)
{
    if(PreChargeMosIsOpen())
    {
        LOG_DEBUG(LOG_TAG, "PreChargeMos Was Opened\r\n");  // 已打开，输出日志提示
    }
    else
    {
        GpioBitSet((&PmosEnHandle));  // 置位预充电MOS控制引脚（导通MOS管）
        LOG_DEBUG(LOG_TAG, "OpenPreChargeMos\r\n");  // 输出打开成功日志
    }
    gFaultAnalysisFlag.prechargemos_enable_flag = 1;  // 标记预充电MOS已使能
}

/**
 * @brief  关闭预充电MOS管
 * @para   None
 * @note   1. 预充电完成后需关闭（避免长期导通损耗）
 *         2. 先判断MOS管当前状态，避免重复关闭
 *         3. 通过GPIO复位操作控制MOS管关断
 *         4. 更新故障分析标志位（预充电MOS禁用状态）
 * @return None
 */
void ClosePreChargeMos(void)
{
    if(PreChargeMosIsOpen())
    {
        GpioBitReset((&PmosEnHandle));  // 复位预充电MOS控制引脚（关断MOS管）
        LOG_DEBUG(LOG_TAG, "ClosePreChargeMos\r\n");  // 输出关闭成功日志
    }
    else
    {
        LOG_DEBUG(LOG_TAG, "PreChargeMos Was Closed\r\n");  // 已关闭，输出日志提示
    }
    gFaultAnalysisFlag.prechargemos_enable_flag = 0;  // 标记预充电MOS已禁用
}

/**
 * @brief  打开所有MOS管（充电+放电+预充电）
 * @para   None
 * @note   1. 批量操作接口，按固定顺序打开所有MOS管
 *         2. 内部调用单个MOS管的打开函数，自动处理状态判断和日志输出
 * @return None
 */
void OpenAllMOS(void)
{
    OpenChargeMos();     // 打开充电MOS
    OpenDisChargeMos();  // 打开放电MOS
    OpenPreChargeMos();  // 打开预充电MOS
}

/**
 * @brief  关闭所有MOS管（充电+放电+预充电）
 * @para   None
 * @note   1. 批量操作接口，按固定顺序关闭所有MOS管
 *         2. 内部调用单个MOS管的关闭函数，自动处理状态判断和日志输出
 *         3. 适用于系统关机、故障保护等场景
 * @return None
 */
void CloseAllMOS(void)
{
    CloseChargeMos();     // 关闭充电MOS
    CloseDisChargeMos();  // 关闭放电MOS
    ClosePreChargeMos();  // 关闭预充电MOS
}