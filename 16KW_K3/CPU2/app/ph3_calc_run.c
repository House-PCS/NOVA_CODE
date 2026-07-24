/*
 * ph3_calc_run.c
 *
 * CPU2 three-phase calculation runtime.
 */
#include "ph3_calc_run.h"
#include "dev_dual_core_receive.h"
#include "dual_data_share.h"
#include "ph3_calc_ifc_update.h"
#include "ph3_calc/ph3_calc_filter.h"
#include "ph3_calc/ph3_calc_channel.h"
#include "mid_vol_and_rcmu_rms.h"

#pragma CODE_SECTION(Ph3CalcRun_Run, ".TI.ramfunc");

Ph3Calc_Channel_t s_grid;
Ph3Calc_Channel_t s_gen;
Ph3Calc_Channel_t s_load;
Ph3Calc_Channel_t s_inv;
Ph3Calc_Channel_t s_ct;

static Ph3Calc_Filter_t s_busCurrFilter;
static volatile bool s_dataUpdateFlag = false;

void Ph3CalcRun_Init(void)
{
    Ph3Calc_ChannelInit(&s_grid, PH3_CALC_PF_MODE_NORMAL, PH3_CALC_PLL_MODE_POSSEQ);
    Ph3Calc_ChannelInit(&s_gen, PH3_CALC_PF_MODE_NORMAL, PH3_CALC_PLL_MODE_POSSEQ);
    Ph3Calc_ChannelInit(&s_load, PH3_CALC_PF_MODE_NORMAL, PH3_CALC_PLL_MODE_POSSEQ);
    Ph3Calc_ChannelInit(&s_inv, PH3_CALC_PF_MODE_INV, PH3_CALC_PLL_MODE_POSSEQ);
    Ph3Calc_ChannelInit(&s_ct, PH3_CALC_PF_MODE_NORMAL, PH3_CALC_PLL_MODE_POSSEQ);

    Ph3Calc_Filter_Init(&s_busCurrFilter, PH3_CALC_FILTER_DEFAULT_WIN);
}

void Ph3CalcRun_Run(void)
{
    volatile CpuAdc_t *adc = DevDcr_GetCpuAdcResult();

    if(adc == (volatile CpuAdc_t *)0)
    {
        return;
    }

    Ph3Calc_ChannelSetSample(&s_grid,
                             adc->grid_volt_phase_a, adc->grid_volt_phase_b, adc->grid_volt_phase_c,
                             adc->grid_curr_phase_a, adc->grid_curr_phase_b, adc->grid_curr_phase_c);
    Ph3Calc_ChannelRun(&s_grid);
    Ph3Calc_InterfaceAngleUpdate(&gstPh3CalcIfcUpdate.grid, &s_grid);
    Ph3Calc_ChannelSetSample(&s_gen,
                             adc->gen_volt_phase_a, adc->gen_volt_phase_b, adc->gen_volt_phase_c,
                             adc->gen_curr_phase_a, adc->gen_curr_phase_b, adc->gen_curr_phase_c);
    Ph3Calc_ChannelRun(&s_gen);
    Ph3Calc_InterfaceAngleUpdate(&gstPh3CalcIfcUpdate.gen, &s_gen);
    Ph3Calc_ChannelSetSample(&s_load,
                             adc->load_volt_phase_a, adc->load_volt_phase_b, adc->load_volt_phase_c,
                             adc->load_curr_phase_a, adc->load_curr_phase_b, adc->load_curr_phase_c);
    Ph3Calc_ChannelRun(&s_load);
    Ph3Calc_InterfaceAngleUpdate(&gstPh3CalcIfcUpdate.load, &s_load);
    Ph3Calc_ChannelSetSample(&s_inv,
                             adc->inv_volt_phase_a, adc->inv_volt_phase_b, adc->inv_volt_phase_c,
                             adc->inv_curr_phase_a, adc->inv_curr_phase_b, adc->inv_curr_phase_c);
    Ph3Calc_ChannelRun(&s_inv);
    Ph3Calc_InterfaceAngleUpdate(&gstPh3CalcIfcUpdate.inv, &s_inv);
    Ph3Calc_ChannelSetSample(&s_ct,
                             adc->grid_volt_phase_a, adc->grid_volt_phase_b, adc->grid_volt_phase_c,
                             adc->ct_a_curr, adc->ct_b_curr, adc->ct_c_curr);
    Ph3Calc_ChannelRun(&s_ct);
    Ph3Calc_InterfaceAngleUpdate(&gstPh3CalcIfcUpdate.ct, &s_ct);
    gstPh3CalcIfcUpdate.bus.curr_filter = Ph3Calc_Filter_Run(&s_busCurrFilter, adc->bus_curr);
    gstPh3CalcIfcUpdate.bus.pos_vol_filter = adc->bus_volt_pos;
    gstPh3CalcIfcUpdate.bus.neg_vol_filter = adc->bus_volt_neg;
    gstPh3CalcIfcUpdate.bus.vol_filter  = gstPh3CalcIfcUpdate.bus.pos_vol_filter + gstPh3CalcIfcUpdate.bus.neg_vol_filter;
    gstPh3CalcIfcUpdate.bus.power_filter = gstPh3CalcIfcUpdate.bus.vol_filter * gstPh3CalcIfcUpdate.bus.curr_filter;
    DualDataShare_Ph3IfcRunDone();
    MidVolAndRcmuRms_Run(adc->relay_middle_volt_phase_a, adc->relay_middle_volt_phase_b, adc->relay_middle_volt_phase_c,
                         adc->leak_curr, gstPh3CalcIfcUpdate.inv.vol.freq_a);
    s_dataUpdateFlag = true;
}

void Ph3CalcRun_FlagUpdate(void)
{
    volatile DevDct_Cpu1ToCpu2Msg_t *msg = DevDcr_GetCpu1ToCpu2Msg();

    if(msg == (volatile DevDct_Cpu1ToCpu2Msg_t *)0)
    {
        return;
    }

    DevDcr_ApplyPh3RunFlag(&s_grid, &msg->grid);
    DevDcr_ApplyPh3RunFlag(&s_gen, &msg->gen);
    DevDcr_ApplyPh3RunFlag(&s_load, &msg->load);
    DevDcr_ApplyPh3RunFlag(&s_inv, &msg->inv);
    DevDcr_ApplyPh3RunFlag(&s_ct, &msg->ct);
}

void Ph3CalcRun_DataUpdate(void)
{
    if(s_dataUpdateFlag)
    {
        s_dataUpdateFlag = false;
        Ph3CalcIfcUpdate(&gstPh3CalcIfcUpdate.grid, &s_grid);
        Ph3CalcIfcUpdate(&gstPh3CalcIfcUpdate.gen, &s_gen);
        Ph3CalcIfcUpdate(&gstPh3CalcIfcUpdate.load, &s_load);
        Ph3CalcIfcUpdate(&gstPh3CalcIfcUpdate.inv, &s_inv);
        Ph3CalcIfcUpdate(&gstPh3CalcIfcUpdate.ct, &s_ct);
        MidVolAndRcmuRms_Update(&gstPh3CalcIfcUpdate);
    }
}
