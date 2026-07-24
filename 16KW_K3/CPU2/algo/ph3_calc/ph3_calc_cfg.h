/*
 * ph3_calc_cfg.h
 *
 * 三相电量计算模块配置参数（CPU 实现，算法参数与 CPU1 控制环一致）
 */
#ifndef ALGO_PH3_CALC_CFG_H_
#define ALGO_PH3_CALC_CFG_H_

#define PH3_CALC_PI                 (3.1415926f)
#define PH3_CALC_M_2PI              (6.2831853f)
#define PH3_CALC_TS                 (0.00005f)   /* 控制周期 1/20kHz */
#define PH3_CALC_PLL_KP             (1.43f)
#define PH3_CALC_PLL_KI             (317.0f)
#define PH3_CALC_PLL_W_NOMINAL      (100.0f * PH3_CALC_PI)
#define PH3_CALC_PLL_W_LIMIT        (50.0f * PH3_CALC_PI)
#define PH3_CALC_PLL_FREQ_MIN       (40.0f)
#define PH3_CALC_PLL_FREQ_MAX       (70.0f)
#define PH3_CALC_RMS_COUNT_MIN      (250.0f)     /* 对应约 80Hz 上限 */
#define PH3_CALC_RMS_COUNT_MAX      (666.0f)     /* 对应约 30Hz 下限 */
#define PH3_CALC_POSNEG_BP_FREQ     (50.0f)      /* 正负序分离带通滤波中心频率 (Hz) */

#define PH3_CALC_VALUE_LIMIT(x, max_v, min_v) \
    (((x) > (max_v)) ? (max_v) : (((x) < (min_v)) ? (min_v) : (x)))

#endif /* ALGO_PH3_CALC_CFG_H_ */
