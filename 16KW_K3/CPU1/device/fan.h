/**
 * @file fan.h
 * @brief Fan control, self-test, and fault detection
 */

#include "stdint.h"
#include "F28x_Project.h"

#ifndef MIDDLEWARE_FAN_H_
#define MIDDLEWARE_FAN_H_

/* ==================== Configuration ==================== */
#define FAN_NUM (1U)

//#define FAN1_CAP_GPIO (80U)
//#define FAN2_CAP_GPIO (81U)
//#define FAN3_CAP_GPIO (52U)
//#define FAN4_CAP_GPIO (53U)

/* Temperature thresholds (degC) */
#define TEMP_THRESHOLD_LEVEL0 (20.0f)
#define TEMP_THRESHOLD_LEVEL1 (40.0f)
#define TEMP_THRESHOLD_LEVEL2 (60.0f)
#define TEMP_THRESHOLD_LEVEL3 (70.0f)
#define TEMP_THRESHOLD_LEVEL4 (80.0f)

/* Self-test duration (100 ms units) */
#define SELF_CHECK_TIME (36U * FAN_NUM) /* 1.2 s */

/* Speed fault check interval (20 kHz ISR counts ~= 250 ms) */
#define SPEED_FAULT_CHECK_TIME ((80000U / 4U) * 0.25f)

typedef enum
{
    FAN_STATE_INIT = 0,
    FAN_STATE_SELF_TEST,
    FAN_STATE_NORMAL,
    FAN_STATE_FAULT,
    FAN_STATE_RECOVERY
} FAN_STATE;

typedef enum
{
    EN_FAN_SPEED_0_PERCENT = 0,
    EN_FAN_SPEED_20_PERCENT,
    EN_FAN_SPEED_40_PERCENT,
    EN_FAN_SPEED_50_PERCENT,
    EN_FAN_SPEED_80_PERCENT,
    EN_FAN_SPEED_100_PERCENT,
    EN_FAN_SPEED_MAX
} EN_SPEED_STATE;

typedef enum
{
    FAN_IN_FAULT_PULSE = 0, /**< Pulse (tach) detection */
    FAN_IN_FAULT_LEVLE    /**< Level detection */
} FAN_IN_CHK_TYPE;

typedef enum
{
    IN_LOW_ACTIVE = 0,
    IN_HIGH_LEVLE
} FAN_ACTIVE_LEVEL_TYPE; /**< Active feedback level */

typedef struct
{
    FAN_STATE current_state;
    EN_SPEED_STATE speed;
} FanState_t;

typedef struct
{
    float duty;
    float temperature;
    uint32_t level_cont;       /**< Active-level pulse count */
    uint32_t total_level_cont; /**< Total sample count */
} FanValue_t;

typedef struct
{
    uint16_t self_check_time;        /**< Self-test time (100 ms units) */
    uint32_t speed_check_counter;    /**< Speed monitor counter */
    uint16_t cap_gpio;               /**< Tach feedback GPIO */
    FAN_IN_CHK_TYPE in_chk_type;
    uint8_t fault_detected;
    FAN_ACTIVE_LEVEL_TYPE active_level_type;
} FanConfig_t;

typedef struct
{
    FanState_t state;
    FanValue_t value;
    FanConfig_t config;
    uint16_t self_check_cont;
} FanStrut_t;

void FanInit(void);
void FanProcessAll(void);
void FanFaultProcessAll(void);
uint8_t TempDataGet(float temp_array[]);
void FanProcess(uint8_t fanIndex);
bool GetFanFaultState(void);
uint8_t FanGetAllFaultState(void);

#endif /* MIDDLEWARE_FAN_H_ */
