/**
 * @file ntc.h
 * @brief NTC temperature acquisition, filtering, and fault detection
 */

#ifndef _NTC_H_
#define _NTC_H_

#include "F28x_Project.h"
#include "dev_ntc.h"
#include "mid_sliding_filter.h"

/** User-configurable channel count */
#define NTC_CHANNEL_NUM         (3U)
#define NTC_AVERAGE_COUNT       (30U)   /**< Sliding-average window size */
#define NTC_FAULT_JUDGE_COUNT   (10U)   /**< Fault check interval (not every cycle) */

typedef enum
{
    NTC_FAULT_NONE = 0,
    NTC_FAULT_BREAK,
} NTC_FAULT_TYPE;

typedef struct
{
    NTC_FAULT_TYPE current_fault;
    NTC_FAULT_TYPE previous_fault;
    bool fault_active;
    uint8_t fault_count;
    uint8_t fault_run_cont; /**< Run counter; fault judge deferred until system init completes */
} NtcFaultChannel_t;

typedef struct
{
    float raw_temperature[NTC_CHANNEL_NUM];
    float filtered_temperature[NTC_CHANNEL_NUM];
    bool valid[NTC_CHANNEL_NUM];
    Mid_SlidingFilter_t filter[NTC_CHANNEL_NUM];
    uint8_t current_channel;
    NtcFaultChannel_t fault[NTC_CHANNEL_NUM];
} NtcTemp_t;

typedef enum
{
    NTC_CHASSIS_TEMP = 0,
    NTC_PV_TEMP,
    NTC_INV_TEMP
} NTC_VALUE_TYPE;

void NTCGetInVol(uint8_t channel, float vol);
void NTCInit(void);
void NTCProcess(void);

float NTCGetRawTemp(uint8_t channel);
float NTCGetFilterTemp(NTC_VALUE_TYPE channel);
float GetNTCMaxTemp(void);
bool GetNTCFault(void);

bool NTCChkData(uint8_t channel);
bool NTCFaultActiveState(uint8_t channel);
uint8_t NTCGetFaultCount(uint8_t channel);
void NTCClearFault(uint8_t channel);
void NTCClearAllFault(void);
bool NTCFaultChangeState(uint8_t channel);

#endif /* _NTC_H_ */
