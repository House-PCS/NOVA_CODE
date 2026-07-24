/*
 * ph3_calc_interface.c
 *
 *
 */
#include "ph3_calc_interface.h"

#pragma CODE_SECTION(Ph3Calc_InterfacePosNegUpdate, ".TI.ramfunc");
#pragma CODE_SECTION(Ph3Calc_InterfaceAngleUpdate, ".TI.ramfunc");

static void Ph3Calc_InterfacePosNegUpdate(Ph3Calc_IfcPosNeg_t* ifc_posneg,
                                          const Ph3Calc_PosNegData_t* posneg)
{
    if((ifc_posneg == NULL) || (posneg == NULL))
    {
        return;
    }

    ifc_posneg->alfa_pos = posneg->alfa_pos;
    ifc_posneg->beta_pos = posneg->beta_pos;
    ifc_posneg->alfa_neg = posneg->alfa_neg;
    ifc_posneg->beta_neg = posneg->beta_neg;
    ifc_posneg->d_pos = posneg->d_pos;
    ifc_posneg->q_pos = posneg->q_pos;
    ifc_posneg->d_neg = posneg->d_neg;
    ifc_posneg->q_neg = posneg->q_neg;
}

void Ph3Calc_InterfaceInit(Ph3Calc_Interface_t* ifc)
{
    if(ifc == NULL)
    {
        return;
    }

    ifc->vol.a_phase_instant = 0.0f;
    ifc->vol.b_phase_instant = 0.0f;
    ifc->vol.c_phase_instant = 0.0f;
    ifc->vol.a_phase_rms = 0.0f;
    ifc->vol.b_phase_rms = 0.0f;
    ifc->vol.c_phase_rms = 0.0f;
    ifc->vol.freq_a = 0.0f;
    ifc->vol.freq_b = 0.0f;
    ifc->vol.freq_c = 0.0f;
    ifc->vol.theta = 0.0f;

    ifc->curr.a_phase_instant = 0.0f;
    ifc->curr.b_phase_instant = 0.0f;
    ifc->curr.c_phase_instant = 0.0f;
    ifc->curr.a_phase_rms = 0.0f;
    ifc->curr.b_phase_rms = 0.0f;
    ifc->curr.c_phase_rms = 0.0f;
    ifc->curr.freq_a = 0.0f;
    ifc->curr.freq_b = 0.0f;
    ifc->curr.freq_c = 0.0f;
    ifc->curr.theta = 0.0f;

    ifc->power.a_phase_active = 0.0f;
    ifc->power.a_phase_reactive = 0.0f;
    ifc->power.a_phase_apperant = 0.0f;
    ifc->power.b_phase_active = 0.0f;
    ifc->power.b_phase_reactive = 0.0f;
    ifc->power.b_phase_apperant = 0.0f;
    ifc->power.c_phase_active = 0.0f;
    ifc->power.c_phase_reactive = 0.0f;
    ifc->power.c_phase_apperant = 0.0f;
    ifc->power.total_active = 0.0f;
    ifc->power.total_reactive = 0.0f;
    ifc->power.total_apperant = 0.0f;
    ifc->power.power_factor = 0.0f;

    ifc->vol_a_phase_count = 0.0f;
    ifc->vol_b_phase_count = 0.0f;
    ifc->vol_c_phase_count = 0.0f;

    ifc->volt_posneg.alfa_pos = 0.0f;
    ifc->volt_posneg.beta_pos = 0.0f;
    ifc->volt_posneg.alfa_neg = 0.0f;
    ifc->volt_posneg.beta_neg = 0.0f;
    ifc->volt_posneg.d_pos = 0.0f;
    ifc->volt_posneg.q_pos = 0.0f;
    ifc->volt_posneg.d_neg = 0.0f;
    ifc->volt_posneg.q_neg = 0.0f;
    ifc->curr_posneg = ifc->volt_posneg;
}

static inline float normalize_angle(float theta)
{
    while (theta >= PH3_CALC_M_2PI)
    {
        theta -= PH3_CALC_M_2PI;
    }
    while (theta < 0.0f)
    {
        theta += PH3_CALC_M_2PI;
    }
    return theta;
}

void Ph3Calc_InterfaceAngleUpdate(Ph3Calc_Interface_t* ifc, const Ph3Calc_Channel_t* ch)
{
    if((ifc == NULL) || (ch == NULL))
    {
        return;
    }

//    ifc->vol.theta = ch->volt_pll.theta_cos;
//    ifc->curr.theta = ch->curr_pll.theta_cos;
    ifc->vol.theta = normalize_angle(ch->volt_pll.theta_sin);
    ifc->curr.theta = normalize_angle(ch->curr_pll.theta_sin);

}

void Ph3Calc_InterfaceUpdate(Ph3Calc_Interface_t* ifc, const Ph3Calc_Channel_t* ch)
{
    if((ifc == NULL) || (ch == NULL))
    {
        return;
    }

    ifc->vol.a_phase_instant = ch->phase_a.vol.inst;
    ifc->vol.b_phase_instant = ch->phase_b.vol.inst;
    ifc->vol.c_phase_instant = ch->phase_c.vol.inst;
    ifc->vol.a_phase_rms = ch->phase_a.vol.rms;
    ifc->vol.b_phase_rms = ch->phase_b.vol.rms;
    ifc->vol.c_phase_rms = ch->phase_c.vol.rms;
    ifc->vol.freq_a = ch->freq_a;
    ifc->vol.freq_b = ch->freq_b;
    ifc->vol.freq_c = ch->freq_c;

    ifc->curr.a_phase_instant = ch->phase_a.cur.inst;
    ifc->curr.b_phase_instant = ch->phase_b.cur.inst;
    ifc->curr.c_phase_instant = ch->phase_c.cur.inst;
    ifc->curr.a_phase_rms = ch->phase_a.cur.rms;
    ifc->curr.b_phase_rms = ch->phase_b.cur.rms;
    ifc->curr.c_phase_rms = ch->phase_c.cur.rms;
    ifc->curr.freq_a = ch->freq_a;
    ifc->curr.freq_b = ch->freq_b;
    ifc->curr.freq_c = ch->freq_c;

    ifc->power.a_phase_active = ch->phase_a.pwr.pwr_act;
    ifc->power.a_phase_reactive = ch->phase_a.pwr.pwr_rea;
    ifc->power.a_phase_apperant = ch->phase_a.pwr.pwr_app;
    ifc->power.b_phase_active = ch->phase_b.pwr.pwr_act;
    ifc->power.b_phase_reactive = ch->phase_b.pwr.pwr_rea;
    ifc->power.b_phase_apperant = ch->phase_b.pwr.pwr_app;
    ifc->power.c_phase_active = ch->phase_c.pwr.pwr_act;
    ifc->power.c_phase_reactive = ch->phase_c.pwr.pwr_rea;
    ifc->power.c_phase_apperant = ch->phase_c.pwr.pwr_app;
    ifc->power.total_active = ch->total_pwr_act;
    ifc->power.total_reactive = ch->total_pwr_rea;
    ifc->power.total_apperant = ch->total_pwr_app;
    ifc->power.power_factor = ch->power_factor;

    ifc->vol_a_phase_count = ch->rms_a.count;
    ifc->vol_b_phase_count = ch->rms_b.count;
    ifc->vol_c_phase_count = ch->rms_c.count;

    Ph3Calc_InterfacePosNegUpdate(&ifc->volt_posneg, &ch->volt_posneg);
    Ph3Calc_InterfacePosNegUpdate(&ifc->curr_posneg, &ch->curr_posneg);
}
