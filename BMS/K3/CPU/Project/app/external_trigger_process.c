/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        external_trigger_process.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-6-1
  * @brief       
  ******************************************************************************
  * @relation
  * @par :
  * <table>
  * <tr><<th>Date          <<th>Version   <<th>Author      <<th>Description
  * <tr><td>2025-6-1        <td>V1.0.0    <td>              <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "external_trigger_process.h"
#define LOG_TAG     "external_trigger_process"  // 日志标签，用于调试输出标识

/**
 * @brief  键值处理总函数
 * @details 统一调度关机相关处理逻辑，作为外部触发的入口函数
 */
void KeyValueProcess()
{
    // 调用关机流程处理函数（核心业务逻辑）
    ShutDownProcess();
}

/**
 * @brief  BMS关机流程处理核心函数
 * @details 负责处理按键消息、GPIO状态检测、电源状态机切换，
 *          包含关机前数据保存、状态迁移等完整关机逻辑
 */
void ShutDownProcess(void)
{
    static uint8_t _gKeyPress = 0;              // 按键按下状态标志位（静态变量，断电前保持状态）
                                                // 0:未按下 1:已检测到长按按键
    static uint8_t key_flag = 0;                // 按键GPIO电平状态标志（静态变量）
                                                // 用于辅助判断按键物理状态
    static EN_BMS_POWER_STATE  _gBmsPowerStatus = EN_BMS_TURNOFF_START;  // BMS电源状态机当前状态
                                                                         // 初始状态：关机启动状态
    static uint32_t key_wait_time = 0;          // 按键等待超时计时（静态变量）
    static uint32_t key_count = 0;              // 按键计数（预留扩展，如多按键检测）
    KeyMsg_t    *p_msg;                         // 按键消息指针，用于接收消息队列数据

    // 从按键消息队列中获取消息（非阻塞，获取成功则处理）
    if(QueueGet(&gKeyMsgQueue, (void *)&p_msg) == true)
    {
        // 根据按键消息类型分支处理
        switch(p_msg->press_value)
        {
            // 检测到"单键长按"事件（触发关机的关键按键操作）
            case EN_KEY_SWITCH_LONG_SINGLE_PRESS:
            {
                _gKeyPress = 1;  // 置位按键按下标志，通知状态机处理
                break;
            }
            // 其他按键类型（如短按、双击等），暂不处理
            default:
                break;
        }
    }

    // 检测按键GPIO电平状态（KeyCheckHandle为按键GPIO配置句柄）
    if(GpioBitGet((&KeyCheckHandle)))
    {
        // GPIO为高电平（按键未按下/已释放），此处预留扩展（如消抖处理）
    }
    else
    {
        // GPIO为低电平（按键物理按下），置位电平状态标志
        key_flag = 1;
    }

    // BMS电源状态机：根据当前状态处理对应逻辑，实现状态迁移
    switch(_gBmsPowerStatus)
    {
        // 状态1：关机启动状态（初始状态，BMS未正常工作）
        case EN_BMS_TURNOFF_START:
        {
            // 触发条件：1.检测到长按按键且GPIO电平有效  OR  2.等待超时（1500ms）
            if(((_gKeyPress == 1) && (key_flag)) || (TimeDiffFromNow(key_wait_time) > 1500))
            {
                // 再次确认按键GPIO状态（防误触发，确保按键已释放/有效）
                if(GpioBitGet((&KeyCheckHandle)))
                {
                    _gKeyPress = 0;  // 清除按键标志（单次触发后复位）
                    // 清除BMS关机标志位（告知系统当前无需关机）
                    garrBMSReg.bms_hold_para.bms_shutdown = 0;
                    // 状态迁移：从"关机启动"切换到"正常工作"状态
                    _gBmsPowerStatus = EN_BMS_TURN_NORMAL;
                }
            }
            break;
        }

        // 状态2：正常工作状态（BMS正常运行，监测电池状态）
        case EN_BMS_TURN_NORMAL:
        {
            // 触发条件：检测到长按按键且GPIO电平有效（用户主动触发关机）
            if((_gKeyPress == 1) && (key_flag))
            {
                // 再次确认按键GPIO状态（防误触发）
                if(GpioBitGet((&KeyCheckHandle)))
                {
                    _gKeyPress = 0;  // 清除按键标志
                    // 状态迁移：从"正常工作"切换到"关机等待"状态（准备执行关机流程）
                    _gBmsPowerStatus = EN_BMS_TURNOFF_WAIT;
                    StopAFESample();  // 停止AFE（模拟前端）采样（避免关机时数据异常）
                }
            }
            else
            {
                _gKeyPress = 0;  // 无有效按键，清除标志（防残留）
            }
            break;
        }

        // 状态3：关机等待状态（执行关机前准备工作）
        case EN_BMS_TURNOFF_WAIT:
        {
            SoxFlashInfoSave();        // 保存SOX（State of X，如SOC/SOH）相关信息到Flash
            SoxEepromInfoSave();       // 保存SOX相关信息到EEPROM（双重存储，提高数据可靠性）
            UpdateRtcTime(&gPackStatus);  // 更新RTC（实时时钟）时间到电池包状态结构体
            // 设置BMS关机标志位为0x0002（告知系统执行关机，标志位含义需结合全局定义）
            garrBMSReg.bms_hold_para.bms_shutdown = 0x0002;
            // 状态迁移：从"关机等待"切换到"关机完成"状态
            _gBmsPowerStatus = EN_BMS_TRUNOFF_END;
            break;
        }

        // 状态4：关机完成状态（关机流程结束，等待下一次上电）
        case EN_BMS_TRUNOFF_END:
        {
            // 空实现：关机后无需额外操作，保持当前状态直到断电/上电复位
            break;
        }

        // 其他未定义状态（容错处理，避免状态异常导致程序跑飞）
        default:
            break;
    }
}