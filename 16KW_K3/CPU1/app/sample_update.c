#include <sample_update.h>
#include "string.h"
#include "ntc.h"

#pragma DATA_SECTION(gstClaAdcResult,"CpuToCla1MsgRAM");

#pragma DATA_SECTION(gstAdcResult,"ramgs0");  /* CPU1/CPU2 shared GS3 RAM */
#pragma CODE_SECTION(GetAdcResult,".TI.ramfunc");
CpuAdc_t  gstAdcResult; /* ADC sampled results */
CpuAdc_t  gstAdcCoeff;  /* ADC conversion coefficients */
CpuAdcCalibration_t gstAdcZero;  /* ADC zero calibration data */
CLAAdc_t  gstClaAdcResult;
/**
 * @brief Initialize ADC conversion coefficients
 * @param none
 */
void AdcCoeffInit(void)
{
    /* Inverter side */
    gstAdcCoeff.inv_volt_phase_a            = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    gstAdcCoeff.inv_volt_phase_b            = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    gstAdcCoeff.inv_volt_phase_c            = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    gstAdcCoeff.inv_curr_phase_a            = (3.0f / 4096.0) / (0.0125f);
    gstAdcCoeff.inv_curr_phase_b            = (3.0f / 4096.0) / (0.0125f);
    gstAdcCoeff.inv_curr_phase_c            = (3.0f / 4096.0) / (0.0125f);
    gstAdcCoeff.leak_curr                   = (3.0f / 4096.0) * ((0.5f + 2.7f + 6.8f) / 6.8f) * 250.0f;
    gstAdcCoeff.load_vol_n_pe               = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    gstAdcCoeff.relay_middle_volt_phase_a   = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    gstAdcCoeff.relay_middle_volt_phase_b   = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    gstAdcCoeff.relay_middle_volt_phase_c   = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    /* Grid side */
    gstAdcCoeff.grid_volt_phase_a           = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    gstAdcCoeff.grid_volt_phase_b           = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    gstAdcCoeff.grid_volt_phase_c           = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    gstAdcCoeff.grid_curr_phase_a           = (3.0f / 4096.0) / (0.0125f);
    gstAdcCoeff.grid_curr_phase_b           = (3.0f / 4096.0) / (0.0125f);
    gstAdcCoeff.grid_curr_phase_c           = (3.0f / 4096.0) / (0.0125f);
    gstAdcCoeff.ct_a_curr                   = (3.0f / 4096.0) * (1000.0f) /8.2f;
    gstAdcCoeff.ct_b_curr                   = (3.0f / 4096.0) * (1000.0f) /8.2f;
    gstAdcCoeff.ct_c_curr                   = (3.0f / 4096.0) * (1000.0f) /8.2f;
    /* Load side */
    gstAdcCoeff.load_volt_phase_a           = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    gstAdcCoeff.load_volt_phase_b           = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    gstAdcCoeff.load_volt_phase_c           = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    gstAdcCoeff.load_curr_phase_a           = (3.0f / 4096.0) / (0.0125f);
    gstAdcCoeff.load_curr_phase_b           = (3.0f / 4096.0) / (0.0125f);
    gstAdcCoeff.load_curr_phase_c           = (3.0f / 4096.0) / (0.0125f);
    /* PV side */
    gstAdcCoeff.pv1_volt                    = (3.0f / 4096.0) * ((750.0f * 7.0f + 62.0f) / 6.8f);
    gstAdcCoeff.pv2_volt                    = (3.0f / 4096.0) * ((750.0f * 7.0f + 62.0f) / 6.8f);
    gstAdcCoeff.pv3_volt                    = (3.0f / 4096.0) * ((750.0f * 7.0f + 62.0f) / 6.8f);
    gstAdcCoeff.pv4_volt                    = (3.0f / 4096.0) * ((750.0f * 7.0f + 62.0f) / 6.8f);
    gstAdcCoeff.pv1_curr                    = (3.0f / 4096.0) / (0.05f);
    gstAdcCoeff.pv2_curr                    = (3.0f / 4096.0) / (0.05f);
    gstAdcCoeff.pv3_curr                    = (3.0f / 4096.0) / (0.05f);
    gstAdcCoeff.pv4_curr                    = (3.0f / 4096.0) / (0.05f);
    /* BUS side */
    gstAdcCoeff.bus_volt_pos                = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    gstAdcCoeff.bus_volt_neg                = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    gstAdcCoeff.bus_curr                    = (3.0f / 4096.0) * ((6.8f + 2.7f) / 6.8f) / (0.05f);
    /* Generator side */
    gstAdcCoeff.gen_volt_phase_a            = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    gstAdcCoeff.gen_volt_phase_b            = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    gstAdcCoeff.gen_volt_phase_c            = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    gstAdcCoeff.gen_curr_phase_a            = (3.0f / 4096.0) / (0.0125f);
    gstAdcCoeff.gen_curr_phase_b            = (3.0f / 4096.0) / (0.0125f);
    gstAdcCoeff.gen_curr_phase_c            = (3.0f / 4096.0) / (0.0125f);
    /* Temperature channels */
    gstAdcCoeff.Temp_A0                     = (3.0f / 4096.0);
    gstAdcCoeff.Temp_A1                     = (3.0f / 4096.0);
    gstAdcCoeff.Temp_A2                     = (3.0f / 4096.0);
    gstAdcCoeff.Temp_A3                     = (3.0f / 4096.0);
    gstAdcCoeff.Temp_A4                     = (3.0f / 4096.0);
    gstAdcCoeff.Temp_A5                     = (3.0f / 4096.0);
    gstAdcCoeff.Temp_A6                     = (3.0f / 4096.0);
    gstAdcCoeff.Temp_A7                     = (3.0f / 4096.0);
}
/**
* @brief ADC Time0 interrupt zero calibration
* @param void
* @return Nothing
* @note Zero calibration for channels sampled in every interrupt (Time0 slot)
*/
void Adc_Time0_Calibration_Zero(void)
{
    static uint32_t calibration_time_cnt = 0;

    calibration_time_cnt++;
    if(calibration_time_cnt > 80000) /* About 1 second at 80kHz CPU interrupt */
    {
        calibration_time_cnt = 0;
        gstAdcZero.calibration_time0_finish = 1;
    }
    /* Zero calibration in progress */
    if(gstAdcZero.calibration_time0_finish == 0)
    {
        gstAdcZero.common_calibration_zero  = 0.95f *gstAdcZero.common_calibration_zero   + 0.05f *AdcdResultRegs.ADCRESULT12;        /* 1.5V bias channel filtered average */
        /* Inverter side current zero */
        gstAdcZero.inv_volt_phase_a_zero    = gstAdcZero.common_calibration_zero;
        gstAdcZero.inv_volt_phase_b_zero    = gstAdcZero.common_calibration_zero;
        gstAdcZero.inv_volt_phase_c_zero    = gstAdcZero.common_calibration_zero;
        gstAdcZero.inv_curr_phase_a_zero    = 0.95f *gstAdcZero.inv_curr_phase_a_zero     + 0.05f *AdcaResultRegs.ADCRESULT1;
        gstAdcZero.inv_curr_phase_b_zero    = 0.95f *gstAdcZero.inv_curr_phase_b_zero     + 0.05f *AdcbResultRegs.ADCRESULT1;
        gstAdcZero.inv_curr_phase_c_zero    = 0.95f *gstAdcZero.inv_curr_phase_c_zero     + 0.05f *AdcdResultRegs.ADCRESULT1;

        gstAdcZero.ct_a_curr_zero           = gstAdcZero.common_calibration_zero;
        gstAdcZero.ct_b_curr_zero           = gstAdcZero.common_calibration_zero;
        gstAdcZero.ct_c_curr_zero           = gstAdcZero.common_calibration_zero;
        gstAdcZero.leak_curr_zero           = gstAdcZero.common_calibration_zero;
    }
    else /* Zero calibration finished, clamp offsets */
    {
        gstAdcZero.common_calibration_zero  = VALUE_LIMIT(gstAdcZero.common_calibration_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        /* Inverter side */
        gstAdcZero.inv_volt_phase_a_zero    = VALUE_LIMIT(gstAdcZero.inv_volt_phase_a_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.inv_volt_phase_b_zero    = VALUE_LIMIT(gstAdcZero.inv_volt_phase_b_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.inv_volt_phase_c_zero    = VALUE_LIMIT(gstAdcZero.inv_volt_phase_c_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.inv_curr_phase_a_zero    = VALUE_LIMIT(gstAdcZero.inv_curr_phase_a_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.inv_curr_phase_b_zero    = VALUE_LIMIT(gstAdcZero.inv_curr_phase_b_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.inv_curr_phase_c_zero    = VALUE_LIMIT(gstAdcZero.inv_curr_phase_c_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);

        gstAdcZero.ct_a_curr_zero           = VALUE_LIMIT(gstAdcZero.ct_a_curr_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.ct_b_curr_zero           = VALUE_LIMIT(gstAdcZero.ct_b_curr_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.ct_c_curr_zero           = VALUE_LIMIT(gstAdcZero.ct_c_curr_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.leak_curr_zero           = VALUE_LIMIT(gstAdcZero.leak_curr_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
    }
}
/**
* @brief ADC Time1 interrupt zero calibration
* @param void
* @return Nothing
* @note Zero calibration for channels sampled in Time1 slot
*/
void Adc_Time1_Calibration_Zero(void)
{
    static uint32_t calibration_time_cnt = 0;

    calibration_time_cnt++;
    if(calibration_time_cnt > 80000) /* About 1 second at 20kHz Time1 slot rate */
    {
        calibration_time_cnt = 0;
        gstAdcZero.calibration_time1_finish = 1;
    }
    /* Zero calibration in progress */
    if(gstAdcZero.calibration_time1_finish == 0)
    {
        gstAdcZero.grid_volt_phase_a_zero   = gstAdcZero.common_calibration_zero;
        gstAdcZero.grid_volt_phase_b_zero   = gstAdcZero.common_calibration_zero;
        gstAdcZero.grid_volt_phase_c_zero   = gstAdcZero.common_calibration_zero;
        gstAdcZero.grid_curr_phase_a_zero   = 0.95f *gstAdcZero.grid_curr_phase_a_zero   + 0.05f *AdccResultRegs.ADCRESULT10;
        gstAdcZero.grid_curr_phase_b_zero   = 0.95f *gstAdcZero.grid_curr_phase_b_zero   + 0.05f *AdccResultRegs.ADCRESULT6;
        gstAdcZero.grid_curr_phase_c_zero   = 0.95f *gstAdcZero.grid_curr_phase_c_zero   + 0.05f *AdcdResultRegs.ADCRESULT2;
        gstAdcZero.pv1_volt_zero            = gstAdcZero.common_calibration_zero;
        gstAdcZero.pv1_curr_zero            = 0.95f *gstAdcZero.pv1_curr_zero            + 0.05f *AdcaResultRegs.ADCRESULT2;
        gstAdcZero.bus_volt_pos_zero        = gstAdcZero.common_calibration_zero;
    }
    else /* Zero calibration finished, clamp offsets */
    {
        /* TIME1 */
        gstAdcZero.grid_volt_phase_a_zero   = VALUE_LIMIT(gstAdcZero.grid_volt_phase_a_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.grid_volt_phase_b_zero   = VALUE_LIMIT(gstAdcZero.grid_volt_phase_b_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.grid_volt_phase_c_zero   = VALUE_LIMIT(gstAdcZero.grid_volt_phase_c_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.grid_curr_phase_a_zero   = VALUE_LIMIT(gstAdcZero.grid_curr_phase_a_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.grid_curr_phase_b_zero   = VALUE_LIMIT(gstAdcZero.grid_curr_phase_b_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.grid_curr_phase_c_zero   = VALUE_LIMIT(gstAdcZero.grid_curr_phase_c_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.pv1_volt_zero            = VALUE_LIMIT(gstAdcZero.pv1_volt_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.pv1_curr_zero            = VALUE_LIMIT(gstAdcZero.pv1_curr_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.bus_volt_pos_zero        = VALUE_LIMIT(gstAdcZero.bus_volt_pos_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
    }
}
/**
* @brief ADC Time2 interrupt zero calibration
* @param void
* @return Nothing
* @note Zero calibration for channels sampled in Time2 slot
*/
void Adc_Time2_Calibration_Zero(void)
{
    static uint32_t calibration_time_cnt = 0;

    calibration_time_cnt++;
    if(calibration_time_cnt > 80000) /* About 1 second at 20kHz Time2 slot rate */
    {
        calibration_time_cnt = 0;
        gstAdcZero.calibration_time2_finish = 1;
    }
    /* Zero calibration in progress */
    if(gstAdcZero.calibration_time2_finish == 0)
    {
        /* TIME2 */
        gstAdcZero.gen_volt_phase_a_zero    = gstAdcZero.common_calibration_zero;
        gstAdcZero.gen_volt_phase_b_zero    = gstAdcZero.common_calibration_zero;
        gstAdcZero.gen_volt_phase_c_zero    = gstAdcZero.common_calibration_zero;
        gstAdcZero.gen_curr_phase_a_zero    = 0.95f *gstAdcZero.gen_curr_phase_a_zero   + 0.05f *AdccResultRegs.ADCRESULT11;
        gstAdcZero.gen_curr_phase_b_zero    = 0.95f *gstAdcZero.gen_curr_phase_b_zero   + 0.05f *AdccResultRegs.ADCRESULT7;
        gstAdcZero.gen_curr_phase_c_zero    = 0.95f *gstAdcZero.gen_curr_phase_c_zero   + 0.05f *AdcdResultRegs.ADCRESULT3;
        gstAdcZero.pv2_volt_zero            = gstAdcZero.common_calibration_zero;
        gstAdcZero.pv2_curr_zero            = 0.95f *gstAdcZero.pv2_curr_zero           + 0.05f *AdcaResultRegs.ADCRESULT3;
        gstAdcZero.bus_volt_neg_zero        = gstAdcZero.common_calibration_zero;
    }
    else /* Zero calibration finished, clamp offsets */
    {
        /* TIME2 */
        gstAdcZero.gen_volt_phase_a_zero    = VALUE_LIMIT(gstAdcZero.gen_volt_phase_a_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.gen_volt_phase_b_zero    = VALUE_LIMIT(gstAdcZero.gen_volt_phase_b_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.gen_volt_phase_c_zero    = VALUE_LIMIT(gstAdcZero.gen_volt_phase_c_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.gen_curr_phase_a_zero    = VALUE_LIMIT(gstAdcZero.gen_curr_phase_a_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.gen_curr_phase_b_zero    = VALUE_LIMIT(gstAdcZero.gen_curr_phase_b_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.gen_curr_phase_c_zero    = VALUE_LIMIT(gstAdcZero.gen_curr_phase_c_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.pv2_volt_zero            = VALUE_LIMIT(gstAdcZero.pv2_volt_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.pv2_curr_zero            = VALUE_LIMIT(gstAdcZero.pv2_curr_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.bus_volt_neg_zero        = VALUE_LIMIT(gstAdcZero.bus_volt_neg_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
    }
}
/**
* @brief ADC Time3 interrupt zero calibration
* @param void
* @return Nothing
* @note Zero calibration for channels sampled in Time3 slot
*/
void Adc_Time3_Calibration_Zero(void)
{
    static uint32_t calibration_time_cnt = 0;

    calibration_time_cnt++;
    if(calibration_time_cnt > 80000) /* About 1 second at 20kHz Time3 slot rate */
    {
        calibration_time_cnt = 0;
        gstAdcZero.calibration_time3_finish = 1;
    }
    /* Zero calibration in progress */
    if(gstAdcZero.calibration_time3_finish == 0)
    {
        /* TIME3 */
        gstAdcZero.load_volt_phase_a_zero   = gstAdcZero.common_calibration_zero;
        gstAdcZero.load_volt_phase_b_zero   = gstAdcZero.common_calibration_zero;
        gstAdcZero.load_volt_phase_c_zero   = gstAdcZero.common_calibration_zero;
        gstAdcZero.load_curr_phase_a_zero   = 0.95f *gstAdcZero.load_curr_phase_a_zero   + 0.05f *AdccResultRegs.ADCRESULT12;
        gstAdcZero.load_curr_phase_b_zero   = 0.95f *gstAdcZero.load_curr_phase_b_zero   + 0.05f *AdccResultRegs.ADCRESULT8;
        gstAdcZero.load_curr_phase_c_zero   = 0.95f *gstAdcZero.load_curr_phase_c_zero   + 0.05f *AdcdResultRegs.ADCRESULT4;
        gstAdcZero.pv3_volt_zero            = gstAdcZero.common_calibration_zero;
        gstAdcZero.pv3_curr_zero            = 0.95f *gstAdcZero.pv3_curr_zero            + 0.05f *AdcaResultRegs.ADCRESULT4;
        gstAdcZero.bus_curr_zero            = gstAdcZero.common_calibration_zero;
    }
    else /* Zero calibration finished, clamp offsets */
    {
        /* TIME3 */
        gstAdcZero.load_volt_phase_a_zero   = VALUE_LIMIT(gstAdcZero.load_volt_phase_a_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.load_volt_phase_b_zero   = VALUE_LIMIT(gstAdcZero.load_volt_phase_b_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.load_volt_phase_c_zero   = VALUE_LIMIT(gstAdcZero.load_volt_phase_c_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.load_curr_phase_a_zero   = VALUE_LIMIT(gstAdcZero.load_curr_phase_a_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.load_curr_phase_b_zero   = VALUE_LIMIT(gstAdcZero.load_curr_phase_b_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.load_curr_phase_c_zero   = VALUE_LIMIT(gstAdcZero.load_curr_phase_c_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.pv3_volt_zero            = VALUE_LIMIT(gstAdcZero.pv3_volt_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.pv3_curr_zero            = VALUE_LIMIT(gstAdcZero.pv3_curr_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.bus_curr_zero            = VALUE_LIMIT(gstAdcZero.bus_curr_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
    }
}
/**
* @brief ADC Time4 interrupt zero calibration
* @param void
* @return Nothing
* @note Zero calibration for channels sampled in Time4 slot
*/
void Adc_Time4_Calibration_Zero(void)
{
    static uint32_t calibration_time_cnt = 0;

    calibration_time_cnt++;
    if(calibration_time_cnt > 80000) /* About 1 second at 20kHz Time4 slot rate */
    {
        calibration_time_cnt = 0;
        gstAdcZero.calibration_time4_finish = 1;
    }
    /* Zero calibration in progress */
    if(gstAdcZero.calibration_time4_finish == 0)
    {
        /* TIME4 */
        gstAdcZero.relay_middle_volt_phase_a_zero = gstAdcZero.common_calibration_zero;
        gstAdcZero.relay_middle_volt_phase_b_zero = gstAdcZero.common_calibration_zero;
        gstAdcZero.relay_middle_volt_phase_c_zero = gstAdcZero.common_calibration_zero;
        gstAdcZero.pv4_volt_zero            = gstAdcZero.common_calibration_zero;
        gstAdcZero.pv4_curr_zero            = 0.95f *gstAdcZero.pv4_curr_zero            + 0.05f *AdcaResultRegs.ADCRESULT5;
    }
    else /* Zero calibration finished, clamp offsets */
    {
        /* TIME4 */
        gstAdcZero.relay_middle_volt_phase_a_zero = VALUE_LIMIT(gstAdcZero.relay_middle_volt_phase_a_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.relay_middle_volt_phase_b_zero = VALUE_LIMIT(gstAdcZero.relay_middle_volt_phase_b_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.relay_middle_volt_phase_c_zero = VALUE_LIMIT(gstAdcZero.relay_middle_volt_phase_c_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.pv4_volt_zero            = VALUE_LIMIT(gstAdcZero.pv4_volt_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.pv4_curr_zero            = VALUE_LIMIT(gstAdcZero.pv4_curr_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
    }
}
/**
* @brief Read ADC results according to CPU interrupt time slot
* @param void
* @return Nothing
* @note CPU interrupt time slot switch handler
*/
void GetAdcResult(EN_CPU_INTERRUPT_TIME value)
{
    static bool first_run = true;
    if(first_run)
    {
        AdcCoeffInit();
        memset((uint16_t *)&gstAdcZero, 2048, (int)sizeof(CpuAdcCalibration_t));
        gstAdcZero.calibration_time0_finish = 0;
        gstAdcZero.calibration_time1_finish = 0;
        gstAdcZero.calibration_time2_finish = 0;
        gstAdcZero.calibration_time3_finish = 0;
        gstAdcZero.calibration_time4_finish = 0;
        first_run = false;
    }
    if(gstAdcZero.calibration_time0_finish)
    {
            /* Channels sampled in every interrupt (Time0 slot) */
         gstAdcResult.inv_volt_phase_a    = gstAdcCoeff.inv_volt_phase_a    * (int16_t)(AdcaResultRegs.ADCRESULT0  - gstAdcZero.inv_volt_phase_a_zero);
         gstAdcResult.inv_volt_phase_b    = gstAdcCoeff.inv_volt_phase_b    * (int16_t)(AdcbResultRegs.ADCRESULT0  - gstAdcZero.inv_volt_phase_b_zero);
         gstAdcResult.inv_volt_phase_c    = gstAdcCoeff.inv_volt_phase_c    * (int16_t)(AdcdResultRegs.ADCRESULT0  - gstAdcZero.inv_volt_phase_c_zero);
         gstAdcResult.inv_curr_phase_a    = -1 * gstAdcCoeff.inv_curr_phase_a    * (int16_t)(AdcaResultRegs.ADCRESULT1  - gstAdcZero.inv_curr_phase_a_zero);
         gstAdcResult.inv_curr_phase_b    = -1 * gstAdcCoeff.inv_curr_phase_b    * (int16_t)(AdcbResultRegs.ADCRESULT1  - gstAdcZero.inv_curr_phase_b_zero);
         gstAdcResult.inv_curr_phase_c    = -1 * gstAdcCoeff.inv_curr_phase_c    * (int16_t)(AdcdResultRegs.ADCRESULT1  - gstAdcZero.inv_curr_phase_c_zero);

         gstAdcResult.ct_a_curr           = gstAdcCoeff.ct_a_curr           * (int16_t)(AdcaResultRegs.ADCRESULT14 - gstAdcZero.ct_a_curr_zero);
         gstAdcResult.ct_b_curr           = gstAdcCoeff.ct_b_curr           * (int16_t)(AdcbResultRegs.ADCRESULT2  - gstAdcZero.ct_b_curr_zero);
         gstAdcResult.ct_c_curr           = gstAdcCoeff.ct_c_curr           * (int16_t)(AdcdResultRegs.ADCRESULT6  - gstAdcZero.ct_c_curr_zero);
         gstAdcResult.leak_curr           = gstAdcCoeff.leak_curr           * (int16_t)(AdcdResultRegs.ADCRESULT7  - gstAdcZero.leak_curr_zero);

         gstClaAdcResult.inv_volt_phase_a = gstAdcResult.inv_volt_phase_a ;
         gstClaAdcResult.inv_volt_phase_b = gstAdcResult.inv_volt_phase_b ;
         gstClaAdcResult.inv_volt_phase_c = gstAdcResult.inv_volt_phase_c ;
         gstClaAdcResult.inv_curr_phase_a = gstAdcResult.inv_curr_phase_a;
         gstClaAdcResult.inv_curr_phase_b = gstAdcResult.inv_curr_phase_b;
         gstClaAdcResult.inv_curr_phase_c = gstAdcResult.inv_curr_phase_c;
         gstClaAdcResult.bus_volt_pos = gstAdcResult.bus_volt_pos;
         gstClaAdcResult.bus_volt_neg = gstAdcResult.bus_volt_neg;
    }
    /* Dispatch zero calibration and sampling by interrupt time slot */
    switch(value)
    {   case EN_CPU_INTERRUPT_TIME_0: /* Interrupt slot 0: Time0 calibration and always-sampled channels */
        {
            Adc_Time0_Calibration_Zero();
            break;
        }
        case EN_CPU_INTERRUPT_TIME_1: /* Interrupt slot 1: S0 = 0, S1 = 0 */
        {
            if(gstAdcZero.calibration_time1_finish == 0)
            {
                Adc_Time1_Calibration_Zero();
            }
            else
            {
                gstAdcResult.bus_volt_neg      = gstAdcCoeff.bus_volt_neg      * (int16_t)(AdcdResultRegs.ADCRESULT8  - gstAdcZero.bus_volt_neg_zero);
                gstClaAdcResult.bus_volt_neg   = gstAdcResult.bus_volt_neg;
                gstAdcResult.grid_volt_phase_a = gstAdcCoeff.grid_volt_phase_a * (int16_t)(AdcaResultRegs.ADCRESULT10 - gstAdcZero.grid_volt_phase_a_zero);
                gstAdcResult.grid_volt_phase_b = gstAdcCoeff.grid_volt_phase_b * (int16_t)(AdcaResultRegs.ADCRESULT6  - gstAdcZero.grid_volt_phase_b_zero);
                gstAdcResult.grid_volt_phase_c = gstAdcCoeff.grid_volt_phase_c * (int16_t)(AdcbResultRegs.ADCRESULT3  - gstAdcZero.grid_volt_phase_c_zero);
                gstAdcResult.grid_curr_phase_a = gstAdcCoeff.grid_curr_phase_a * (int16_t)(AdccResultRegs.ADCRESULT10 - gstAdcZero.grid_curr_phase_a_zero);
                gstAdcResult.grid_curr_phase_b = gstAdcCoeff.grid_curr_phase_b * (int16_t)(AdccResultRegs.ADCRESULT6  - gstAdcZero.grid_curr_phase_b_zero);
                gstAdcResult.grid_curr_phase_c = gstAdcCoeff.grid_curr_phase_b * (int16_t)(AdcdResultRegs.ADCRESULT2  - gstAdcZero.grid_curr_phase_c_zero);
                gstAdcResult.pv1_volt          = gstAdcCoeff.pv1_volt          * (int16_t)(AdccResultRegs.ADCRESULT2  - gstAdcZero.pv1_volt_zero);
                gstAdcResult.pv1_curr          = gstAdcCoeff.pv1_curr          * (int16_t)(AdcaResultRegs.ADCRESULT2  - gstAdcZero.pv1_curr_zero);
            }

            break;
        }
        case EN_CPU_INTERRUPT_TIME_2: /* Interrupt slot 2: S0 = 0, S1 = 1 */
        {
            if(gstAdcZero.calibration_time2_finish == 0)
            {
                Adc_Time2_Calibration_Zero();
            }
            else
            {
                gstAdcResult.gen_volt_phase_a = gstAdcCoeff.gen_volt_phase_a * (int16_t)(AdcaResultRegs.ADCRESULT11 - gstAdcZero.gen_volt_phase_a_zero);
                gstAdcResult.gen_volt_phase_b = gstAdcCoeff.gen_volt_phase_b * (int16_t)(AdcaResultRegs.ADCRESULT7  - gstAdcZero.gen_volt_phase_b_zero);
                gstAdcResult.gen_volt_phase_c = gstAdcCoeff.gen_volt_phase_c * (int16_t)(AdcbResultRegs.ADCRESULT4  - gstAdcZero.gen_volt_phase_c_zero);
                gstAdcResult.gen_curr_phase_a = gstAdcCoeff.gen_curr_phase_a * (int16_t)(AdccResultRegs.ADCRESULT11 - gstAdcZero.gen_curr_phase_a_zero);
                gstAdcResult.gen_curr_phase_b = gstAdcCoeff.gen_curr_phase_b * (int16_t)(AdccResultRegs.ADCRESULT7  - gstAdcZero.gen_curr_phase_b_zero);
                gstAdcResult.gen_curr_phase_c = gstAdcCoeff.gen_curr_phase_c * (int16_t)(AdcdResultRegs.ADCRESULT3  - gstAdcZero.gen_curr_phase_c_zero);
                gstAdcResult.pv2_volt         = gstAdcCoeff.pv2_volt         * (int16_t)(AdccResultRegs.ADCRESULT3  - gstAdcZero.pv2_volt_zero);
                gstAdcResult.pv2_curr         = gstAdcCoeff.pv2_curr         * (int16_t)(AdcaResultRegs.ADCRESULT3  - gstAdcZero.pv2_curr_zero);
                gstAdcResult.bus_volt_pos     = gstAdcCoeff.bus_volt_pos     * (int16_t)(AdcdResultRegs.ADCRESULT9  - gstAdcZero.bus_volt_pos_zero);
                gstClaAdcResult.bus_volt_pos  = gstAdcResult.bus_volt_pos;
            }

            break;
        }
        case EN_CPU_INTERRUPT_TIME_3: /* Interrupt slot 3: S0 = 1, S1 = 0 */
        {
            if(gstAdcZero.calibration_time3_finish == 0)
            {
                Adc_Time3_Calibration_Zero();
            }
            else
            {
                gstAdcResult.load_volt_phase_a = gstAdcCoeff.load_volt_phase_a * (int16_t)(AdcaResultRegs.ADCRESULT12 - gstAdcZero.load_volt_phase_a_zero);
                gstAdcResult.load_volt_phase_b = gstAdcCoeff.load_volt_phase_b * (int16_t)(AdcaResultRegs.ADCRESULT8  - gstAdcZero.load_volt_phase_b_zero);
                gstAdcResult.load_volt_phase_c = gstAdcCoeff.load_volt_phase_c * (int16_t)(AdcbResultRegs.ADCRESULT5  - gstAdcZero.load_volt_phase_c_zero);
                gstAdcResult.load_curr_phase_a = gstAdcCoeff.load_curr_phase_a * (int16_t)(AdccResultRegs.ADCRESULT12 - gstAdcZero.load_curr_phase_a_zero);
                gstAdcResult.load_curr_phase_b = gstAdcCoeff.load_curr_phase_b * (int16_t)(AdccResultRegs.ADCRESULT8  - gstAdcZero.load_curr_phase_b_zero);
                gstAdcResult.load_curr_phase_c = gstAdcCoeff.load_curr_phase_c * (int16_t)(AdcdResultRegs.ADCRESULT4  - gstAdcZero.load_curr_phase_c_zero);
                gstAdcResult.pv3_volt          = gstAdcCoeff.pv3_volt          * (int16_t)(AdccResultRegs.ADCRESULT4  - gstAdcZero.pv3_volt_zero);
                gstAdcResult.pv3_curr          = gstAdcCoeff.pv3_curr          * (int16_t)(AdcaResultRegs.ADCRESULT4  - gstAdcZero.pv3_curr_zero);
                gstAdcResult.bus_curr          = gstAdcCoeff.bus_curr          * (int16_t)(AdcdResultRegs.ADCRESULT10  - gstAdcZero.bus_curr_zero);
            }

            break;
        }
        case EN_CPU_INTERRUPT_TIME_4: /* Interrupt slot 4: S0 = 1, S1 = 1 */
        {
            if(gstAdcZero.calibration_time4_finish == 0)
            {
                Adc_Time4_Calibration_Zero();
            }
            else
            {
                gstAdcResult.relay_middle_volt_phase_a = gstAdcCoeff.relay_middle_volt_phase_a * (int16_t)(AdcaResultRegs.ADCRESULT13 - gstAdcZero.relay_middle_volt_phase_a_zero);
                gstAdcResult.relay_middle_volt_phase_b = gstAdcCoeff.relay_middle_volt_phase_b * (int16_t)(AdcaResultRegs.ADCRESULT9  - gstAdcZero.relay_middle_volt_phase_b_zero);
                gstAdcResult.relay_middle_volt_phase_c = gstAdcCoeff.relay_middle_volt_phase_c * (int16_t)(AdcbResultRegs.ADCRESULT6  - gstAdcZero.relay_middle_volt_phase_c_zero);
    //            gstAdcResult.pv4_volt          = gstAdcCoeff.pv4_volt          * (int16_t)(AdccResultRegs.ADCRESULT5  - gstAdcZero.pv4_volt_zero);
    //            gstAdcResult.pv4_curr          = gstAdcCoeff.pv4_curr          * (int16_t)(AdcaResultRegs.ADCRESULT5  - gstAdcZero.pv4_curr_zero);
                gstAdcResult.pv4_volt          = 0;
                gstAdcResult.pv4_curr          = 0;
                gstAdcResult.load_vol_n_pe = gstAdcCoeff.load_vol_n_pe * (int16_t)(AdcdResultRegs.ADCRESULT11  - gstAdcZero.common_calibration_zero);
            }

            break;
        }
        default:
        {
            break;

        }

    }
}
void TempVolUpdate(void)              /* 1ms temperature and auxiliary voltage update */
{
    static uint8_t i = 0;
    if(!Get_Adc_Time0_Calibration_State())
    {
        return;
    }
    gstAdcResult.inv_temp = NTCGetFilterTemp(NTC_INV_TEMP);
    gstAdcResult.pv_temp = NTCGetFilterTemp(NTC_PV_TEMP);
    gstAdcResult.ctr_temp = NTCGetFilterTemp(NTC_CHASSIS_TEMP);
    switch(i)
    {
        case 0:
        {
            gstAdcResult.Temp_A5 = gstAdcCoeff.Temp_A5 * (int16_t)(AdcdResultRegs.ADCRESULT12);
            GpioDataRegs.GPACLEAR.bit.GPIO25  = 1;
            GpioDataRegs.GPACLEAR.bit.GPIO26  = 1;
            GpioDataRegs.GPACLEAR.bit.GPIO27  = 1;
            break;
        }
        case 1:
        {
            gstAdcResult.V1_5 = gstAdcCoeff.Temp_A0 * (int16_t)(AdcdResultRegs.ADCRESULT12);
            GpioDataRegs.GPASET.bit.GPIO25  = 1;
            GpioDataRegs.GPACLEAR.bit.GPIO26  = 1;
            GpioDataRegs.GPACLEAR.bit.GPIO27  = 1;
            break;
        }
        case 2:
        {
//            gstAdcResult.Temp_A1 = gstAdcCoeff.Temp_A1 * (int16_t)(AdcdResultRegs.ADCRESULT12);
            GpioDataRegs.GPACLEAR.bit.GPIO25  = 1;
            GpioDataRegs.GPASET.bit.GPIO26  = 1;
            GpioDataRegs.GPACLEAR.bit.GPIO27  = 1;
            break;
        }
        case 3:
        {
            gstAdcResult.Temp_A0 = gstAdcCoeff.Temp_A0 * (int16_t)(AdcdResultRegs.ADCRESULT12);
            GpioDataRegs.GPASET.bit.GPIO25  = 1;
            GpioDataRegs.GPASET.bit.GPIO26  = 1;
            GpioDataRegs.GPACLEAR.bit.GPIO27  = 1;
            break;
        }
        case 4:
        {
            gstAdcResult.Temp_A1 = gstAdcCoeff.Temp_A1 * (int16_t)(AdcdResultRegs.ADCRESULT12);
            GpioDataRegs.GPACLEAR.bit.GPIO25  = 1;
            GpioDataRegs.GPACLEAR.bit.GPIO26  = 1;
            GpioDataRegs.GPASET.bit.GPIO27  = 1;
            break;
        }
        case 5:
        {
            gstAdcResult.Temp_A2 = gstAdcCoeff.Temp_A2 * (int16_t)(AdcdResultRegs.ADCRESULT12);
            GpioDataRegs.GPASET.bit.GPIO25  = 1;
            GpioDataRegs.GPACLEAR.bit.GPIO26  = 1;
            GpioDataRegs.GPASET.bit.GPIO27  = 1;
            break;
        }
        case 6:
        {
            gstAdcResult.Temp_A3 = gstAdcCoeff.Temp_A3 * (int16_t)(AdcdResultRegs.ADCRESULT12);
            GpioDataRegs.GPACLEAR.bit.GPIO25  = 1;
            GpioDataRegs.GPASET.bit.GPIO26  = 1;
            GpioDataRegs.GPASET.bit.GPIO27  = 1;
            break;
        }
        case 7:
        {
            gstAdcResult.Temp_A4 = gstAdcCoeff.Temp_A4 * (int16_t)(AdcdResultRegs.ADCRESULT12);
            GpioDataRegs.GPASET.bit.GPIO25  = 1;
            GpioDataRegs.GPASET.bit.GPIO26  = 1;
            GpioDataRegs.GPASET.bit.GPIO27  = 1;
            break;
        }
        default :
        {
            GpioDataRegs.GPACLEAR.bit.GPIO25  = 1;
            GpioDataRegs.GPACLEAR.bit.GPIO26  = 1;
            GpioDataRegs.GPACLEAR.bit.GPIO27  = 1;
            break;
        }
    }
    if(i < 7)       /* MUX channel scan cycle */
    {
        i++;
    }
    else
    {
        /*********/
        NTCGetInVol(0,gstAdcResult.Temp_A1);
        NTCGetInVol(1,gstAdcResult.Temp_A2);
        NTCGetInVol(2,gstAdcResult.Temp_A4);
        NTCProcess();        /* NTC temperature filter processing */
        i = 0;
    }
}

uint16_t Get_Adc_Time0_Calibration_State(void)
{
    return gstAdcZero.calibration_time0_finish;
}
uint16_t Get_Adc_Time1_Calibration_State(void)
{
    return gstAdcZero.calibration_time1_finish;
}
uint16_t Get_Adc_Time2_Calibration_State(void)
{
    return gstAdcZero.calibration_time2_finish;
}
uint16_t Get_Adc_Time3_Calibration_State(void)
{
    return gstAdcZero.calibration_time3_finish;
}
uint16_t Get_Adc_Time4_Calibration_State(void)
{
    return gstAdcZero.calibration_time4_finish;
}
