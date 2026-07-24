/*
 * Fan.c
 *
 *  Created on: 2024年8月12日
 *      Author: Nova007456
 */
#include <fan.h>
#include "F28x_Project.h"
#include "dev_ntc.h"
#include "pwm.h"
#include "debug_analysis.h"

#pragma CODE_SECTION(_FanFaultProcess,".TI.ramfunc");
#pragma CODE_SECTION(FanFaultProcessAll,".TI.ramfunc");
// 全局风扇数组
static FanStrut_t _gFan[FAN_NUM];

/**
* @brief: _FanConfig
* @param: fanIndex   风扇位号索引
*         pwmModule  PWM基地址
*         pwmChannel PWM通道
*         cap_gpio   风扇反馈的GPIO口
*         self_check_time 故障检测计数时间
* @return:Nothing
* @note:初始化风扇配置
*/
static void _FanConfig(uint8_t fanIndex, uint16_t cap_gpio ,FAN_IN_CHK_TYPE in_chk_type,FAN_ACTIVE_LEVEL_TYPE  active_level_type)
{
    if(fanIndex >= FAN_NUM) return ;

    _gFan[fanIndex].state.current_state = FAN_STATE_INIT;
    _gFan[fanIndex].state.speed = EN_FAN_SPEED_0_PERCENT;

    _gFan[fanIndex].value.duty = 0.0f;
    _gFan[fanIndex].value.temperature = 0.0f;
    _gFan[fanIndex].value.level_cont = 0;
    _gFan[fanIndex].value.total_level_cont = 0;

    _gFan[fanIndex].config.self_check_time = SELF_CHECK_TIME;
    _gFan[fanIndex].config.fault_detected = 0;
    _gFan[fanIndex].config.in_chk_type = in_chk_type;
    _gFan[fanIndex].config.speed_check_counter = 0;
    _gFan[fanIndex].config.cap_gpio = cap_gpio;
    _gFan[fanIndex].config.active_level_type = active_level_type;
    _gFan[fanIndex].self_check_cont = 0;
}

/**
* @brief: FanInit
* @param: Nothing
* @return:Nothing
* @note:初始化风扇  风扇故障检测值不同，目的为分时控制，减少中断负荷
*/
void FanInit(void)
{
    // 风扇1配置
    _FanConfig(0, FAN1_CAP_GPIO ,FAN_IN_FAULT_PULSE ,IN_HIGH_LEVLE);
//    // 风扇2配置
//    _FanConfig(1, FAN2_PWM_BASE, FAN2_PWM_CHANNEL, FAN2_CAP_GPIO ,FAN_IN_FAULT_LEVLE ,IN_LOW_ACTIVE);
//    // 风扇3配置
//    _FanConfig(2, FAN3_PWM_BASE, FAN3_PWM_CHANNEL, FAN3_CAP_GPIO ,FAN_IN_FAULT_LEVLE ,IN_LOW_ACTIVE);
//    // 风扇4配置
//    _FanConfig(3, FAN4_PWM_BASE, FAN4_PWM_CHANNEL, FAN4_CAP_GPIO ,FAN_IN_FAULT_LEVLE ,IN_LOW_ACTIVE);
}

/**
* @brief: _FanCalculateSpeed
* @param: fanIndex   风扇位号索引
* @return:转速状态
* @note:风扇转速设置（自检结束后进行转速检测）
*/
static void _FanCalculateSpeed(uint8_t fanIndex)
{
    if(fanIndex >= FAN_NUM)
    {
        return ;
    }
    float temp = _gFan[fanIndex].value.temperature;
    // 多温度等级控制 - 使用所有转速等级
    if(temp >= TEMP_THRESHOLD_LEVEL4)
    {
        _gFan[fanIndex].state.speed = EN_FAN_SPEED_100_PERCENT;
    }
    else if(temp >= TEMP_THRESHOLD_LEVEL3)
    {
        _gFan[fanIndex].state.speed = EN_FAN_SPEED_80_PERCENT;
    }
    else if(temp >= TEMP_THRESHOLD_LEVEL2)
    {
        _gFan[fanIndex].state.speed = EN_FAN_SPEED_60_PERCENT;
    }
    else if(temp >= TEMP_THRESHOLD_LEVEL1)
    {
        _gFan[fanIndex].state.speed = EN_FAN_SPEED_40_PERCENT;
    }
    else if(temp >= TEMP_THRESHOLD_LEVEL0)
    {
        _gFan[fanIndex].state.speed = EN_FAN_SPEED_20_PERCENT;
    }
    else
    {
        _gFan[fanIndex].state.speed = EN_FAN_SPEED_0_PERCENT;
    }
}
/**
* @brief: _FanDutySet
* @param: fanIndex   风扇位号索引
* @return:转速状态
* @note:风扇转速设置（自检结束后进行转速检测）
*/
static void _FanDutySet(uint8_t fanIndex)
{
    if(fanIndex >= FAN_NUM) return;

    switch(_gFan[fanIndex].state.speed)
    {
        case EN_FAN_SPEED_0_PERCENT:
            _gFan[fanIndex].value.duty = 0.0f;
            break;
        case EN_FAN_SPEED_20_PERCENT:
            _gFan[fanIndex].value.duty = 0.2f;
            break;
        case EN_FAN_SPEED_40_PERCENT:
            _gFan[fanIndex].value.duty = 0.4f;
            break;
        case EN_FAN_SPEED_60_PERCENT:
            _gFan[fanIndex].value.duty = 0.6f;
            break;
        case EN_FAN_SPEED_80_PERCENT:
            _gFan[fanIndex].value.duty = 0.8f;
            break;
        case EN_FAN_SPEED_100_PERCENT:
            _gFan[fanIndex].value.duty = 1.0f;
            break;
        default:
            _gFan[fanIndex].value.duty = 0.0f;
            break;
    }
}
void _gFanPwmUpdate(void)
{
    float duty_arr[FAN_NUM] = {0};

    for(uint8_t i = 0 ; i < FAN_NUM ; i++)
    {
        duty_arr[i] = _gFan[i].value.duty;
    }
    FanEpwmUpdata(duty_arr,1);
}
/**
* @brief: _FanStateInit
* @param: fanIndex   风扇位号索引
* @return:noting
* @note:风扇状态初始化
*/
static void _FanStateInit(uint8_t fanIndex)
{
    if(fanIndex >= FAN_NUM) return;

    // 初始化完成，进入自检状态
    _gFan[fanIndex].state.current_state = FAN_STATE_SELF_TEST;
    _gFan[fanIndex].self_check_cont = 0;
    // 自检时以40%速度运行
    _gFan[fanIndex].state.speed = EN_FAN_SPEED_40_PERCENT;
    _FanDutySet(fanIndex);
}

/**
* @brief: _FanSelfCheck
* @param: fanIndex   风扇位号索引
* @return:
* @note:风扇自检
*/
static void _FanSelfCheck(uint8_t fanIndex)
{
    if(fanIndex >= FAN_NUM) return;
    _gFan[fanIndex].self_check_cont ++;      //自检计数
    if(_gFan[fanIndex].self_check_cont >= _gFan[fanIndex].config.self_check_time)
    {
        _gFan[fanIndex].self_check_cont = 0;
        if(_gFan[fanIndex].config.fault_detected)
        {
            _gFan[fanIndex].state.current_state = FAN_STATE_FAULT;            // 自检发现故障，进入故障状态
        }
        else
        {
            _gFan[fanIndex].state.current_state = FAN_STATE_NORMAL;            // 自检通过，进入正常运行状态
        }
        _gFan[fanIndex].state.speed = EN_FAN_SPEED_0_PERCENT;
        _FanDutySet(fanIndex);
    }
}
/**
* @brief: _FanStateNormal
* @param: fanIndex   风扇位号索引
* @return:
* @note:风扇正常状态处理函数
*/
static void _FanStateNormal(uint8_t fanIndex)
{
    if(fanIndex >= FAN_NUM) return;
    _FanCalculateSpeed(fanIndex);                           // 根据温度调整转速
    _FanDutySet(fanIndex);
    if(_gFan[fanIndex].config.fault_detected)                 // 检测到故障，进入故障状态
    {
        _gFan[fanIndex].state.current_state = FAN_STATE_FAULT;
    }
}
/**
* @brief: _FanStateFault
* @param: fanIndex   风扇位号索引
* @return:
* @note:风扇故障状态处理函数
*/
static void _FanStateFault(uint8_t fanIndex)
{
    if(fanIndex >= FAN_NUM) return;
    static uint32_t recovery_counter[FAN_NUM] = {0};
    _gFan[fanIndex].state.speed = EN_FAN_SPEED_0_PERCENT;     // 故障状态下停止风扇
    _FanDutySet(fanIndex);

    recovery_counter[fanIndex]++;         // 例如：经过一段时间后尝试恢复
    if(recovery_counter[fanIndex] >= 1)
    {
        recovery_counter[fanIndex] = 0;
        _gFan[fanIndex].state.current_state = FAN_STATE_RECOVERY;
//        _gFan[fanIndex].config.fault_detected = 0; // 清除故障标志
    }
}
/**
* @brief: _FanStateRecovery
* @param: fanIndex   风扇位号索引
* @return:
* @note:风扇故障恢复状态处理函数
*/
static void _FanStateRecovery(uint8_t fanIndex)
{
    if(fanIndex >= FAN_NUM) return;

    _gFan[fanIndex].state.current_state = FAN_STATE_SELF_TEST;    // 重新自检
    _gFan[fanIndex].state.speed = EN_FAN_SPEED_40_PERCENT;
    _FanDutySet(fanIndex);
}
/**
 * @brief 风扇状态机主处理函数
 */
static void _FanStateProcess(uint8_t fanIndex)
{
    if(fanIndex >= FAN_NUM) return;
    switch(_gFan[fanIndex].state.current_state)
    {
        case FAN_STATE_INIT:
            _FanStateInit(fanIndex);
            break;
        case FAN_STATE_SELF_TEST:
            _FanSelfCheck(fanIndex);
            break;
        case FAN_STATE_NORMAL:
            _FanStateNormal(fanIndex);
            break;
        case FAN_STATE_FAULT:
            _FanStateFault(fanIndex);
            break;
        case FAN_STATE_RECOVERY:
            _FanStateRecovery(fanIndex);
            break;
        default:
            _gFan[fanIndex].state.current_state = FAN_STATE_INIT;
            break;
    }
}
/**
* @brief: _FanSelfCheck
* @param: fanIndex   风扇位号索引
* @return:
* @note:风扇自检
*/
static void _FanFaultProcess(uint8_t fanIndex)
{
    EN_SPEED_STATE current_speed = _gFan[fanIndex].state.speed;

    if(fanIndex >= FAN_NUM) return;
    if(current_speed == EN_FAN_SPEED_0_PERCENT)  return;

    uint8_t current_gpio_state = GPIO_ReadPin(_gFan[fanIndex].config.cap_gpio);

    if(current_gpio_state == _gFan[fanIndex].config.active_level_type)
    {
        // 有效脉冲计数
        _gFan[fanIndex].value.level_cont++;
    }
    // 总脉冲计数
    _gFan[fanIndex].value.total_level_cont++;
    // ==================== 转速故障检测 ====================
    _gFan[fanIndex].config.speed_check_counter++;        //故障检测分时计算计数
        // SetCpuDebugData(4,_gFan[fanIndex].value.level_cont);//20
        // SetCpuDebugData(5,_gFan[fanIndex].value.total_level_cont);//21
    // 每100ms(2000次中断)进行一次转速计算和故障判断
    if(_gFan[fanIndex].config.speed_check_counter >= (SPEED_FAULT_CHECK_TIME + fanIndex))
    {
        _gFan[fanIndex].config.speed_check_counter = 0;
        if(_gFan[fanIndex].config.in_chk_type == FAN_IN_FAULT_LEVLE)         /*假设为电平检测*/
        {
            if(_gFan[fanIndex].value.level_cont < 0.9f * _gFan[fanIndex].value.total_level_cont)
            {
                _gFan[fanIndex].config.fault_detected = 1;
            }
            else
            {
                _gFan[fanIndex].config.fault_detected = 0;
            }
        }
        else                                                                   /*假设为脉冲检测*/
        {
            // 检查有效电平是否匹配
            if(_gFan[fanIndex].value.level_cont < (0.3f * _gFan[fanIndex].value.total_level_cont) || _gFan[fanIndex].value.level_cont > (0.7f * _gFan[fanIndex].value.total_level_cont))
            {
                _gFan[fanIndex].config.fault_detected = 1;
            }
            else
            {
                _gFan[fanIndex].config.fault_detected = 0;
            }

        }
        // 清零计数用于下一个周期
        _gFan[fanIndex].value.level_cont = 0;
        _gFan[fanIndex].value.total_level_cont = 0;
    }

}

/**
* @brief: FanGetAllFaultState
* @param: fanIndex   风扇位号索引
* @return: Fault_Detected
* @note:获取所有风扇故障状态
*/
uint8_t FanGetAllFaultState(void)
{
    uint8_t i;
    for(i = 0; i < FAN_NUM; i++)
    {
        if(_gFan[i].config.fault_detected !=0)
        {
            return 1;
        }
    }
    return 0;
}
/**
* @brief: _FanProcess
* @param: fanIndex   风扇位号索引
* @return:
* @note:单个风扇处理函数
*/
 void FanProcess(uint8_t fanIndex)
{
    if(fanIndex >= FAN_NUM) return;
    _FanStateProcess(fanIndex);
}
/**
 * @brief TempDataGet
 * @param temp_array 温度数组，每个元素对应一个风扇的温度
 * @param array_size 数组大小
 * @return 成功处理的风扇数量
 * @note:温度数据获取
 */
uint8_t TempDataGet(float temp_array[])
{
    uint8_t i;
    uint8_t processed_count = 0;
    // 参数安全检查
    if(temp_array == NULL) {
        return 0;
    }
    for(i = 0; i < FAN_NUM; i++)
    {
        _gFan[i].value.temperature = temp_array[i];
        processed_count++;
    }
    return processed_count;
}
/**
* @brief: FanProcessAll
* @param:
* @return:
* @note:所有风扇处理函数，放入主循环实现
*/
void FanProcessAll(void)
{
    static uint8_t i = 0;
//    static float  gTempArr[NTC_CHANNEL_NUM];
//    /*获取NTC有效数据温度*/
//    for (uint8_t j = 0; j < NTC_CHANNEL_NUM; j++)
//    {
//        if (NTCChkData(j))
//        {
//            gTempArr[j] = NTCGetFilterTemp(j);
//        }
//    }
//    /*将温度传递至风扇数组中*/
//    TempDataGet(gTempArr);
    _gFan[0].value.temperature = GetNTCMaxTemp();
    FanProcess(i);
    i++;
    if(i >= FAN_NUM)
    {
        i = 0;
    }
    _gFanPwmUpdate();      /*传递占空比*/
}
/**
* @brief: FanFaultProcessAll
* @param:
* @return:
* @note:风扇自检
*/
void FanFaultProcessAll(void)
{
     uint8_t i  ;
     for(i = 0; i <= FAN_NUM; i++)
     {
         _FanFaultProcess(i);
     }
}
/**
* @brief: FanFaultProcessAll
* @param:
* @return:
* @note:风扇自检
*/
bool GetFanFaultState(void)
{
     uint8_t i  ;
     for(i = 0; i < FAN_NUM; i++)
     {
         if(_gFan[i].config.fault_detected)
         {
             return true;
         }
     }
     return false;
}

