/**
 * @file phase_seq.c
 * @brief 三相相序识别：正相序 ABC 判断与线序调换建议
 */

#include "phase_seq.h"
#include <math.h>

PhaseSeq_Cfg_t _gstGridPhaseSeq;
PhaseSeq_Cfg_t _gstGenPhaseSeq;
PhaseSeq_Result_t _gstGridPhaseSeqResult;
PhaseSeq_Result_t _gstGenPhaseSeqResult;

void PhaseSeq_CfgInit(PhaseSeq_Cfg_t* cfg)
{
    if(cfg == NULL) {
        return;
    }

    cfg->tol_rad = PHASE_SEQ_TOL_RAD_DEFAULT;
    cfg->min_volt_rms = PHASE_SEQ_MIN_VOLT_RMS_DEFAULT;
}

static float32 _PhaseSeq_NormTheta(float32 theta)
{
    while(theta >= M_2PI) {
        theta -= M_2PI;
    }
    while(theta < 0.0f) {
        theta += M_2PI;
    }
    return theta;
}

static float32 _PhaseSeq_WrapDelta(float32 delta)
{
    if(delta > M_PI) {
        delta -= M_2PI;
    } else if(delta < -M_PI) {
        delta += M_2PI;
    }
    return delta;
}

static uint8_t _PhaseSeq_IsNormalInternal(float32 theta_a,
                                            float32 theta_b,
                                            float32 theta_c,
                                            float32 tol_rad,
                                            float32* err_b,
                                            float32* err_c,
                                            float32* exp_b,
                                            float32* exp_c)
{
    float32 local_exp_b;
    float32 local_exp_c;
    float32 local_err_b;
    float32 local_err_c;

    theta_a = _PhaseSeq_NormTheta(theta_a);
    theta_b = _PhaseSeq_NormTheta(theta_b);
    theta_c = _PhaseSeq_NormTheta(theta_c);

    PhaseSeq_ExpectedFromA(theta_a, &local_exp_b, &local_exp_c);

    local_err_b = _PhaseSeq_WrapDelta(theta_b - local_exp_b);
    local_err_c = _PhaseSeq_WrapDelta(theta_c - local_exp_c);

    if(err_b != NULL) {
        *err_b = local_err_b;
    }
    if(err_c != NULL) {
        *err_c = local_err_c;
    }
    if(exp_b != NULL) {
        *exp_b = local_exp_b;
    }
    if(exp_c != NULL) {
        *exp_c = local_exp_c;
    }

    if((fabsf(local_err_b) <= tol_rad) && (fabsf(local_err_c) <= tol_rad)) {
        return 1U;
    }
    return 0U;
}

void PhaseSeq_ExpectedFromA(float32 theta_a,
                            float32* expected_theta_b,
                            float32* expected_theta_c)
{
    float32 theta_b;
    float32 theta_c;

    if((expected_theta_b == NULL) || (expected_theta_c == NULL)) {
        return;
    }

    theta_a = _PhaseSeq_NormTheta(theta_a);

    /* 与 period_count*2/3、period_count*1/3 同比例 */
    theta_b = theta_a + M_2PI * (2.0f / 3.0f);
    theta_c = theta_a + M_2PI * (1.0f / 3.0f);

    *expected_theta_b = _PhaseSeq_NormTheta(theta_b);
    *expected_theta_c = _PhaseSeq_NormTheta(theta_c);
}

uint8_t Sts_PhaseSeq_IsNormal(float32 theta_a,
                                float32 theta_b,
                                float32 theta_c,
                                float32 tol_rad)
{
    return _PhaseSeq_IsNormalInternal(theta_a,
        theta_b,
        theta_c,
        tol_rad,
        NULL,
        NULL,
        NULL,
        NULL);
}

uint8_t PhaseSeq_GetSwapAdvice(float32 theta_a,
                                 float32 theta_b,
                                 float32 theta_c,
                                 float32 tol_rad)
{
    /* 当前接线 A-B-C */
    if(_PhaseSeq_IsNormalInternal(theta_a, theta_b, theta_c, tol_rad, NULL, NULL, NULL, NULL) != 0U) {
        return EN_PHASE_SEQ_SWAP_NONE;
    }

    /* 对调 B、C：逻辑 A-C-B */
    if(_PhaseSeq_IsNormalInternal(theta_a, theta_c, theta_b, tol_rad, NULL, NULL, NULL, NULL) != 0U) {
        return EN_PHASE_SEQ_SWAP_BC;
    }

    /* 对调 A、B：逻辑 B-A-C */
    if(_PhaseSeq_IsNormalInternal(theta_b, theta_a, theta_c, tol_rad, NULL, NULL, NULL, NULL) != 0U) {
        return EN_PHASE_SEQ_SWAP_AB;
    }

    /* 对调 A、C：逻辑 C-B-A */
    if(_PhaseSeq_IsNormalInternal(theta_c, theta_b, theta_a, tol_rad, NULL, NULL, NULL, NULL) != 0U) {
        return EN_PHASE_SEQ_SWAP_AC;
    }

    return EN_PHASE_SEQ_SWAP_UNKNOWN;
}

void Sts_PhaseSeq_Analyze(float32 theta_a,
                          float32 theta_b,
                          float32 theta_c,
                          float32 volt_rms_a,
                          float32 volt_rms_b,
                          float32 volt_rms_c,
                          const PhaseSeq_Cfg_t* cfg,
                          PhaseSeq_Result_t* result,
                          uint8_t clear_flag)
{
    PhaseSeq_Cfg_t local_cfg;
    float32 min_volt;

    if(result == NULL) {
        return;
    }

    result->is_valid = 0U;
    result->is_normal = 0U;
    result->swap_advice = EN_PHASE_SEQ_SWAP_UNKNOWN;
    result->err_b_rad = 0.0f;
    result->err_c_rad = 0.0f;
    result->exp_theta_b = 0.0f;
    result->exp_theta_c = 0.0f;

    if(cfg == NULL || clear_flag)
    {
        PhaseSeq_CfgInit(&local_cfg);
        cfg = &local_cfg;
    }

    min_volt = volt_rms_a;
    if(volt_rms_b < min_volt)
    {
        min_volt = volt_rms_b;
    }
    if(volt_rms_c < min_volt)
    {
        min_volt = volt_rms_c;
    }

    if(min_volt < cfg->min_volt_rms)
    {
        return;
    }

    result->is_valid = 1U;
    result->is_normal = _PhaseSeq_IsNormalInternal(theta_a,
        theta_b,
        theta_c,
        cfg->tol_rad,
        &result->err_b_rad,
        &result->err_c_rad,
        &result->exp_theta_b,
        &result->exp_theta_c);

    if(result->is_normal != 0U)
    {
        result->swap_advice = EN_PHASE_SEQ_SWAP_NONE;
    } else
    {
        result->swap_advice = PhaseSeq_GetSwapAdvice(theta_a,
            theta_b,
            theta_c,
            cfg->tol_rad);
    }
}
