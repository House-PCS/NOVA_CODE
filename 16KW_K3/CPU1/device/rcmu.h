/*
 * rcmu.h
 *
 *  Created on: Oct 30, 2025
 *      Author: Nova006348
 */
#include "F28x_Project.h"
#include "system.h"
#include <string.h>
#include <math.h>
#ifndef APP_RCMU_H_
#define APP_RCMU_H_


/* RMS calculation macro */
#define LEAK_CURRENT_CALCULATE_RMS(sum_squares, sample_count) \
    ((sample_count) == 0 ? 0.0f : sqrtf((sum_squares) / (sample_count)))

/* History buffer update macro */
#define LEAK_CURRENT_UPDATE_HISTORY_VALUES(history, new_value) \
    do { \
        float *_hist = (history); \
        float _new_val = (new_value); \
        if (_hist != NULL) { \
            _hist[2] = _hist[1]; \
            _hist[1] = _hist[0]; \
            _hist[0] = _new_val; \
        } \
    } while(0)



/* System configuration macros */
#define LEAK_CURRENT_SELF_TEST_THRESHOLD_MA     30.0f  /* Pre-check threshold 30mA */
#define LEAK_CURRENT_SELF_TEST_RANGE_MIN_MA     40.0f  /* Self-test range min 40mA */
#define LEAK_CURRENT_SELF_TEST_RANGE_MAX_MA     60.0f  /* Self-test range max 60mA */
#define LEAK_CURRENT_SELF_TEST_FAULT_COUNT      5U     /* Self-test success count threshold */

#define LEAK_CURRENT_INIT_DELAY_TIME          250U      /* Init delay 5s (250 * 20ms) */

//#define LEAK_CURRENT_FAULT_THRESHOLD_THREE    120.0f /* Instant fault threshold 120mA */




#define LEAK_CURRENT_FAULT_THRESHOLD_TWO                 280.0f  /* Level-2 fault threshold ~300mA */
#define LEAK_CURRENT_TWO_LEVEL_FAULT_THRESHOLD           12U     /* Level-2 fault count threshold */

#define LEAK_CURRENT_SQUARE_DIFF_150MA_THRESHOLD        148.0f   /* 150mA step threshold */
#define LEAK_CURRENT_SQUARE_DIFF_150MA_COUNT           1U     /* 150mA step count threshold */

#define LEAK_CURRENT_SQUARE_DIFF_60MA_THRESHOLD        58.0f   /* 60mA step threshold */
#define LEAK_CURRENT_SQUARE_DIFF_60MA_COUNT     3U     /* 60mA step count threshold */

/* Fault count configuration */
#define LEAK_CURRENT_SQUARE_DIFF_30MA_THRESHOLD        27.0f /* 30mA step threshold */
#define LEAK_CURRENT_SQUARE_DIFF_30MA_COUNT     6U     /* 30mA step count threshold */

#define LEAK_CURRENT_LPF_CUTOFF_HZ      500.0f   /* LPF cutoff 500Hz */
#define LEAK_CURRENT_SAMPLE_FREQ_HZ     20000.0f /* Sample rate 20kHz */

/* State machine */
typedef enum
{
    EN_LEAK_CURR_STATE_PRE_CHK = 0,   /* Pre-check with test IO off */
    EN_LEAK_CURR_STATE_SELF_CHK,         /* Self-check with test IO on */
    EN_LEAK_CURR_STATE_RUN_DELAY,             /* Post-init delay state */
    EN_LEAK_CURR_STATE_RUN_NOMAL,       /* Normal run state */
    EN_LEAK_CURR_STATE_FAULT                   /* Fault state */
} EN_LEAK_CURR_STATE;

/* Leak current detection result */
typedef struct
{
    int16 self_test_passed;        /* Self-test result: 0 idle, 1 pass, -1 fail */
    bool fault_flag;      /* Fault flag */
    uint8_t leak_type;      /* Leak current type */
    float rms_current_ma;         /* Leak current RMS (mA) */
    float instant_curr_ma;
    float avg_rms_current_ma;     /* Average leak current RMS (mA) */
} LeakResult_t;

/* Leak current detector internal state */
typedef struct
{

    /* Self-test related */
    uint8_t self_test_fault_count;     /* Pre-check fault count */
    uint8_t self_test_success_count;   /* Self-check success count */

    EN_LEAK_CURR_STATE current_state;       /* RCMU run state */
    uint32_t cycle_sample_cont;        /* Samples per AC cycle */
    /* Fault detection related */
    uint8_t two_level_fault_count;          /* ~300mA threshold counter */
    uint8_t square_diff_150ma_count;         /* 150mA step counter */
    uint8_t square_diff_60ma_count;         /* 60mA step counter */
    uint8_t square_diff_30ma_count;         /* 30mA step counter */
    /* History storage */
    float history_150ma[3];            /* 0: current, 1: previous, 2: before previous */
    float history_60ma[3];            /* 0: current, 1: previous, 2: before previous */
    float history_30ma[3];            /* 0: current, 1: previous, 2: before previous */
    /* System flags */
    bool self_test_requested;
    bool fault_cleared;
    /* Output result */
    LeakResult_t result;
} LeakDetect_t;

void LeakCurrProcess(float leak_current_ma, uint32_t samp_freq, uint32_t inverter_freq);

void LeakCurrRequestSelfTest(void);  /* Request self-test */
bool LeakCurrFaultClare(void);        /* Clear fault and restart self-test */
int GetLeakCurrSelfTestState(void);
bool GetLeakCurrFaultFlag(void);
float GetLeakCurrValue(void);
float GetLeakCurrFilterValue(void);
#endif /* APP_RCMU_H_ */
