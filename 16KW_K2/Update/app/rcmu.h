/*
 * rcmu.h
 *
 *  Created on: 2025年10月30日
 *      Author: Nova006348
 */
#include "F28x_Project.h"
#include "system.h"
#include <string.h>
#include <math.h>
#ifndef APP_RCMU_H_
#define APP_RCMU_H_


/* RMS计算宏定义 */
#define LEAK_CURRENT_CALCULATE_RMS(sum_squares, sample_count) \
    ((sample_count) == 0 ? 0.0f : sqrtf((sum_squares) / (sample_count)))

/* 历史值更新 */
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



/* 系统配置宏 - 便于平台化移植 */
#define LEAK_CURRENT_SELF_TEST_THRESHOLD_MA     30.0f  /* 自检阈值30mA */
#define LEAK_CURRENT_SELF_TEST_RANGE_MIN_MA     40.0f  /* 自检范围最小值30mA */
#define LEAK_CURRENT_SELF_TEST_RANGE_MAX_MA     60.0f  /* 自检范围最大值70mA */
#define LEAK_CURRENT_SELF_TEST_FAULT_COUNT      5U     /* 自检故障计数阈值 */

#define LEAK_CURRENT_INIT_DELAY_TIME          250U      /* 初始化延迟5s(250*0.02) */

//#define LEAK_CURRENT_FAULT_THRESHOLD_THREE    120.0f /* 立即故障阈值120mA */




#define LEAK_CURRENT_FAULT_THRESHOLD_TWO                 280.0f  /* 300mA故障阈值 */
#define LEAK_CURRENT_TWO_LEVEL_FAULT_THRESHOLD           12U     /* 300mA故障计数阈值 */

#define LEAK_CURRENT_SQUARE_DIFF_150MA_THRESHOLD        148.0f   /* 300mA故障阈值 */
#define LEAK_CURRENT_SQUARE_DIFF_150MA_COUNT           1U     /* 300mA故障计数阈值 */

#define LEAK_CURRENT_SQUARE_DIFF_60MA_THRESHOLD        58.0f   /* 60mA平方差阈值 */
#define LEAK_CURRENT_SQUARE_DIFF_60MA_COUNT     3U     /* 58mA平方差计数阈值 */

/* 故障计数配置 */
#define LEAK_CURRENT_SQUARE_DIFF_30MA_THRESHOLD        27.0f /* 30mA平方差阈值 */
#define LEAK_CURRENT_SQUARE_DIFF_30MA_COUNT     6U     /* 30mA平方差计数阈值 */

#define LEAK_CURRENT_LowPass_W0 500.0f       //截止频率500hZ
/* 状态枚举 */
typedef enum
{
    EN_LEAK_CURR_STATE_PRE_CHK = 0,   /* 自检关闭IO阶段 */
    EN_LEAK_CURR_STATE_SELF_CHK,         /* 自检打开IO阶段 */
    EN_LEAK_CURR_STATE_RUN_DELAY,             /* 初始化延迟阶段 */
    EN_LEAK_CURR_STATE_RUN_NOMAL,       /* 正常运行阶段 */
    EN_LEAK_CURR_STATE_FAULT                   /* 故障状态 */
} EN_LEAK_CURR_STATE;

/* 漏电流检测结果结构体 */
typedef struct
{
    int16 self_test_passed;        /* 自检通过标志 */
    bool fault_flag;      /* 故障类型 */
    uint8_t leak_type;      //漏电流种类    
    float rms_current_ma;         /* 漏电流有效值(mA) */
    float instant_curr_ma;
    float avg_rms_current_ma;     /* 平均漏电流有效值(mA) */
} LeakResult_t;

/* 漏电流检测模块内部状态 */
typedef struct
{

    /* 自检相关 */
    uint8_t self_test_fault_count;     /*自检故障计数*/
    uint8_t self_test_success_count;   /*自检成功计数*/

    EN_LEAK_CURR_STATE current_state;       /*RCMU运行状态*/
    uint32_t cycle_sample_cont;        /* 每周期采样点数 */
    /* 故障检测相关 */
    uint8_t two_level_fault_count;          /*55mA阈值*/
    uint8_t square_diff_150ma_count;         /*150mA方差*/
    uint8_t square_diff_60ma_count;         /*60mA方差*/
    uint8_t square_diff_30ma_count;         /*30mA方差*/
    /* 历史值存储 */
    float history_150ma[3];            /* 0:当前, 1:上一次, 2:上上次 */
    float history_60ma[3];            /* 0:当前, 1:上一次, 2:上上次 */
    float history_30ma[3];            /* 0:当前, 1:上一次, 2:上上次 */
    /* 系统状态 */
    bool self_test_requested;
    bool fault_cleared;
    /* 输出结果 */
    LeakResult_t result;         //自检状态
} LeakDetect_t;
/*1阶巴特沃斯滤波器结构体*/
typedef struct RcmuLowPassfilter_1order_stru_t
{
   float32 a0;
   float32 a1;
   float32 b1;

   float32 yn;
   float32 yn_last1;
   float32 xn_last1;

} RcmuLowPassfilter_1order_t;
/* 函数声明 */
void LeakCurrProcess(float leak_current_ma, uint32_t samp_freq, uint32_t inverter_freq);

void LeakCurrRequestSelfTest(void);  /* 请求自检函数 */
bool LeakCurrFaultClare(void);        /* 清除故障函数 */
int GetLeakCurrSelfTestState(void);
bool GetLeakCurrFaultFlag(void);
float GetLeakCurrValue(void);
float GetLeakCurrFilterValue(void);
#endif /* APP_RCMU_H_ */
