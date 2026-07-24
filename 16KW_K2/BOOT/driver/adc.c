/*
 * adc.c
 *
 *  Created on: 2025年1月22日
 *      Author: Nova007456
 */
#include "adc.h"

#pragma DATA_SECTION(gstAdcResult,"CpuToCla1MsgRAM");
CpuAdc_t  gstAdcResult; //ADC的采样结果
CpuAdc_t  gstAdcCoeff;  //ADC的采样系数
CpuAdcCalibration_t gstAdcZero;  //ADC的零点偏执值

/**
* @brief: Adc_Calibration_Zero
* @param: void
* @return:Nothing
* @note:ADC模块零点校准
*/
void Adc_Calibration_Zero(void)
{
    static uint32_t calibration_cnt = 0;

    calibration_cnt++;
    if(calibration_cnt > 320000)//校准8秒
    {
        calibration_cnt = 0;
        gstAdcZero.calibration_finish = 1;
    }
    //零点校准滤波
    if(gstAdcZero.calibration_finish == 0)
    {
        gstAdcZero.common_calibration_zero  = 0.8f *gstAdcZero.common_calibration_zero   + 0.2f *AdcaResultRegs.ADCRESULT0;
        gstAdcZero.inv_volt_zero    = 0.8f *gstAdcZero.inv_volt_zero     + 0.2f *AdcaResultRegs.ADCRESULT0;  //逆变电压
        // gstAdcZero.inv_volt_phase_b_zero    = 0.8f *gstAdcZero.inv_volt_phase_b_zero     + 0.2f *AdcbResultRegs.ADCRESULT0; //无用信号
        // gstAdcZero.inv_volt_phase_c_zero    = 0.8f *gstAdcZero.inv_volt_phase_c_zero     + 0.2f *AdcdResultRegs.ADCRESULT0; //无用信号
        gstAdcZero.inv_L_curr_zero    = 0.8f *gstAdcZero.inv_L_curr_zero     + 0.2f *AdcaResultRegs.ADCRESULT1;  //ADC_L_CURR
        // gstAdcZero.inv_curr_phase_b_zero    = 0.8f *gstAdcZero.inv_curr_phase_b_zero     + 0.2f *AdcbResultRegs.ADCRESULT1; //无用信号
        gstAdcZero.inv_curr_tol_zero    = 0.8f *gstAdcZero.inv_curr_tol_zero     + 0.2f *AdcdResultRegs.ADCRESULT1;  //ADC_CURR_TOL
        //不需校准的值则默认使用通用零点值
        gstAdcZero.ct_curr_zero           = gstAdcZero.common_calibration_zero;  //CT电流采样
        // gstAdcZero.ct_b_curr_zero           = gstAdcZero.common_calibration_zero;
        // gstAdcZero.ct_c_curr_zero           = gstAdcZero.common_calibration_zero;
        gstAdcZero.leak_curr_zero           = gstAdcZero.common_calibration_zero;
        // gstAdcZero.Grid_volt_phase_a_zero   = gstAdcZero.common_calibration_zero;  //NC
        // gstAdcZero.Grid_volt_phase_b_zero   = gstAdcZero.common_calibration_zero;  //NC
        // gstAdcZero.Grid_volt_phase_c_zero   = gstAdcZero.common_calibration_zero;  //NC
        // gstAdcZero.Grid_curr_phase_a_zero   = gstAdcZero.common_calibration_zero;  //NC
        // gstAdcZero.Grid_curr_phase_b_zero   = gstAdcZero.common_calibration_zero;  //NC
        gstAdcZero.Grid_curr_zero              = gstAdcZero.common_calibration_zero;     //电网侧电流
        gstAdcZero.pv1_volt_zero            = gstAdcZero.common_calibration_zero;
        gstAdcZero.pv1_curr_zero            = gstAdcZero.common_calibration_zero;
        gstAdcZero.bus_volt_all_zero        = gstAdcZero.common_calibration_zero;     //BUS+-电压
        // gstAdcZero.gen_volt_phase_a_zero    = gstAdcZero.common_calibration_zero;
        // gstAdcZero.gen_volt_phase_b_zero    = gstAdcZero.common_calibration_zero;
        gstAdcZero.gen_volt_zero            = gstAdcZero.common_calibration_zero;     //油机电压
        // gstAdcZero.gen_curr_phase_a_zero    = gstAdcZero.common_calibration_zero;
        // gstAdcZero.gen_curr_phase_b_zero    = gstAdcZero.common_calibration_zero;
        gstAdcZero.gen_curr_zero    = gstAdcZero.common_calibration_zero;     //油机电流
        gstAdcZero.pv2_volt_zero            = gstAdcZero.common_calibration_zero;
        gstAdcZero.pv2_curr_zero            = gstAdcZero.common_calibration_zero;
        gstAdcZero.bus_volt_pn_zero        = gstAdcZero.common_calibration_zero;     //BUS+N
        // gstAdcZero.load_volt_phase_a_zero   = gstAdcZero.common_calibration_zero;    //NC 
        // gstAdcZero.load_volt_phase_b_zero   = gstAdcZero.common_calibration_zero;    //NC
        gstAdcZero.load_volt_zero   = gstAdcZero.common_calibration_zero;     //负载电压
        // gstAdcZero.load_current_phase_a_zero= gstAdcZero.common_calibration_zero;
        // gstAdcZero.load_current_phase_b_zero= gstAdcZero.common_calibration_zero;
        gstAdcZero.load_current_zero= gstAdcZero.common_calibration_zero;     //负载电流
        gstAdcZero.pv3_volt_zero            = gstAdcZero.common_calibration_zero;
        gstAdcZero.pv3_curr_zero            = gstAdcZero.common_calibration_zero;
        gstAdcZero.bus_curr_zero            = gstAdcZero.common_calibration_zero;
        // gstAdcZero.relay_middle_volt_phase_a_zero = gstAdcZero.common_calibration_zero;
        // gstAdcZero.relay_middle_volt_phase_b_zero = gstAdcZero.common_calibration_zero;
        // gstAdcZero.relay_middle_volt_phase_c_zero = gstAdcZero.common_calibration_zero;
        gstAdcZero.pv4_volt_zero                  = gstAdcZero.common_calibration_zero;
        gstAdcZero.pv4_curr_zero                  = gstAdcZero.common_calibration_zero;
    }
    else//校准限幅
    {
        gstAdcZero.common_calibration_zero  = VALUE_LIMIT(gstAdcZero.common_calibration_zero,ADC_ZERO_MAX,ADC_ZERO_MIN);
        gstAdcZero.inv_volt_zero    = ADC_ZERO_LIMIT(gstAdcZero.inv_volt_zero,gstAdcZero.common_calibration_zero);
        // gstAdcZero.inv_volt_phase_b_zero    = ADC_ZERO_LIMIT(gstAdcZero.inv_volt_phase_b_zero,gstAdcZero.common_calibration_zero);
        // gstAdcZero.inv_volt_phase_c_zero    = ADC_ZERO_LIMIT(gstAdcZero.inv_volt_phase_c_zero,gstAdcZero.common_calibration_zero);
        gstAdcZero.inv_L_curr_zero    = ADC_ZERO_LIMIT(gstAdcZero.inv_L_curr_zero,gstAdcZero.common_calibration_zero);
        // gstAdcZero.inv_curr_phase_b_zero    = ADC_ZERO_LIMIT(gstAdcZero.inv_curr_phase_b_zero,gstAdcZero.common_calibration_zero);
        gstAdcZero.inv_curr_tol_zero    = ADC_ZERO_LIMIT(gstAdcZero.inv_curr_tol_zero,gstAdcZero.common_calibration_zero);
        gstAdcZero.ct_curr_zero           = ADC_ZERO_LIMIT(gstAdcZero.ct_curr_zero,gstAdcZero.common_calibration_zero);
        // gstAdcZero.ct_b_curr_zero           = ADC_ZERO_LIMIT(gstAdcZero.ct_b_curr_zero,gstAdcZero.common_calibration_zero);
        // gstAdcZero.ct_c_curr_zero           = ADC_ZERO_LIMIT(gstAdcZero.ct_c_curr_zero,gstAdcZero.common_calibration_zero);
        gstAdcZero.leak_curr_zero           = ADC_ZERO_LIMIT(gstAdcZero.leak_curr_zero,gstAdcZero.common_calibration_zero);
        // gstAdcZero.Grid_volt_phase_a_zero   = ADC_ZERO_LIMIT(gstAdcZero.Grid_volt_phase_a_zero,gstAdcZero.common_calibration_zero);
        // gstAdcZero.Grid_volt_phase_b_zero   = ADC_ZERO_LIMIT(gstAdcZero.Grid_volt_phase_b_zero,gstAdcZero.common_calibration_zero);
        // gstAdcZero.Grid_volt_phase_c_zero   = ADC_ZERO_LIMIT(gstAdcZero.Grid_volt_phase_c_zero,gstAdcZero.common_calibration_zero);
        // gstAdcZero.Grid_curr_phase_a_zero   = ADC_ZERO_LIMIT(gstAdcZero.Grid_curr_phase_a_zero,gstAdcZero.common_calibration_zero);
        // gstAdcZero.Grid_curr_phase_b_zero   = ADC_ZERO_LIMIT(gstAdcZero.Grid_curr_phase_b_zero,gstAdcZero.common_calibration_zero);
        gstAdcZero.Grid_curr_zero   = ADC_ZERO_LIMIT(gstAdcZero.Grid_curr_zero,gstAdcZero.common_calibration_zero);
        gstAdcZero.pv1_volt_zero            = ADC_ZERO_LIMIT(gstAdcZero.pv1_volt_zero,gstAdcZero.common_calibration_zero);
        gstAdcZero.pv1_curr_zero            = ADC_ZERO_LIMIT(gstAdcZero.pv1_curr_zero,gstAdcZero.common_calibration_zero);
        gstAdcZero.bus_volt_all_zero        = ADC_ZERO_LIMIT(gstAdcZero.bus_volt_all_zero,gstAdcZero.common_calibration_zero);
        // gstAdcZero.gen_volt_phase_a_zero    = ADC_ZERO_LIMIT(gstAdcZero.gen_volt_phase_a_zero,gstAdcZero.common_calibration_zero);
        // gstAdcZero.gen_volt_phase_b_zero    = ADC_ZERO_LIMIT(gstAdcZero.gen_volt_phase_b_zero,gstAdcZero.common_calibration_zero);
        gstAdcZero.gen_volt_zero    = ADC_ZERO_LIMIT(gstAdcZero.gen_volt_zero,gstAdcZero.common_calibration_zero);
        // gstAdcZero.gen_curr_phase_a_zero    = ADC_ZERO_LIMIT(gstAdcZero.gen_curr_phase_a_zero,gstAdcZero.common_calibration_zero);
        // gstAdcZero.gen_curr_phase_b_zero    = ADC_ZERO_LIMIT(gstAdcZero.gen_curr_phase_b_zero,gstAdcZero.common_calibration_zero);
        gstAdcZero.gen_curr_zero    = ADC_ZERO_LIMIT(gstAdcZero.gen_curr_zero,gstAdcZero.common_calibration_zero);
        gstAdcZero.pv2_volt_zero            = ADC_ZERO_LIMIT(gstAdcZero.pv2_volt_zero,gstAdcZero.common_calibration_zero);
        gstAdcZero.pv2_curr_zero            = ADC_ZERO_LIMIT(gstAdcZero.pv2_curr_zero,gstAdcZero.common_calibration_zero);
        gstAdcZero.bus_volt_pn_zero        = ADC_ZERO_LIMIT(gstAdcZero.bus_volt_pn_zero,gstAdcZero.common_calibration_zero);
        // gstAdcZero.load_volt_phase_a_zero   = ADC_ZERO_LIMIT(gstAdcZero.load_volt_phase_a_zero,gstAdcZero.common_calibration_zero);
        // gstAdcZero.load_volt_phase_b_zero   = ADC_ZERO_LIMIT(gstAdcZero.load_volt_phase_b_zero,gstAdcZero.common_calibration_zero);
        gstAdcZero.load_volt_zero   = ADC_ZERO_LIMIT(gstAdcZero.load_volt_zero,gstAdcZero.common_calibration_zero);
        // gstAdcZero.load_current_phase_a_zero= ADC_ZERO_LIMIT(gstAdcZero.load_current_phase_a_zero,gstAdcZero.common_calibration_zero);
        // gstAdcZero.load_current_phase_b_zero= ADC_ZERO_LIMIT(gstAdcZero.load_current_phase_b_zero,gstAdcZero.common_calibration_zero);
        gstAdcZero.load_current_zero= ADC_ZERO_LIMIT(gstAdcZero.load_current_zero,gstAdcZero.common_calibration_zero);
        gstAdcZero.pv3_volt_zero            = ADC_ZERO_LIMIT(gstAdcZero.pv3_volt_zero,gstAdcZero.common_calibration_zero);
        gstAdcZero.pv3_curr_zero            = ADC_ZERO_LIMIT(gstAdcZero.pv3_curr_zero,gstAdcZero.common_calibration_zero);
        gstAdcZero.bus_curr_zero            = ADC_ZERO_LIMIT(gstAdcZero.bus_curr_zero,gstAdcZero.common_calibration_zero);
        // gstAdcZero.relay_middle_volt_phase_a_zero = ADC_ZERO_LIMIT(gstAdcZero.relay_middle_volt_phase_a_zero,gstAdcZero.common_calibration_zero);
        // gstAdcZero.relay_middle_volt_phase_b_zero = ADC_ZERO_LIMIT(gstAdcZero.relay_middle_volt_phase_b_zero,gstAdcZero.common_calibration_zero);
        // gstAdcZero.relay_middle_volt_phase_c_zero = ADC_ZERO_LIMIT(gstAdcZero.relay_middle_volt_phase_c_zero,gstAdcZero.common_calibration_zero);
        gstAdcZero.pv4_volt_zero                  = ADC_ZERO_LIMIT(gstAdcZero.pv4_volt_zero,gstAdcZero.common_calibration_zero);
        gstAdcZero.pv4_curr_zero                  = ADC_ZERO_LIMIT(gstAdcZero.pv4_curr_zero,gstAdcZero.common_calibration_zero);
    }

}
/**
* @brief: CpuInterruptTimeSwitch
* @param: void
* @return:Nothing
* @note:CPU中断时间切换
*/
void GetAdcResult(EN_CPU_INTERRUPT_TIME value)
{
    /*      直接采样部分      */ //TODO:需更改变量命名
    gstAdcResult.inv_volt    = gstAdcCoeff.inv_volt    * (int16_t)(AdcaResultRegs.ADCRESULT0  - gstAdcZero.inv_volt_zero);
    // gstAdcResult.inv_volt_phase_b    = gstAdcCoeff.inv_volt_phase_b    * (int16_t)(AdcbResultRegs.ADCRESULT0  - gstAdcZero.inv_volt_phase_b_zero);
    // gstAdcResult.inv_volt_phase_c    = gstAdcCoeff.inv_volt_phase_c    * (int16_t)(AdcdResultRegs.ADCRESULT0  - gstAdcZero.inv_volt_phase_c_zero);
    // gstAdcResult.inv_L_curr    = gstAdcCoeff.inv_L_curr    * (int16_t)(AdcaResultRegs.ADCRESULT1  - gstAdcZero.inv_L_curr_zero);
    gstAdcResult.inv_L_curr    = (gstAdcResult.inv_L_curr1 + gstAdcResult.inv_L_curr2 + gstAdcResult.inv_L_curr3 + gstAdcResult.inv_L_curr4) / 4.0; 
    // gstAdcResult.inv_curr_phase_b    = gstAdcCoeff.inv_curr_phase_b    * (int16_t)(AdcbResultRegs.ADCRESULT1  - gstAdcZero.inv_curr_phase_b_zero);
    // gstAdcResult.inv_curr_tol    = gstAdcCoeff.inv_curr_tol    * (int16_t)(AdcdResultRegs.ADCRESULT1  - gstAdcZero.inv_curr_tol_zero); // T
    gstAdcResult.inv_curr_tol    = (gstAdcResult.inv_curr_tol1 + gstAdcResult.inv_curr_tol2 + gstAdcResult.inv_curr_tol3 + gstAdcResult.inv_curr_tol4 ) / 4.0; 
    gstAdcResult.ct_curr           = gstAdcCoeff.ct_curr           * (int16_t)(AdcaResultRegs.ADCRESULT14 - gstAdcZero.ct_curr_zero);
    // gstAdcResult.ct_b_curr           = gstAdcCoeff.ct_b_curr           * (int16_t)(AdcbResultRegs.ADCRESULT2  - gstAdcZero.ct_b_curr_zero);
    // gstAdcResult.ct_c_curr           = gstAdcCoeff.ct_c_curr           * (int16_t)(AdcdResultRegs.ADCRESULT6  - gstAdcZero.ct_c_curr_zero);
    gstAdcResult.leak_curr           = gstAdcCoeff.leak_curr           * (int16_t)(AdcdResultRegs.ADCRESULT7  - gstAdcZero.leak_curr_zero);

    /*      切换采样部分      */
    switch(value)
    {   case EN_CPU_INTERRUPT_TIME_0://切换时序0,ADC的基准值
        {
            Adc_Calibration_Zero();
            break;
        }
        case EN_CPU_INTERRUPT_TIME_1://切换时序1 S0 = 0 S1 = 0
        {
            // gstAdcResult.Grid_volt_phase_a = gstAdcCoeff.Grid_volt_phase_a * (int16_t)(AdcaResultRegs.ADCRESULT10 - gstAdcZero.Grid_volt_phase_a_zero);
            // gstAdcResult.Grid_volt_phase_b = gstAdcCoeff.Grid_volt_phase_b * (int16_t)(AdcaResultRegs.ADCRESULT6  - gstAdcZero.Grid_volt_phase_b_zero);
            // gstAdcResult.Grid_volt_phase_c = gstAdcCoeff.Grid_volt_phase_c * (int16_t)(AdcbResultRegs.ADCRESULT3  - gstAdcZero.Grid_volt_phase_c_zero);
            // gstAdcResult.Grid_curr_phase_a = gstAdcCoeff.Grid_curr_phase_a * (int16_t)(AdccResultRegs.ADCRESULT10 - gstAdcZero.Grid_curr_phase_a_zero);
            // gstAdcResult.Grid_curr_phase_b = gstAdcCoeff.Grid_curr_phase_b * (int16_t)(AdccResultRegs.ADCRESULT6  - gstAdcZero.Grid_curr_phase_b_zero);
            gstAdcResult.inv_L_curr1    = gstAdcCoeff.inv_L_curr    * (int16_t)(AdcaResultRegs.ADCRESULT1  - gstAdcZero.inv_L_curr_zero);
            gstAdcResult.inv_curr_tol1    = gstAdcCoeff.inv_curr_tol    * (int16_t)(AdcdResultRegs.ADCRESULT1  - gstAdcZero.inv_curr_tol_zero); // T
            gstAdcResult.Grid_curr = gstAdcCoeff.Grid_curr * (int16_t)(AdcdResultRegs.ADCRESULT2  - gstAdcZero.Grid_curr_zero);
            gstAdcResult.pv1_volt          = gstAdcCoeff.pv1_volt          * (int16_t)(AdccResultRegs.ADCRESULT2  - 2048);//gstAdcZero.pv1_volt_zero);
            gstAdcResult.pv1_curr          = gstAdcCoeff.pv1_curr          * (int16_t)(AdcaResultRegs.ADCRESULT2  - 2048);
            gstAdcResult.bus_volt_all      = gstAdcCoeff.bus_volt_all      * (int16_t)(AdcdResultRegs.ADCRESULT8  - gstAdcZero.bus_volt_all_zero); //BUS正对N
            break;
        }
        case EN_CPU_INTERRUPT_TIME_2://切换时序2 S0 = 0 S1 = 1
        {
            // gstAdcResult.gen_volt_phase_a  = gstAdcCoeff.gen_volt_phase_a * (int16_t)(AdcaResultRegs.ADCRESULT11 - gstAdcZero.gen_volt_phase_a_zero);
            // gstAdcResult.gen_volt_phase_b  = gstAdcCoeff.gen_volt_phase_b * (int16_t)(AdcaResultRegs.ADCRESULT7  - gstAdcZero.gen_volt_phase_b_zero);
            // gstAdcResult.gen_volt  = gstAdcCoeff.gen_volt * (int16_t)(AdcbResultRegs.ADCRESULT4  - gstAdcZero.gen_volt_zero);
            // gstAdcResult.gen_curr_phase_a  = gstAdcCoeff.gen_curr_phase_a * (int16_t)(AdccResultRegs.ADCRESULT11 - gstAdcZero.gen_curr_phase_a_zero);
            // gstAdcResult.gen_curr_phase_b  = gstAdcCoeff.gen_curr_phase_b * (int16_t)(AdccResultRegs.ADCRESULT7  - gstAdcZero.gen_curr_phase_b_zero);
            gstAdcResult.inv_L_curr2    = gstAdcCoeff.inv_L_curr    * (int16_t)(AdcaResultRegs.ADCRESULT1  - gstAdcZero.inv_L_curr_zero);
            gstAdcResult.inv_curr_tol2    = gstAdcCoeff.inv_curr_tol    * (int16_t)(AdcdResultRegs.ADCRESULT1  - gstAdcZero.inv_curr_tol_zero); // T
            gstAdcResult.gen_curr  = gstAdcCoeff.gen_curr * (int16_t)(AdcdResultRegs.ADCRESULT3  - gstAdcZero.gen_curr_zero);
            gstAdcResult.pv2_volt          = gstAdcCoeff.pv2_volt         * (int16_t)(AdccResultRegs.ADCRESULT3  - 2048);//gstAdcZero.pv2_volt_zero);
            gstAdcResult.pv2_curr          = gstAdcCoeff.pv2_curr         * (int16_t)(AdcaResultRegs.ADCRESULT3  - 2048);
            gstAdcResult.bus_volt_pn      = gstAdcCoeff.bus_volt_pn     * (int16_t)(AdcdResultRegs.ADCRESULT9  - gstAdcZero.bus_volt_pn_zero);
            break;
        }
        case EN_CPU_INTERRUPT_TIME_3://切换时序3 S0 = 1 S1 = 0
        {
            gstAdcResult.inv_L_curr3    = gstAdcCoeff.inv_L_curr    * (int16_t)(AdcaResultRegs.ADCRESULT1  - gstAdcZero.inv_L_curr_zero);
            gstAdcResult.inv_curr_tol3    = gstAdcCoeff.inv_curr_tol    * (int16_t)(AdcdResultRegs.ADCRESULT1  - gstAdcZero.inv_curr_tol_zero); // T
            // gstAdcResult.load_volt_phase_a      = gstAdcCoeff.load_volt_phase_a     * (int16_t)(AdcaResultRegs.ADCRESULT12 - gstAdcZero.load_volt_phase_a_zero);
            // gstAdcResult.load_volt_phase_b      = gstAdcCoeff.load_volt_phase_b     * (int16_t)(AdcaResultRegs.ADCRESULT8  - gstAdcZero.load_volt_phase_b_zero);
            gstAdcResult.load_volt      = gstAdcCoeff.load_volt     * (int16_t)(AdcbResultRegs.ADCRESULT5  - gstAdcZero.load_volt_zero);
            // gstAdcResult.load_current_phase_a   = gstAdcCoeff.load_current_phase_a  * (int16_t)(AdccResultRegs.ADCRESULT12 - gstAdcZero.load_current_phase_a_zero);
            // gstAdcResult.load_current_phase_b   = gstAdcCoeff.load_current_phase_b  * (int16_t)(AdccResultRegs.ADCRESULT8  - gstAdcZero.load_current_phase_b_zero);
            gstAdcResult.load_current   = gstAdcCoeff.load_current  * (int16_t)(AdcdResultRegs.ADCRESULT4  - gstAdcZero.load_current_zero);
            gstAdcResult.pv3_volt               = gstAdcCoeff.pv3_volt              * (int16_t)(AdccResultRegs.ADCRESULT4  -2048);//- gstAdcZero.pv3_volt_zero);
            gstAdcResult.pv3_curr               = gstAdcCoeff.pv3_curr              * (int16_t)(AdcaResultRegs.ADCRESULT4  - 2048);
            gstAdcResult.bus_curr               = gstAdcCoeff.bus_curr              * (int16_t)(AdcdResultRegs.ADCRESULT10 - gstAdcZero.bus_curr_zero);
            break;
        }
        case EN_CPU_INTERRUPT_TIME_4://切换时序4 S0 = 1 S1 = 1
        {
            gstAdcResult.inv_L_curr4    = gstAdcCoeff.inv_L_curr    * (int16_t)(AdcaResultRegs.ADCRESULT1  - gstAdcZero.inv_L_curr_zero);
            gstAdcResult.inv_curr_tol4    = gstAdcCoeff.inv_curr_tol    * (int16_t)(AdcdResultRegs.ADCRESULT1  - gstAdcZero.inv_curr_tol_zero); // T
            // gstAdcResult.relay_middle_volt_phase_a  = gstAdcCoeff.relay_middle_volt_phase_a * (int16_t)(AdcaResultRegs.ADCRESULT13 - gstAdcZero.relay_middle_volt_phase_a_zero);
            // gstAdcResult.relay_middle_volt_phase_b  = gstAdcCoeff.relay_middle_volt_phase_b * (int16_t)(AdcaResultRegs.ADCRESULT9  - gstAdcZero.relay_middle_volt_phase_b_zero);
            // gstAdcResult.relay_middle_volt_phase_c  = gstAdcCoeff.relay_middle_volt_phase_c * (int16_t)(AdcbResultRegs.ADCRESULT6  - gstAdcZero.relay_middle_volt_phase_c_zero);
            gstAdcResult.pv4_volt                   = gstAdcCoeff.pv4_volt                  * (int16_t)(AdccResultRegs.ADCRESULT5  - 2048);//gstAdcZero.pv4_volt_zero);
            gstAdcResult.pv4_curr                   = gstAdcCoeff.pv4_curr                  * (int16_t)(AdcaResultRegs.ADCRESULT5  - 2048);
            gstAdcResult.grid_vol                   = gstAdcCoeff.grid_vol                  * (int16_t)(AdcaResultRegs.ADCRESULT11  - gstAdcZero.grid_vol_zero);
            break;
        }
        default:
        {
            break;

        }

    }
    /*      温度采样部分      */
    if(GpioDataRegs.GPADAT.bit.GPIO25 == 0)
    {
        //TAG:ADC的温度采样框架已搭好,改下变量名便可以直接用
        switch(value)
        {
            case EN_CPU_INTERRUPT_TIME_1://此时温度切换IO状态为000
            {
                gstAdcResult.Temp_A0 = gstAdcCoeff.Temp_A0 * (int16_t)(AdcdResultRegs.ADCRESULT12 - gstAdcZero.common_calibration_zero);
            }
            break;
            case EN_CPU_INTERRUPT_TIME_2://此时温度切换IO状态为001
            {
                gstAdcResult.Temp_A1 = gstAdcCoeff.Temp_A1 * (int16_t)(AdcdResultRegs.ADCRESULT12 - gstAdcZero.common_calibration_zero);
            }
            break;
            case EN_CPU_INTERRUPT_TIME_3://此时温度切换IO状态为010
            {
                gstAdcResult.Temp_A2 = gstAdcCoeff.Temp_A2 * (int16_t)(AdcdResultRegs.ADCRESULT12 - gstAdcZero.common_calibration_zero);
            }
            break;
            case EN_CPU_INTERRUPT_TIME_4://此时温度切换IO状态为011
            {
                gstAdcResult.Temp_A3 = gstAdcCoeff.Temp_A3 * (int16_t)(AdcdResultRegs.ADCRESULT12 - gstAdcZero.common_calibration_zero);
            }
            break;
            default:

            break;
        }
    }
    else
    {
        switch(value)
        {
            case EN_CPU_INTERRUPT_TIME_1://此时温度切换IO状态为100
            {
                gstAdcResult.Temp_A4 = gstAdcCoeff.Temp_A4 * (int16_t)(AdcdResultRegs.ADCRESULT12 - gstAdcZero.common_calibration_zero);
            }
            break;
            case EN_CPU_INTERRUPT_TIME_2://此时温度切换IO状态为101
            {
                gstAdcResult.Temp_A5 = gstAdcCoeff.Temp_A5 * (int16_t)(AdcdResultRegs.ADCRESULT12 - gstAdcZero.common_calibration_zero);
            }
            break;
            case EN_CPU_INTERRUPT_TIME_3://此时温度切换IO状态为110
            {
                gstAdcResult.Temp_A6 = gstAdcCoeff.Temp_A6 * (int16_t)(AdcdResultRegs.ADCRESULT12 - gstAdcZero.common_calibration_zero);
            }
            break;
            case EN_CPU_INTERRUPT_TIME_4://此时温度切换IO状态为111
            {
                gstAdcResult.Temp_A7 = gstAdcCoeff.Temp_A7 * (int16_t)(AdcdResultRegs.ADCRESULT12 - gstAdcZero.common_calibration_zero);
            }
            break;
            default:

            break;
        }
    }
}
/**
 * @brief AdcCoeffInit
 * @param
 */
void AdcCoeffInit(void)
{
    gstAdcCoeff.inv_volt            = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    // gstAdcCoeff.inv_volt_phase_b            = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    // gstAdcCoeff.inv_volt_phase_c            = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    gstAdcCoeff.inv_L_curr            = (3.0f / 4096.0) / (0.00625f);
    // gstAdcCoeff.inv_curr_phase_b            = (3.0f / 4096.0) / (0.00625f);
    gstAdcCoeff.inv_curr_tol            = (3.0f / 4096.0) / (0.00625f);
    gstAdcCoeff.ct_curr                   = (3.0f / 4096.0) * (1000.0f);
    // gstAdcCoeff.ct_b_curr                   = (3.0f / 4096.0) * (1000.0f);
    // gstAdcCoeff.ct_c_curr                   = (3.0f / 4096.0) * (1000.0f);
    gstAdcCoeff.leak_curr                   = (3.0f / 4096.0) / (4.0f);

    // gstAdcCoeff.Grid_volt_phase_a           = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    // gstAdcCoeff.Grid_volt_phase_b           = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    // gstAdcCoeff.Grid_volt_phase_c           = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    // gstAdcCoeff.Grid_curr_phase_a           = (3.0f / 4096.0) / (0.00625f);
    // gstAdcCoeff.Grid_curr_phase_b           = (3.0f / 4096.0) / (0.00625f);
    gstAdcCoeff.Grid_curr           = (3.0f / 4096.0) / (0.00625f);

    // gstAdcCoeff.gen_volt_phase_a            = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    // gstAdcCoeff.gen_volt_phase_b            = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    gstAdcCoeff.gen_volt            = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    // gstAdcCoeff.gen_curr_phase_a            = (3.0f / 4096.0) / (0.00625f);
    // gstAdcCoeff.gen_curr_phase_b            = (3.0f / 4096.0) / (0.00625f);
    gstAdcCoeff.gen_curr            = (3.0f / 4096.0) / (0.00625f);


    // gstAdcCoeff.load_volt_phase_a           = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    // gstAdcCoeff.load_volt_phase_b           = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    gstAdcCoeff.load_volt           = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    // gstAdcCoeff.load_current_phase_a        = (3.0f / 4096.0) / (0.00625f);
    // gstAdcCoeff.load_current_phase_b        = (3.0f / 4096.0) / (0.00625f);
    gstAdcCoeff.load_current        = (3.0f / 4096.0) / (0.00625f);

    gstAdcCoeff.bus_curr                    = (3.0f / 4096.0) / (0.0125f);
    gstAdcCoeff.pv1_volt                    = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    gstAdcCoeff.pv2_volt                    = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    gstAdcCoeff.pv3_volt                    = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    gstAdcCoeff.pv4_volt                    = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);

    gstAdcCoeff.pv1_curr                    = (3.0f / 4096.0) / (0.05f);
    gstAdcCoeff.pv2_curr                    = (3.0f / 4096.0) / (0.05f);
    gstAdcCoeff.pv3_curr                    = (3.0f / 4096.0) / (0.05f);
    gstAdcCoeff.pv4_curr                    = (3.0f / 4096.0) / (0.05f);

    gstAdcCoeff.bus_volt_all                = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    gstAdcCoeff.bus_volt_pn                = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    // gstAdcCoeff.relay_middle_volt_phase_a   = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    // gstAdcCoeff.relay_middle_volt_phase_b   = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    // gstAdcCoeff.relay_middle_volt_phase_c   = (3.0f / 4096.0) * ((470.0f * 6.0f + 62.0f) / 6.8f);
    gstAdcCoeff.Temp_A0                     = 1.0f;
    gstAdcCoeff.Temp_A1                     = 1.0f;
    gstAdcCoeff.Temp_A2                     = 1.0f;
    gstAdcCoeff.Temp_A3                     = 1.0f;
    gstAdcCoeff.Temp_A4                     = 1.0f;
    gstAdcCoeff.Temp_A5                     = 1.0f;
    gstAdcCoeff.Temp_A6                     = 1.0f;
    gstAdcCoeff.Temp_A7                     = 1.0f;
}
/**
 * @brief AdcConfig
 * @param
 */
void AdcConfig(void)
{
    EALLOW;
    //write configurations
    AdcaRegs.ADCCTL2.bit.PRESCALE = 14;
    AdcbRegs.ADCCTL2.bit.PRESCALE = 14;
    AdccRegs.ADCCTL2.bit.PRESCALE = 14;
    AdcdRegs.ADCCTL2.bit.PRESCALE = 14;
    AdcaRegs.ADCCTL2.bit.RESOLUTION = ADC_RESOLUTION_12BIT;
    AdcaRegs.ADCCTL2.bit.SIGNALMODE = ADC_SIGNALMODE_SINGLE;
    AdcbRegs.ADCCTL2.bit.RESOLUTION = ADC_RESOLUTION_12BIT;
    AdcbRegs.ADCCTL2.bit.SIGNALMODE = ADC_SIGNALMODE_SINGLE;
    AdccRegs.ADCCTL2.bit.RESOLUTION = ADC_RESOLUTION_12BIT;
    AdccRegs.ADCCTL2.bit.SIGNALMODE = ADC_SIGNALMODE_SINGLE;
    AdcdRegs.ADCCTL2.bit.RESOLUTION = ADC_RESOLUTION_12BIT;
    AdcdRegs.ADCCTL2.bit.SIGNALMODE = ADC_SIGNALMODE_SINGLE;
    AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;
    AdcbRegs.ADCCTL1.bit.INTPULSEPOS = 1;
    AdccRegs.ADCCTL1.bit.INTPULSEPOS = 1;
    AdcdRegs.ADCCTL1.bit.INTPULSEPOS = 1;
    AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;
    AdcbRegs.ADCCTL1.bit.ADCPWDNZ = 1;
    AdccRegs.ADCCTL1.bit.ADCPWDNZ = 1;
    AdcdRegs.ADCCTL1.bit.ADCPWDNZ = 1;
    AdcaRegs.ADCSOCPRICTL.bit.SOCPRIORITY = 1; //SOC0具有最高优先级

    DELAY_US(2000);

    /*     Adc A0    */
    AdcaRegs.ADCSOC0CTL.bit.CHSEL = 0;                  //逆变A相电压
    AdcaRegs.ADCSOC0CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
    /*     Adc A1    */
    AdcaRegs.ADCSOC1CTL.bit.CHSEL = 1;                  //逆变A相电流
    AdcaRegs.ADCSOC1CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcaRegs.ADCSOC1CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
    /*     Adc A2    */
    AdcaRegs.ADCSOC2CTL.bit.CHSEL = 2;                  //PV1电流
    AdcaRegs.ADCSOC2CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcaRegs.ADCSOC2CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

    AdcaRegs.ADCSOC3CTL.bit.CHSEL = 2;                  //PV2电流
    AdcaRegs.ADCSOC3CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcaRegs.ADCSOC3CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

    AdcaRegs.ADCSOC4CTL.bit.CHSEL = 2;                  //PV3电流
    AdcaRegs.ADCSOC4CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcaRegs.ADCSOC4CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

    AdcaRegs.ADCSOC5CTL.bit.CHSEL = 2;                  //PV4电流
    AdcaRegs.ADCSOC5CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcaRegs.ADCSOC5CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
    /*     Adc A3    */
    AdcaRegs.ADCSOC6CTL.bit.CHSEL = 3;                  //市电B相电压
    AdcaRegs.ADCSOC6CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcaRegs.ADCSOC6CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

    AdcaRegs.ADCSOC7CTL.bit.CHSEL = 3;                  //发电机B相电压
    AdcaRegs.ADCSOC7CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcaRegs.ADCSOC7CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

    AdcaRegs.ADCSOC8CTL.bit.CHSEL = 3;                  //负载B相电压
    AdcaRegs.ADCSOC8CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcaRegs.ADCSOC8CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

    AdcaRegs.ADCSOC9CTL.bit.CHSEL = 3;                  //总继电器与单继电器中间的B相电压(ADC_RelayMid_VOL_B)
    AdcaRegs.ADCSOC9CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcaRegs.ADCSOC9CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
    /*     Adc A4    */
    AdcaRegs.ADCSOC10CTL.bit.CHSEL = 4;                  //市电A相电压
    AdcaRegs.ADCSOC10CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcaRegs.ADCSOC10CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

    AdcaRegs.ADCSOC11CTL.bit.CHSEL = 4;                  //发电机A相电压
    AdcaRegs.ADCSOC11CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcaRegs.ADCSOC11CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

    AdcaRegs.ADCSOC12CTL.bit.CHSEL = 4;                  //负载A相电压
    AdcaRegs.ADCSOC12CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcaRegs.ADCSOC12CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

    AdcaRegs.ADCSOC13CTL.bit.CHSEL = 4;                  //总继电器与单继电器中间的A相电压(ADC_RelayMid_VOL_A)
    AdcaRegs.ADCSOC13CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcaRegs.ADCSOC13CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
    /*     Adc A5    */
    AdcaRegs.ADCSOC14CTL.bit.CHSEL = 5;                  //A相CT电流
    AdcaRegs.ADCSOC14CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcaRegs.ADCSOC14CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
    /*     Adc B0    */
    AdcbRegs.ADCSOC0CTL.bit.CHSEL = 0;                  //逆变B相电压
    AdcbRegs.ADCSOC0CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcbRegs.ADCSOC0CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
    /*     Adc B1    */
    AdcbRegs.ADCSOC1CTL.bit.CHSEL = 1;                  //逆变B相电流
    AdcbRegs.ADCSOC1CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcbRegs.ADCSOC1CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
    /*     Adc B2    */
    AdcbRegs.ADCSOC2CTL.bit.CHSEL = 2;                  //B相CT电流
    AdcbRegs.ADCSOC2CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcbRegs.ADCSOC2CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
    /*     Adc B3    */
    AdcbRegs.ADCSOC3CTL.bit.CHSEL = 3;                  //市电C相电压
    AdcbRegs.ADCSOC3CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcbRegs.ADCSOC3CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

    AdcbRegs.ADCSOC4CTL.bit.CHSEL = 3;                  //发电机C相电压
    AdcbRegs.ADCSOC4CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcbRegs.ADCSOC4CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

    AdcbRegs.ADCSOC5CTL.bit.CHSEL = 3;                  //负载C相电压
    AdcbRegs.ADCSOC5CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcbRegs.ADCSOC5CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

    AdcbRegs.ADCSOC6CTL.bit.CHSEL = 3;                  //总继电器与单继电器中间的A相电压(ADC_RelayMid_VOL_C)
    AdcbRegs.ADCSOC6CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcbRegs.ADCSOC6CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
    /*     Adc C2    */
    AdccRegs.ADCSOC2CTL.bit.CHSEL = 2;                  //PV1电压
    AdccRegs.ADCSOC2CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdccRegs.ADCSOC2CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

    AdccRegs.ADCSOC3CTL.bit.CHSEL = 2;                  //PV2电压
    AdccRegs.ADCSOC3CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdccRegs.ADCSOC3CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

    AdccRegs.ADCSOC4CTL.bit.CHSEL = 2;                  //PV3电压
    AdccRegs.ADCSOC4CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdccRegs.ADCSOC4CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

    AdccRegs.ADCSOC5CTL.bit.CHSEL = 2;                  //PV4电压
    AdccRegs.ADCSOC5CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdccRegs.ADCSOC5CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
    /*     Adc C3    */
    AdccRegs.ADCSOC6CTL.bit.CHSEL = 3;                  //市电B相电流
    AdccRegs.ADCSOC6CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdccRegs.ADCSOC6CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

    AdccRegs.ADCSOC7CTL.bit.CHSEL = 3;                  //发电机B相电流
    AdccRegs.ADCSOC7CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdccRegs.ADCSOC7CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

    AdccRegs.ADCSOC8CTL.bit.CHSEL = 3;                  //负载B相电流
    AdccRegs.ADCSOC8CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdccRegs.ADCSOC8CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

    AdccRegs.ADCSOC9CTL.bit.CHSEL = 3;                  //预留
    AdccRegs.ADCSOC9CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdccRegs.ADCSOC9CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
    /*     Adc C4    */
    AdccRegs.ADCSOC10CTL.bit.CHSEL = 4;                  //市电A相电流
    AdccRegs.ADCSOC10CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdccRegs.ADCSOC10CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

    AdccRegs.ADCSOC11CTL.bit.CHSEL = 4;                  //发电机A相电流
    AdccRegs.ADCSOC11CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdccRegs.ADCSOC11CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

    AdccRegs.ADCSOC12CTL.bit.CHSEL = 4;                  //负载A相电流
    AdccRegs.ADCSOC12CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdccRegs.ADCSOC12CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

    AdccRegs.ADCSOC13CTL.bit.CHSEL = 4;                  //预留
    AdccRegs.ADCSOC13CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdccRegs.ADCSOC13CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
    /*     Adc D0    */
    AdcdRegs.ADCSOC0CTL.bit.CHSEL = 0;                  //逆变C相电压
    AdcdRegs.ADCSOC0CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcdRegs.ADCSOC0CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
    /*     Adc D1    */
    AdcdRegs.ADCSOC1CTL.bit.CHSEL = 1;                  //逆变C相电流
    AdcdRegs.ADCSOC1CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcdRegs.ADCSOC1CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
    /*     Adc D2    */
    AdcdRegs.ADCSOC2CTL.bit.CHSEL = 2;                  //市电C相电流
    AdcdRegs.ADCSOC2CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcdRegs.ADCSOC2CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

    AdcdRegs.ADCSOC3CTL.bit.CHSEL = 2;                  //发电机C相电流
    AdcdRegs.ADCSOC3CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcdRegs.ADCSOC3CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

    AdcdRegs.ADCSOC4CTL.bit.CHSEL = 2;                  //负载C相电流
    AdcdRegs.ADCSOC4CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcdRegs.ADCSOC4CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

    AdcdRegs.ADCSOC5CTL.bit.CHSEL = 2;                  //预留
    AdcdRegs.ADCSOC5CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcdRegs.ADCSOC5CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
    /*     Adc D3    */
    AdcdRegs.ADCSOC6CTL.bit.CHSEL = 3;                  //C相CT电流
    AdcdRegs.ADCSOC6CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcdRegs.ADCSOC6CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
    /*     Adc D4    */
    AdcdRegs.ADCSOC7CTL.bit.CHSEL = 4;                  //RCMU电压(漏电流电压)
    AdcdRegs.ADCSOC7CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcdRegs.ADCSOC7CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
    /*     Adc D14    */
    AdcdRegs.ADCSOC8CTL.bit.CHSEL = 14;                 //BUS正电压
    AdcdRegs.ADCSOC8CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcdRegs.ADCSOC8CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

    AdcdRegs.ADCSOC9CTL.bit.CHSEL = 14;                 //BUS负电压
    AdcdRegs.ADCSOC9CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcdRegs.ADCSOC9CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

    AdcdRegs.ADCSOC10CTL.bit.CHSEL = 14;                 //BUS的平均电流
    AdcdRegs.ADCSOC10CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcdRegs.ADCSOC10CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

    AdcdRegs.ADCSOC11CTL.bit.CHSEL = 14;                 //市电N对PE电压
    AdcdRegs.ADCSOC11CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcdRegs.ADCSOC11CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
    /*     Adc D15    */
    AdcdRegs.ADCSOC12CTL.bit.CHSEL = 15;                 //温度获取
    AdcdRegs.ADCSOC12CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
    AdcdRegs.ADCSOC12CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
    EDIS;
}
/**
 * @brief Get_Adc_Calibration_State
 * @param  ADC的零点校准状态
 */
uint16_t Get_Adc_Calibration_State(void)
{
    return gstAdcZero.calibration_finish;
}
/**
 * @brief Adc_Init
 * @param
 */
void Adc_Init(void)
{
    AdcCoeffInit();
    AdcConfig();
}
