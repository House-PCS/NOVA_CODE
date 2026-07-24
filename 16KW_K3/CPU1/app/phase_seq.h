#ifndef PHASE_SEQ_H
#define PHASE_SEQ_H
#include "F28x_Project.h"


//#define M_PI            3.14159265358979323846f
#define M_2PI           6.28318530717958647692f
/**
 * @brief 正相序 ABC 判断默认容差：±15°
 */
#define PHASE_SEQ_TOL_RAD_DEFAULT   (2.0f * M_PI / 360.0f * 15.0f)
/**
 * @brief 相序识别最低电压门槛，低于此值不做判断（防失锁误报）
 */
#define PHASE_SEQ_MIN_VOLT_RMS_DEFAULT  (50.0f)
/**
 * @brief 线序调换建议
 */
typedef enum {
    EN_PHASE_SEQ_SWAP_NONE = 0,   /**< 当前相序正确，无需调换 */
    EN_PHASE_SEQ_SWAP_BC,         /**< 调换 B、C 相线 */
    EN_PHASE_SEQ_SWAP_AB,         /**< 调换 A、B 相线 */
    EN_PHASE_SEQ_SWAP_AC,         /**< 调换 A、C 相线 */
    EN_PHASE_SEQ_SWAP_UNKNOWN     /**< 无法判定 */
} EN_PHASE_SEQ_SWAP;
/**
 * @brief 相序识别结果
 */
typedef struct phase_seq_result_stru_t {
    uint8_t  is_valid;        /**< 1: 结果有效; 0: 电压过低/输入无效 */
    uint8_t  is_normal;       /**< 1: 正相序 ABC; 0: 非正相序 */
    uint8_t  swap_advice;     /**< EN_STS_PHASE_SEQ_SWAP */
    float32 err_b_rad;       /**< B 相角误差 (rad), 相对正相序期望值 */
    float32 err_c_rad;       /**< C 相角误差 (rad), 相对正相序期望值 */
    float32 exp_theta_b;     /**< 正相序下 B 相期望角 (rad) */
    float32 exp_theta_c;     /**< 正相序下 C 相期望角 (rad) */
} PhaseSeq_Result_t;
/**
 * @brief 相序识别配置
 */
typedef struct phase_seq_cfg_stru_t {
    float32 tol_rad;         /**< 角度容差 (rad) */
    float32 min_volt_rms;    /**< 最低电压有效值 (V), 取三相最小值 */
} PhaseSeq_Cfg_t;
/**
 * @brief 初始化默认配置
 */
void PhaseSeq_CfgInit(PhaseSeq_Cfg_t* cfg);
/**
 * @brief 由 A 相角推算正相序 ABC 下 B/C 的期望相位
 *
 * 正相序：B 滞后 A 120°，C 滞后 A 240°
 * 等价于：θ_B = θ_A + 4π/3, θ_C = θ_A + 2π/3 (mod 2π)
 */
void PhaseSeq_ExpectedFromA(float32 theta_a,
                            float32* expected_theta_b,
                            float32* expected_theta_c);
/**
 * @brief 判断三相是否为正相序 ABC
 *
 * @return 1: 正相序; 0: 非正相序
 */
uint8_t PhaseSeq_IsNormal(float32 theta_a,
                            float32 theta_b,
                            float32 theta_c,
                            float32 tol_rad);
/**
 * @brief 获取线序调换建议
 *
 * @return EN_STS_PHASE_SEQ_SWAP
 */
uint8_t PhaseSeq_GetSwapAdvice(float32 theta_a,
                                 float32 theta_b,
                                 float32 theta_c,
                                 float32 tol_rad);
/**
 * @brief 完整相序识别（含有效性、误差、调换建议）
 *
 * @param theta_a/b/c  三相瞬时相位 (rad)
 * @param volt_rms_a/b/c 三相电压有效值 (V)，用于有效性判断
 * @param cfg          配置，传 NULL 则使用默认配置
 * @param result       输出结果
 */
void Sts_PhaseSeq_Analyze(float32 theta_a,
                          float32 theta_b,
                          float32 theta_c,
                          float32 volt_rms_a,
                          float32 volt_rms_b,
                          float32 volt_rms_c,
                          const PhaseSeq_Cfg_t* cfg,
                          PhaseSeq_Result_t* result,
                          uint8_t clear_flag);


extern PhaseSeq_Cfg_t _gstGridPhaseSeq;
extern PhaseSeq_Cfg_t _gstGenPhaseSeq;
extern PhaseSeq_Result_t _gstGridPhaseSeqResult;
extern PhaseSeq_Result_t _gstGenPhaseSeqResult;
#endif /* PHASE_SEQ_H */
