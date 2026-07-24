/*
 * mid_vol_and_rcmu_rms.c
 *
 * Relay middle voltage (3-phase) and RCMU leak current RMS calculation on CPU2.
 */
#include "mid_vol_and_rcmu_rms.h"
#include "mid_filter.h"

#pragma CODE_SECTION(MidVolAndRcmuRms_Run, ".TI.ramfunc");
#pragma CODE_SECTION(MidVolAndRcmuRms_Update, ".TI.ramfunc");

MidVolAndRcmuRms_t gstMidVolAndRcmuRms;

static Mid_Filter1OrderButter_t _gLowfilter_1order;

#define LEAK_CURRENT_LPF_CUTOFF_HZ (500.0f)
#define LEAK_CURRENT_SAMPLE_FREQ_HZ (20000.0f)
static uint16_t s_cycle_sample_count = 0;
static float32 s_leak_curr_filter = 0.0f;
void MidVolAndRcmuRms_Init(void)
{
    s_cycle_sample_count = 0;
    gstMidVolAndRcmuRms.mid_vol_a_rms = 0.0f;
    gstMidVolAndRcmuRms.mid_vol_b_rms = 0.0f;
    gstMidVolAndRcmuRms.mid_vol_c_rms = 0.0f;
    gstMidVolAndRcmuRms.rcmu_curr_rms = 0.0f;
    Mid_Filter1OrderButter_Init(&_gLowfilter_1order, LEAK_CURRENT_LPF_CUTOFF_HZ, LEAK_CURRENT_SAMPLE_FREQ_HZ);
}

void MidVolAndRcmuRms_Run(float32 mid_vol_a, float32 mid_vol_b, float32 mid_vol_c,
                          float32 leak_curr, float32 freq)
{
    static uint16_t s_cycle_count = 0;
    static float32 leak_rms_sum = 0.0f;
    static float32 mid_vol_a_rms_sum = 0.0f;
    static float32 mid_vol_b_rms_sum = 0.0f;
    static float32 mid_vol_c_rms_sum = 0.0f;
    s_leak_curr_filter = Mid_Filter1OrderButter_Update(&_gLowfilter_1order, leak_curr);
    leak_rms_sum += s_leak_curr_filter * s_leak_curr_filter;
    mid_vol_a_rms_sum += mid_vol_a * mid_vol_a;
    mid_vol_b_rms_sum += mid_vol_b * mid_vol_b;
    mid_vol_c_rms_sum += mid_vol_c * mid_vol_c;
    s_cycle_count++;
    if (s_cycle_count >= s_cycle_sample_count)
    {
        gstMidVolAndRcmuRms.rcmu_curr_rms = leak_rms_sum / s_cycle_sample_count;
        gstMidVolAndRcmuRms.mid_vol_a_rms = mid_vol_a_rms_sum / s_cycle_sample_count;
        gstMidVolAndRcmuRms.mid_vol_b_rms = mid_vol_b_rms_sum / s_cycle_sample_count;
        gstMidVolAndRcmuRms.mid_vol_c_rms = mid_vol_c_rms_sum / s_cycle_sample_count;
        leak_rms_sum = 0.0f;
        mid_vol_a_rms_sum = 0.0f;
        mid_vol_b_rms_sum = 0.0f;
        mid_vol_c_rms_sum = 0.0f;
        s_cycle_count = 0;
        s_cycle_sample_count = LEAK_CURRENT_SAMPLE_FREQ_HZ / ((freq > 0) ? ((freq < 30) ? 30 : ((freq > 70) ? 70 : freq)) :50);
    }
}

void MidVolAndRcmuRms_Update(DualDataShare_Ph3Ifc_t *ph3_ifc)
{
    ph3_ifc->rcmu_curr_rms = gstMidVolAndRcmuRms.rcmu_curr_rms;
    ph3_ifc->mid_vol_a_rms = gstMidVolAndRcmuRms.mid_vol_a_rms;
    ph3_ifc->mid_vol_b_rms = gstMidVolAndRcmuRms.mid_vol_b_rms;
    ph3_ifc->mid_vol_c_rms = gstMidVolAndRcmuRms.mid_vol_c_rms;
}
