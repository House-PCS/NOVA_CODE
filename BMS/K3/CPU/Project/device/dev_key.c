/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_key.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-6-1
  * @brief       按键驱动核心实现文件，支持GPIO类型按键的短按、长按单次、长按连续、组合按键检测
  *              采用状态机机制处理按键消抖、按下/抬起判断、按键事件分类，事件通过消息队列上报
  ******************************************************************************
  * @attention
  * 1. 按键硬件模式当前仅支持GPIO，后续可扩展ADC按键
  * 2. 每次仅处理一个按键事件，避免多按键冲突
  * 3. 消息队列需提前初始化，确保事件能正常上报
  * 4. 依赖gpio.h（GPIO操作）、dev_key.h（枚举/结构体定义）、Tim系统定时器接口
  * @relation
  * @par :
  * <table>
  * <tr><<th>Date          <<th>Version   <<th>Author      <<th>Description
  * <tr><td>2025-6-1    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "dev_key.h"   // 包含按键相关枚举、结构体定义
#include "gpio.h"      // 包含GPIO操作接口

#define LOG_TAG "dev_key"  // 日志标签，用于调试输出

// 按键轮询周期（单位：ms），建议在定时器中断中按此周期调用KeyDetectProcess
#define KEY_POLLING_PERIOD              20
// 按键消抖时间（单位：ms），过滤机械按键抖动干扰
#define KEY_FILTER_TIME                 20
// 预留：按键中按时间阈值（当前未使用）
#define KEY_PRESS_MIDDLE_TIME           300
// 长按连续模式：首次连续触发前的等待时间（单位：ms）
#define KEY_PRESS_CONTINUOUS_PRE_TIME   1000
// 长按连续模式：后续连续触发的时间间隔（单位：ms）
#define KEY_PRESS_CONTINUOUS_TIME       100
// 长按单次模式：触发长按事件的最小时间阈值（单位：ms）
#define KEY_PRESS_LONG_TIME             1500
// 组合按键：触发组合长按事件的最小时间阈值（单位：ms）
#define KEY_COMBINATION_PRESS_LONG_TIME 2000

// 按键消息队列：存储按键事件，供上层模块读取
Queue_t gKeyMsgQueue;
// 按键信息数组：存储每个按键的配置参数（按EN_KEY_SRC_NUM定义的按键数量）
static KeyInfo_t _gKeyInfo[EN_KEY_SRC_NUM];
// GPIO句柄数组：存储每个按键对应的GPIO操作句柄
static Gpio_HandleType *pHandle[EN_KEY_SRC_NUM];
// 电源输入状态标识（预留，当前未使用）
static uint8_t power_input_status = 0;

/**
  * @brief  按键硬件状态读取函数
  * @param  src: 按键源（EN_KEY_SRC枚举类型），指定要读取的按键
  * @retval bool: 按键当前状态
  *               true - 按键按下（符合配置的按压电平）
  *               false - 按键未按下
  * @note   仅支持GPIO模式按键读取，根据按键配置的按压电平（高/低）判断状态
  */
static bool _KeyPress(EN_KEY_SRC src)
{
    // 判断按键硬件模式是否为GPIO
    if(_gKeyInfo[src].key_hw_mode == EN_KEY_HW_MODE_GPIO)
    {
        // 读取GPIO引脚电平
        uint8_t value = GpioBitGet(pHandle[src]);
        // 判断按键配置的按压电平为低电平有效
        if(_gKeyInfo[src].key_press_volt == EN_KEY_PRESS_VOLTAGE_LOW)
        {
            return (bool)(!value);  // 低电平有效：读取值为0时返回按下（true）
        }
        else
        {
            return (bool)(value);   // 高电平有效：读取值为1时返回按下（true）
        }
    }
    else
    {
        /*后续增加ADC按键的读取逻辑*/
        return false;
    }
}

/**
  * @brief  按键模块初始化函数
  * @param  无
  * @retval 无
  * @note   1. 初始化按键配置信息（按键源、按压功能、有效电平、硬件模式）
  *         2. 初始化按键消息队列（存储按键事件）
  *         3. 调用底层BSP初始化函数（配置GPIO等硬件）
  */
void KeyInit()
{
    // 初始化第0个按键（EN_KEY_SRC_SWITCH）的配置信息
    _gKeyInfo[0].key_src = EN_KEY_SRC_SWITCH;  // 按键源：开关按键
    // 按压功能：支持短按和长按单次
    _gKeyInfo[0].key_press_func = EN_KEY_PRESS_FUNC_SHORT_LONG_SINGLE_PRESS;
    _gKeyInfo[0].key_press_volt = EN_KEY_PRESS_VOLTAGE_LOW;  // 低电平有效
    _gKeyInfo[0].key_hw_mode = EN_KEY_HW_MODE_GPIO;  // 硬件模式：GPIO
    pHandle[0] = &KeyCheckHandle;  // 绑定该按键的GPIO操作句柄

    // 初始化按键消息队列：队列长度KEY_EVENT_MSG_QUEUE_NUM，每个元素大小为KeyMsg_t
    QueueInit(&gKeyMsgQueue, KEY_EVENT_MSG_QUEUE_NUM, sizeof(KeyMsg_t));
    KeyCheckBspInit();  // 按键底层BSP初始化（配置GPIO为输入模式等）
}

/**
  * @brief  组合按键检测函数
  * @param  index: 当前检测的目标按键索引
  * @retval bool: 组合按键检测结果
  *               true - 存在组合按键（除目标按键外还有其他按键按下）
  *               false - 无组合按键（仅目标按键按下或无按键按下）
  * @note   遍历所有按键源，判断是否有除目标按键外的其他按键处于按下状态
  */
static bool _CombinationPressDetect(EN_KEY_SRC index)
{
    int j = 0;
    // 遍历所有按键源
    for(j = 0; j < EN_KEY_SRC_NUM; j++)
    {
        // 跳过当前目标按键
        if(j != index)
        {
            // 检测其他按键是否按下，若有则判定为组合按键
            if(_KeyPress((EN_KEY_SRC)j) == true)/*有组合按键*/
            {
                return true;
            }
        }
    }
    return false;  // 无其他按键按下，非组合按键
}

/**
  * @brief  按键检测主处理函数（状态机核心）
  * @param  无
  * @retval 无
  * @note   1. 需按KEY_POLLING_PERIOD周期调用，建议在定时器中断中执行
  *         2. 采用状态机管理按键生命周期：空闲→消抖→按下→持续→抬起/等待
  *         3. 支持短按、长按单次、长按连续、组合按键四种事件类型
  *         4. 按键事件通过消息队列gKeyMsgQueue上报，上层需读取队列处理
  */
void KeyDetectProcess()
{
    static uint32_t timeout = 0;                  // 预留超时变量（当前未使用）
    static uint32_t key_filter_time_cnt = 0;      // 消抖时间计数（记录消抖开始时刻）
    static uint32_t key_press_time_cnt = 0;       // 按键按下时间计数（记录按下/连续触发时刻）
    KeyMsg_t    msg;                              // 按键事件消息（用于上报）
    static EN_KEY_SRC  key_index = EN_KEY_SRC_NUM;// 当前处理的按键索引（初始化为无效值）
    static EN_KEY_HW_STATUS key_status = EN_KEY_STATUS_IDLE;  // 按键状态机当前状态

    // 状态机切换逻辑
    switch(key_status)
    {
        // 状态1：空闲状态（无按键按下）
        case EN_KEY_STATUS_IDLE:
        {
            uint8_t i;
            /*每次只处理一个按键，避免多按键冲突*/
            // 遍历所有按键源，检测是否有按键按下
            for(i = 0; i < EN_KEY_SRC_NUM; i++)
            {
                if(_KeyPress((EN_KEY_SRC)i) == true)
                {
                    key_index = (EN_KEY_SRC)i;  // 记录当前按下的按键索引
                    key_status = EN_KEY_STATUS_FILTER;  // 切换到消抖状态
                    key_filter_time_cnt = TimGetSystime();  // 记录消抖开始时间
                    return;  // 退出本次处理，等待下一次轮询
                }
            }
            // 遍历所有按键均未按下，重置按键索引为无效值
            if(i == EN_KEY_SRC_NUM)
            {
                key_index = EN_KEY_SRC_NUM;
            }
            break;
        }

        // 状态2：消抖状态（过滤按键机械抖动）
        case EN_KEY_STATUS_FILTER:
        {
            // 消抖时间已到（当前时间 - 消抖开始时间 > 消抖阈值）
            if((uint32_t)(TimGetSystime() - key_filter_time_cnt) > KEY_FILTER_TIME)
            {
                // 再次检测按键：仍处于按下状态（确认是有效按下）
                if(_KeyPress(key_index) == true)
                {
                    key_status = EN_KEY_STATUS_PRESS_DOWN;  // 切换到按下状态
                }
                else
                {
                    // 按键已抬起（抖动导致的误触发），返回空闲状态
                    key_status = EN_KEY_STATUS_IDLE;
                }
            }
            break;
        }

        // 状态3：按下状态（确认按键有效按下，判断按键功能类型）
        case EN_KEY_STATUS_PRESS_DOWN:
        {
            // 检测是否为组合按键
            if(_CombinationPressDetect(key_index) == true)/*有组合按键*/
            {
                key_status = EN_KEY_STATUS_SUSTAIN;  // 切换到持续状态（组合按键专用）
            }
            else/*无组合按键，根据按键功能类型处理*/
            {
                // 功能类型1：仅支持短按（按下即触发事件）
                if(_gKeyInfo[key_index].key_press_func == EN_KEY_PRESS_FUNC_ONLY_SHORT_PRESS)
                {
                    // 构造短按事件消息（事件ID = 基础短按ID + 按键索引）
                    msg.press_value = (EN_KEY_PRESSED_MSG)(EN_KEY_SWITCH_SHORT_PRESSED + key_index);
                    QueuePut(&gKeyMsgQueue, &msg);  // 事件消息入队
                    key_status = EN_KEY_STATUS_SUSTAIN;  // 切换到持续状态（等待按键抬起）
                }
                // 功能类型2：支持短按和长按单次
                else if(_gKeyInfo[key_index].key_press_func == EN_KEY_PRESS_FUNC_SHORT_LONG_SINGLE_PRESS)
                {
                    key_status = EN_KEY_STATUS_SUSTAIN;  // 切换到持续状态（判断长按/短按）
                    key_press_time_cnt  = TimGetSystime();  // 记录按键按下开始时间
                }
                else/*功能类型3：支持长按连续（按下先触发一次短按，后续持续触发）*/
                {
                    // 先触发一次短按事件
                    msg.press_value = (EN_KEY_PRESSED_MSG)(EN_KEY_SWITCH_SHORT_PRESSED + key_index);
                    QueuePut(&gKeyMsgQueue, &msg);  // 事件消息入队
                    // 切换到长按连续前置状态（等待连续触发阈值）
                    key_status = EN_KEY_STATUS_SUSTAIN_CONTINUOUS_PRE;
                    key_press_time_cnt  = TimGetSystime();  // 记录首次触发时间
                }
            }
            break;
        }

        // 状态4：持续状态（处理长按单次、组合按键长按）
        case EN_KEY_STATUS_SUSTAIN:
        {
            uint8_t combination_flag = 0;  // 组合按键标识（预留）
            // 功能类型：长按单次（判断是否达到长按阈值）
            if(_gKeyInfo[key_index].key_press_func == EN_KEY_PRESS_FUNC_SHORT_LONG_SINGLE_PRESS)
            {
                // 按下时间超过长按阈值
                if((uint32_t)(TimGetSystime() - key_press_time_cnt) > KEY_PRESS_LONG_TIME)
                {
                    // 确认无组合按键（避免组合按键误触发）
                    if(_CombinationPressDetect(key_index) == false)
                    {
                        // 构造长按单次事件消息
                        msg.press_value = (EN_KEY_PRESSED_MSG)(EN_KEY_SWITCH_LONG_SINGLE_PRESS + key_index);
                        QueuePut(&gKeyMsgQueue, &msg);  // 事件消息入队
                        key_status = EN_KEY_STATUS_WAIT;  // 切换到等待状态（等待按键抬起）
                        key_press_time_cnt  = TimGetSystime();  // 更新时间戳（无实际用途，预留）
                    }
                }
                else
                {
                    // 未达到长按阈值，检测按键是否抬起（抬起则触发短按）
                    if(_KeyPress(key_index) == false)/*准备执行按键抬起触发短按*/
                    {
                        key_status = EN_KEY_STATUS_PRESS_UP;  // 切换到抬起状态
                    }
                }
            }
            else/*非长按单次功能（如仅短按、组合按键），仅等待按键抬起*/
            {
                if(_KeyPress(key_index) == false)
                {
                    key_status = EN_KEY_STATUS_IDLE;  // 按键抬起，返回空闲状态
                }
            }

            // 检测组合按键长按（无论当前功能类型，只要组合按键按下时间达标）
            if((uint32_t)(TimGetSystime() - key_press_time_cnt) > KEY_COMBINATION_PRESS_LONG_TIME)
            {
                if(_CombinationPressDetect(key_index) == true)
                {
                    /*执行组合按键功能（预留接口，当前未实现具体逻辑）*/
//                    _CombinationLongPressFunc(key_index);
                    key_status = EN_KEY_STATUS_WAIT;  // 切换到等待状态（等待按键抬起）
                }
            }
            break;
        }

        // 状态5：长按连续前置状态（长按连续模式专用，等待首次连续触发阈值）
        case EN_KEY_STATUS_SUSTAIN_CONTINUOUS_PRE:
        {
            // 按键抬起，返回空闲状态
            if(_KeyPress(key_index) == false)
            {
                key_status = EN_KEY_STATUS_IDLE;
            }
            else
            {
                // 等待时间超过连续触发前置阈值
                if((uint32_t)(TimGetSystime() - key_press_time_cnt) > KEY_PRESS_CONTINUOUS_PRE_TIME)
                {
                    // 切换到长按连续状态（开始持续触发短按事件）
                    key_status = EN_KEY_STATUS_SUSTAIN_CONTINUOUS;
                }
            }
            break;
        }

        // 状态6：长按连续状态（长按连续模式专用，按间隔持续触发短按事件）
        case EN_KEY_STATUS_SUSTAIN_CONTINUOUS:
        {
            // 按键抬起，返回空闲状态
            if(_KeyPress(key_index) == false)
            {
                key_status = EN_KEY_STATUS_IDLE;
            }
            else
            {
                // 达到连续触发时间间隔
                if((uint32_t)(TimGetSystime() - key_press_time_cnt) > KEY_PRESS_CONTINUOUS_TIME)
                {
                    // 构造短按事件消息（持续触发）
                    msg.press_value = (EN_KEY_PRESSED_MSG)(EN_KEY_SWITCH_SHORT_PRESSED + key_index);
                    QueuePut(&gKeyMsgQueue, &msg);  // 事件消息入队
                    key_press_time_cnt  = TimGetSystime();  // 更新时间戳（下次触发计时起点）
                }
            }
            break;
        }

        // 状态7：等待状态（长按/组合按键触发后，等待按键抬起）
        case EN_KEY_STATUS_WAIT:
        {
            // 按键抬起，返回空闲状态
            if(_KeyPress(key_index) == false)
            {
                key_status = EN_KEY_STATUS_IDLE;
            }
            break;
        }

        // 状态8：抬起状态（短按+长按单次模式专用，未达长按阈值时抬起触发短按）
        case EN_KEY_STATUS_PRESS_UP:
        {
            // 构造短按事件消息（按键抬起时触发）
            msg.press_value = (EN_KEY_PRESSED_MSG)(EN_KEY_SWITCH_SHORT_PRESSED + key_index);
            QueuePut(&gKeyMsgQueue, &msg);  // 事件消息入队
            key_status = EN_KEY_STATUS_IDLE;  // 返回空闲状态
            break;
        }

        // 默认状态：异常状态，返回空闲
        default:
            break;
    }
    return;

// 以下为旧版按键处理逻辑（已注释禁用），采用每个按键独立状态机，现改用全局单状态机
#if 0
    for(i = 0; i < EN_KEY_SRC_NUM; i++)
    {
        switch(_gKeyInfo[i].key_status)
        {
            case EN_KEY_STATUS_IDLE:
            {
                if(_gKeyInfo[i].key_press() == true)
                {
                    _gKeyInfo[i].key_status = EN_KEY_STATUS_FILTER;
                    _gKeyInfo[i].key_filter_time_cnt = TimGetSystime();
                }
                break;
            }
            case EN_KEY_STATUS_FILTER:
            {
                if((uint32_t)(TimGetSystime() - _gKeyInfo[i].key_filter_time_cnt) > KEY_FILTER_TIME)
                {
                    if(_gKeyInfo[i].key_press() == true)
                    {
                        _gKeyInfo[i].key_status = EN_KEY_STATUS_PRESS_DOWN;
                    }
                    else
                    {
                        _gKeyInfo[i].key_status = EN_KEY_STATUS_IDLE;
                    }
                    _gKeyInfo[i].key_press_time_cnt  = TimGetSystime();
                }
                break;
            }
            case EN_KEY_STATUS_PRESS_DOWN:
            {
                if(_gKeyInfo[i].key_press_func == EN_KEY_PRESS_FUNC_ONLY_SHORT_PRESS)
                {
                    msg.press_value = EN_KEY_SWITCH_SHORT_PRESSED + i;
                    QueuePut(&gKeyMsgQueue, &msg);
                    _gKeyInfo[i].key_status = EN_KEY_STATUS_WAIT;
                }
                else if(_gKeyInfo[i].key_press_func == EN_KEY_PRESS_FUNC_SHORT_LONG_SINGLE_PRESS)/*长按单次*/
                {
                    if((uint32_t)(TimGetSystime() - _gKeyInfo[i].key_press_time_cnt) > KEY_PRESS_LONG_TIME)
                    {
                        msg.press_value = EN_KEY_SWITCH_LONG_PRESSED + i;
                        QueuePut(&gKeyMsgQueue, &msg);
                        _gKeyInfo[i].key_status = EN_KEY_STATUS_WAIT;
                        _gKeyInfo[i].key_press_time_cnt  = TimGetSystime();
                    }
                    else
                    {
                        if(_gKeyInfo[i].key_press() == false)
                        {
                            _gKeyInfo[i].key_status = EN_KEY_STATUS_PRESS_UP;
                        }
                    }
                }
                else/*长按连续*/
                {
                    /*先触发一次短按*/
                    msg.press_value = EN_KEY_SWITCH_SHORT_PRESSED + i;
                    QueuePut(&gKeyMsgQueue, &msg);
                    _gKeyInfo[i].key_status = EN_KEY_STATUS_SUSTAIN_CONTINUOUS_PRE;
                    _gKeyInfo[i].key_press_time_cnt  = TimGetSystime();
                }
                break;
            }
            case EN_KEY_STATUS_SUSTAIN_CONTINUOUS_PRE:/*只在长按连续模式触发*/
            {
                if(_gKeyInfo[i].key_press() == false)
                {
                    _gKeyInfo[i].key_status = EN_KEY_STATUS_IDLE;
                }
                else
                {
                    if((uint32_t)(TimGetSystime() - _gKeyInfo[i].key_press_time_cnt) > KEY_PRESS_CONTINUOUS_PRE_TIME)
                    {
                        _gKeyInfo[i].key_status = EN_KEY_STATUS_SUSTAIN_CONTINUOUS;
                    }
                }
                break;
            }
            case EN_KEY_STATUS_SUSTAIN_CONTINUOUS:/*只在长按连续模式触发*/
            {
                if(_gKeyInfo[i].key_press() == false)
                {
                    _gKeyInfo[i].key_status = EN_KEY_STATUS_IDLE;
                }
                else
                {
                    if((uint32_t)(TimGetSystime() - _gKeyInfo[i].key_press_time_cnt) > KEY_PRESS_CONTINUOUS_TIME)
                    {
                        msg.press_value = EN_KEY_SWITCH_SHORT_PRESSED + i;
                        QueuePut(&gKeyMsgQueue, &msg);
                        _gKeyInfo[i].key_press_time_cnt  = TimGetSystime();
                    }
                }
                break;
            }
            case EN_KEY_STATUS_WAIT:
            {
                if(_gKeyInfo[i].key_press() == false)
                {
                    _gKeyInfo[i].key_status = EN_KEY_STATUS_IDLE;
                }
                break;
            }
            case EN_KEY_STATUS_PRESS_UP:/*同时支持短按和长按单次功能的按键，短按是在按键抬起时触发(按下时长未达到长按阈值)*/
            {
                msg.press_value = EN_KEY_SWITCH_SHORT_PRESSED + i;
                QueuePut(&gKeyMsgQueue, &msg);
                _gKeyInfo[i].key_status = EN_KEY_STATUS_IDLE;
                break;
            }
            default:
                break;
        }
    }
#endif
}