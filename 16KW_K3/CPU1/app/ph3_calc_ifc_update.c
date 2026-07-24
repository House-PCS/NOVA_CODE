/*
 * ph3_calc_ifc_update.c
 *
 * CPU1 ph3 calc data info (mapped from CPU2 GS4 shared RAM).
 */
#include "ph3_calc_ifc_update.h"
#include "dev_dual_core_receive.h"
#include <string.h>
#include "debug_analysis.h"
Ph3CalcInfo_t gstPh3CalcInfo;

static void Ph3CalcInfo_CopyVol(Ph3CalcInfoVol_t *dst, const volatile Ph3Calc_IfcVol_t *src)
{
    dst->a_phase_rms = src->a_phase_rms;
    dst->b_phase_rms = src->b_phase_rms;
    dst->c_phase_rms = src->c_phase_rms;
    dst->freq = src->freq_a;
    dst->theta = src->theta;
}

static void Ph3CalcInfo_CopyCurr(Ph3CalcInfoCurr_t *dst, const volatile Ph3Calc_IfcCurr_t *src)
{
    dst->a_phase_rms = src->a_phase_rms;
    dst->b_phase_rms = src->b_phase_rms;
    dst->c_phase_rms = src->c_phase_rms;
    dst->freq = src->freq_a;
    dst->theta = src->theta;
}

static void Ph3CalcInfo_CopyPower(Ph3CalcInfoPower_t *dst, const volatile Ph3Calc_IfcPower_t *src)
{
    dst->a_phase_active = src->a_phase_active;
    dst->a_phase_reactive = src->a_phase_reactive;
    dst->a_phase_apperant = src->a_phase_apperant;
    dst->b_phase_active = src->b_phase_active;
    dst->b_phase_reactive = src->b_phase_reactive;
    dst->b_phase_apperant = src->b_phase_apperant;
    dst->c_phase_active = src->c_phase_active;
    dst->c_phase_reactive = src->c_phase_reactive;
    dst->c_phase_apperant = src->c_phase_apperant;
    dst->total_active = src->total_active;
    dst->total_reactive = src->total_reactive;
    dst->total_apperant = src->total_apperant;
    dst->power_factor = src->power_factor;
}

static void Ph3CalcInfo_CopyBus(Ph3CalcInfoBus_t *dst, const volatile Ph3CalcIfcBus_t *src)
{
    dst->vol_filter = src->vol_filter;
    dst->curr_filter = src->curr_filter;
    dst->power_filter = src->power_filter;
    dst->pos_vol_filter = src->pos_vol_filter;
    dst->neg_vol_filter = src->neg_vol_filter;
}

static void Ph3CalcInfo_CopyChannel(const volatile Ph3Calc_Interface_t *src,
                                    Ph3CalcInfoVol_t *vol,
                                    Ph3CalcInfoCurr_t *curr,
                                    Ph3CalcInfoPower_t *power)
{
    Ph3CalcInfo_CopyVol(vol, &src->vol);
    Ph3CalcInfo_CopyCurr(curr, &src->curr);
    Ph3CalcInfo_CopyPower(power, &src->power);
}

void Ph3CalcInfo_Init(void)
{
    (void)memset(&gstPh3CalcInfo, 0, sizeof(gstPh3CalcInfo));
    DevDcr_ResetPh3Ifc();
}

bool Ph3CalcInfo_Update(void)
{
    volatile DualDataShare_Ph3Ifc_t *src = DevDcr_GetPh3Ifc();

    if(src == (volatile DualDataShare_Ph3Ifc_t *)0)
    {
        return false;
    }
    gstPh3CalcInfo.grid_vol.theta = src->grid.vol.theta;
    gstPh3CalcInfo.inv_vol.theta = src->inv.vol.theta;
    gstPh3CalcInfo.load_vol.theta = src->load.vol.theta;
    gstPh3CalcInfo.gen_vol.theta = src->gen.vol.theta;
    Ph3CalcInfo_CopyBus(&gstPh3CalcInfo.bus, &src->bus);
    if(gstPh3CalcInfo.ferq_devision == 0)
    {
        Ph3CalcInfo_CopyVol(&gstPh3CalcInfo.grid_vol, &src->grid.vol);
    }
    else if(gstPh3CalcInfo.ferq_devision == 1)
    {
        Ph3CalcInfo_CopyCurr(&gstPh3CalcInfo.grid_curr, &src->grid.curr);
    }
    else if(gstPh3CalcInfo.ferq_devision == 3)
    {
        Ph3CalcInfo_CopyPower(&gstPh3CalcInfo.grid_power, &src->grid.power);
    }
    else if(gstPh3CalcInfo.ferq_devision == 4)
    {
        gstPh3CalcInfo.grid_a_phase_count = src->grid.vol_a_phase_count;
        gstPh3CalcInfo.grid_b_phase_count = src->grid.vol_b_phase_count;
        gstPh3CalcInfo.grid_c_phase_count = src->grid.vol_c_phase_count;
    }
    else if(gstPh3CalcInfo.ferq_devision == 5)
    {
        Ph3CalcInfo_CopyVol(&gstPh3CalcInfo.gen_vol, &src->gen.vol);
    }
    else if(gstPh3CalcInfo.ferq_devision == 6)
    {
        Ph3CalcInfo_CopyCurr(&gstPh3CalcInfo.gen_curr, &src->gen.curr);
    }
    else if(gstPh3CalcInfo.ferq_devision == 8)
    {
        Ph3CalcInfo_CopyPower(&gstPh3CalcInfo.gen_power, &src->gen.power);
    }
    else if(gstPh3CalcInfo.ferq_devision == 9)
    {
        gstPh3CalcInfo.gen_a_phase_count = src->gen.vol_a_phase_count;
        gstPh3CalcInfo.gen_b_phase_count = src->gen.vol_b_phase_count;
        gstPh3CalcInfo.gen_c_phase_count = src->gen.vol_c_phase_count;
    }
    else if(gstPh3CalcInfo.ferq_devision == 10)
    {
        Ph3CalcInfo_CopyVol(&gstPh3CalcInfo.load_vol, &src->load.vol);
    }
    else if(gstPh3CalcInfo.ferq_devision == 11)
    {
        Ph3CalcInfo_CopyCurr(&gstPh3CalcInfo.load_curr, &src->load.curr);
    }
    else if(gstPh3CalcInfo.ferq_devision == 13)
    {
        Ph3CalcInfo_CopyPower(&gstPh3CalcInfo.load_power, &src->load.power);
    }
    else if(gstPh3CalcInfo.ferq_devision == 15)
    {
        Ph3CalcInfo_CopyVol(&gstPh3CalcInfo.inv_vol, &src->inv.vol);
    }
    else if(gstPh3CalcInfo.ferq_devision == 16)
    {
        Ph3CalcInfo_CopyCurr(&gstPh3CalcInfo.inv_curr, &src->inv.curr);
    }
    else if(gstPh3CalcInfo.ferq_devision == 18)
    {
        Ph3CalcInfo_CopyPower(&gstPh3CalcInfo.inv_power, &src->inv.power);
    }
    else if(gstPh3CalcInfo.ferq_devision == 19)
    {
        Ph3CalcInfo_CopyCurr(&gstPh3CalcInfo.ct_curr, &src->ct.curr);
    }
    else if(gstPh3CalcInfo.ferq_devision == 21)
    {
        Ph3CalcInfo_CopyPower(&gstPh3CalcInfo.ct_power, &src->ct.power);
    }
    else if(gstPh3CalcInfo.ferq_devision == 22)
    {
        gstPh3CalcInfo.rcmu_curr_rms = src->rcmu_curr_rms;
        gstPh3CalcInfo.mid_vol.a_phase_rms = src->mid_vol_a_rms;
        gstPh3CalcInfo.mid_vol.b_phase_rms = src->mid_vol_b_rms;
        gstPh3CalcInfo.mid_vol.c_phase_rms = src->mid_vol_c_rms;
    }

    gstPh3CalcInfo.ferq_devision++;
    if(gstPh3CalcInfo.ferq_devision >= 23U)
    {
        gstPh3CalcInfo.ferq_devision = 0U;
    }

    return true;
}
