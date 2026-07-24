/**
 * @file fan.c
 * @brief Fan control, self-test, and fault detection
 */

#include <fan.h>
#include "F28x_Project.h"
#include "ntc.h"
#include "pwm.h"
#include "debug_analysis.h"
#include "gpio.h"

#pragma CODE_SECTION(_FanFaultProcess, ".TI.ramfunc");

static FanStrut_t _gFan[FAN_NUM];

/**
 * @brief Configure one fan instance
 * @param fanIndex Fan index
 * @param cap_gpio Tach feedback GPIO
 * @param in_chk_type Feedback detection type
 * @param active_level_type Active feedback level
 */
static void _FanConfig(uint8_t fanIndex, uint16_t cap_gpio, FAN_IN_CHK_TYPE in_chk_type,
                       FAN_ACTIVE_LEVEL_TYPE active_level_type)
{
    if(fanIndex >= FAN_NUM)
    {
        return;
    }

    _gFan[fanIndex].state.current_state = FAN_STATE_INIT;
    _gFan[fanIndex].state.speed = EN_FAN_SPEED_0_PERCENT;

    _gFan[fanIndex].value.duty = 0.0f;
    _gFan[fanIndex].value.temperature = 0.0f;
    _gFan[fanIndex].value.level_cont = 0U;
    _gFan[fanIndex].value.total_level_cont = 0U;

    _gFan[fanIndex].config.self_check_time = SELF_CHECK_TIME;
    _gFan[fanIndex].config.fault_detected = 0U;
    _gFan[fanIndex].config.in_chk_type = in_chk_type;
    _gFan[fanIndex].config.speed_check_counter = 0U;
    _gFan[fanIndex].config.cap_gpio = cap_gpio;
    _gFan[fanIndex].config.active_level_type = active_level_type;
    _gFan[fanIndex].self_check_cont = 0U;
}

/**
 * @brief Initialize all fans (staggered fault-check timing to reduce ISR load)
 */
void FanInit(void)
{
    _FanConfig(0U, FAN1_CAP_GPIO, FAN_IN_FAULT_PULSE, IN_HIGH_LEVLE);
}

/**
 * @brief Update fan speed from NTC temperatures
 * @param fanIndex Fan index
 */
static void _FanCalculateSpeed(uint8_t fanIndex)
{
    float ctr_temp = 0.0f;
    float pv_temp = 0.0f;
    float inv_temp = 0.0f;

    if(fanIndex >= FAN_NUM)
    {
        return;
    }

    ctr_temp = NTCGetFilterTemp(NTC_CHASSIS_TEMP);
    pv_temp = NTCGetFilterTemp(NTC_PV_TEMP);
    inv_temp = NTCGetFilterTemp(NTC_INV_TEMP);

    if(_gFan[fanIndex].state.speed == EN_FAN_SPEED_80_PERCENT)
    {
        if((inv_temp < 100.0f) && (pv_temp < 100.0f) && (ctr_temp < 75.0f))
        {
            _gFan[fanIndex].state.speed = EN_FAN_SPEED_50_PERCENT;
        }
    }
    else
    {
        if((inv_temp < 75.0f) && (pv_temp < 75.0f) && (ctr_temp < 60.0f))
        {
            _gFan[fanIndex].state.speed = EN_FAN_SPEED_0_PERCENT;
        }
        else if((inv_temp > 105.0f) || (pv_temp > 105.0f) || (ctr_temp > 80.0f))
        {
            _gFan[fanIndex].state.speed = EN_FAN_SPEED_80_PERCENT;
        }
        else if((inv_temp > 85.0f) || (pv_temp > 85.0f) || (ctr_temp > 70.0f))
        {
            _gFan[fanIndex].state.speed = EN_FAN_SPEED_50_PERCENT;
        }
    }
}

/**
 * @brief Map speed state to PWM duty
 * @param fanIndex Fan index
 */
static void _FanDutySet(uint8_t fanIndex)
{
    if(fanIndex >= FAN_NUM)
    {
        return;
    }

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
        case EN_FAN_SPEED_50_PERCENT:
            _gFan[fanIndex].value.duty = 0.5f;
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
    uint8_t i;

    for(i = 0U; i < FAN_NUM; i++)
    {
        duty_arr[i] = _gFan[i].value.duty;
    }
    FanEpwmUpdata(duty_arr, 1U);
}

/**
 * @brief Leave INIT and enter self-test at 40% speed
 * @param fanIndex Fan index
 */
static void _FanStateInit(uint8_t fanIndex)
{
    if(fanIndex >= FAN_NUM)
    {
        return;
    }

    _gFan[fanIndex].state.current_state = FAN_STATE_SELF_TEST;
    _gFan[fanIndex].self_check_cont = 0U;
    _gFan[fanIndex].state.speed = EN_FAN_SPEED_40_PERCENT;
    _FanDutySet(fanIndex);
}

/**
 * @brief Self-test: run at fixed speed and check tach feedback
 * @param fanIndex Fan index
 */
static void _FanSelfCheck(uint8_t fanIndex)
{
    if(fanIndex >= FAN_NUM)
    {
        return;
    }

    _gFan[fanIndex].self_check_cont++;
    if(_gFan[fanIndex].self_check_cont >= _gFan[fanIndex].config.self_check_time)
    {
        _gFan[fanIndex].self_check_cont = 0U;
        if(_gFan[fanIndex].config.fault_detected)
        {
            _gFan[fanIndex].state.current_state = FAN_STATE_FAULT;
        }
        else
        {
            _gFan[fanIndex].state.current_state = FAN_STATE_NORMAL;
        }
        _gFan[fanIndex].state.speed = EN_FAN_SPEED_0_PERCENT;
        _FanDutySet(fanIndex);
    }
}

/**
 * @brief Normal operation: temperature-based speed control
 * @param fanIndex Fan index
 */
static void _FanStateNormal(uint8_t fanIndex)
{
    if(fanIndex >= FAN_NUM)
    {
        return;
    }

    _FanCalculateSpeed(fanIndex);
    _FanDutySet(fanIndex);
    if(_gFan[fanIndex].config.fault_detected)
    {
        _gFan[fanIndex].state.current_state = FAN_STATE_FAULT;
    }
}

/**
 * @brief Fault state: stop fan and schedule recovery
 * @param fanIndex Fan index
 */
static void _FanStateFault(uint8_t fanIndex)
{
    static uint32_t recovery_counter[FAN_NUM] = {0};

    if(fanIndex >= FAN_NUM)
    {
        return;
    }

    _gFan[fanIndex].state.speed = EN_FAN_SPEED_0_PERCENT;
    _FanDutySet(fanIndex);

    recovery_counter[fanIndex]++;
    if(recovery_counter[fanIndex] >= 1U)
    {
        recovery_counter[fanIndex] = 0U;
        _gFan[fanIndex].state.current_state = FAN_STATE_RECOVERY;
    }
}

/**
 * @brief Recovery: re-enter self-test at 40% speed
 * @param fanIndex Fan index
 */
static void _FanStateRecovery(uint8_t fanIndex)
{
    if(fanIndex >= FAN_NUM)
    {
        return;
    }

    _gFan[fanIndex].state.current_state = FAN_STATE_SELF_TEST;
    _gFan[fanIndex].state.speed = EN_FAN_SPEED_40_PERCENT;
    _FanDutySet(fanIndex);
}

/**
 * @brief Fan state machine
 * @param fanIndex Fan index
 */
static void _FanStateProcess(uint8_t fanIndex)
{
    if(fanIndex >= FAN_NUM)
    {
        return;
    }

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
 * @brief Tach / level fault detection (called from ISR)
 * @param fanIndex Fan index
 */
static void _FanFaultProcess(uint8_t fanIndex)
{
    EN_SPEED_STATE current_speed;
    uint8_t current_gpio_state = 0U;

    if(fanIndex >= FAN_NUM)
    {
        return;
    }

    current_speed = _gFan[fanIndex].state.speed;
    if(current_speed == EN_FAN_SPEED_0_PERCENT)
    {
        return;
    }

    current_gpio_state = GPIO_ReadPin(_gFan[fanIndex].config.cap_gpio);

    if(current_gpio_state == _gFan[fanIndex].config.active_level_type)
    {
        _gFan[fanIndex].value.level_cont++;
    }

    _gFan[fanIndex].value.total_level_cont++;
    _gFan[fanIndex].config.speed_check_counter++;

    if(_gFan[fanIndex].config.speed_check_counter >= (SPEED_FAULT_CHECK_TIME + fanIndex))
    {
        _gFan[fanIndex].config.speed_check_counter = 0U;
        if(_gFan[fanIndex].config.in_chk_type == FAN_IN_FAULT_LEVLE)
        {
            if(_gFan[fanIndex].value.level_cont < 0.9f * _gFan[fanIndex].value.total_level_cont)
            {
                _gFan[fanIndex].config.fault_detected = 1U;
            }
            else
            {
                _gFan[fanIndex].config.fault_detected = 0U;
            }
        }
        else
        {
            if(_gFan[fanIndex].value.level_cont < (0.3f * _gFan[fanIndex].value.total_level_cont)
               || _gFan[fanIndex].value.level_cont > (0.7f * _gFan[fanIndex].value.total_level_cont))
            {
                _gFan[fanIndex].config.fault_detected = 1U;
            }
            else
            {
                _gFan[fanIndex].config.fault_detected = 0U;
            }
        }

        _gFan[fanIndex].value.level_cont = 0U;
        _gFan[fanIndex].value.total_level_cont = 0U;
    }
}

uint8_t FanGetAllFaultState(void)
{
    uint8_t i;

    for(i = 0U; i < FAN_NUM; i++)
    {
        if(_gFan[i].config.fault_detected != 0U)
        {
            return 1U;
        }
    }
    return 0U;
}

void FanProcess(uint8_t fanIndex)
{
    if(fanIndex >= FAN_NUM)
    {
        return;
    }
    _FanStateProcess(fanIndex);
}

/**
 * @brief Copy external temperature array into fan instances
 * @param temp_array Temperature per fan
 * @param array_size Unused; kept for API compatibility
 * @return Number of fans updated
 */
uint8_t TempDataGet(float temp_array[])
{
    uint8_t i;
    uint8_t processed_count = 0U;

    if(temp_array == NULL)
    {
        return 0U;
    }

    for(i = 0U; i < FAN_NUM; i++)
    {
        _gFan[i].value.temperature = temp_array[i];
        processed_count++;
    }
    return processed_count;
}

/**
 * @brief Round-robin fan process (call from main loop)
 */
void FanProcessAll(void)
{
    static uint8_t i = 0U;

    _gFan[0].value.temperature = GetNTCMaxTemp();
    FanProcess(i);
    i++;
    if(i >= FAN_NUM)
    {
        i = 0U;
    }
    _gFanPwmUpdate();
}

/**
 * @brief Run tach fault detection for all fans (call from ISR)
 */
void FanFaultProcessAll(void)
{
    uint8_t i;

    for(i = 0U; i <= FAN_NUM; i++)
    {
        _FanFaultProcess(i);
    }
}

bool GetFanFaultState(void)
{
    uint8_t i;

    for(i = 0U; i < FAN_NUM; i++)
    {
        if(_gFan[i].config.fault_detected)
        {
            return true;
        }
    }
    return false;
}
