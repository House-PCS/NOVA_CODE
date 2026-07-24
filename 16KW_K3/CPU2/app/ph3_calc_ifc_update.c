/*
 * ph3_calc_ifc_update.c
 *
 * CPU2 ph3 calc interface update (GS4 shared RAM, consumed by CPU1).
 */
#include "ph3_calc_ifc_update.h"

#pragma CODE_SECTION(Ph3CalcIfcUpdate, ".TI.ramfunc");
#pragma CODE_SECTION(Ph3CalcIfcAngleUpdate, ".TI.ramfunc");

#pragma DATA_SECTION(gstPh3CalcIfcUpdate, "ramgs1")
DualDataShare_Ph3Ifc_t gstPh3CalcIfcUpdate;

static void Ph3CalcIfcBus_Init(Ph3CalcIfcBus_t *bus)
{
    bus->vol_filter = 0.0f;
    bus->curr_filter = 0.0f;
    bus->power_filter = 0.0f;
    bus->pos_vol_filter = 0.0f;
    bus->neg_vol_filter = 0.0f;
}

void Ph3CalcIfcUpdate_Init(void)
{
    Ph3Calc_InterfaceInit(&gstPh3CalcIfcUpdate.grid);
    Ph3Calc_InterfaceInit(&gstPh3CalcIfcUpdate.gen);
    Ph3Calc_InterfaceInit(&gstPh3CalcIfcUpdate.load);
    Ph3Calc_InterfaceInit(&gstPh3CalcIfcUpdate.inv);
    Ph3Calc_InterfaceInit(&gstPh3CalcIfcUpdate.ct);
    Ph3CalcIfcBus_Init(&gstPh3CalcIfcUpdate.bus);
    gstPh3CalcIfcUpdate.rcmu_curr_rms = 0.0f;
    gstPh3CalcIfcUpdate.mid_vol_a_rms = 0.0f;
    gstPh3CalcIfcUpdate.mid_vol_b_rms = 0.0f;
    gstPh3CalcIfcUpdate.mid_vol_c_rms = 0.0f;
}

void Ph3CalcIfcUpdate(Ph3Calc_Interface_t *ifc, const Ph3Calc_Channel_t *ch)
{
    if((ifc == NULL) || (ch == NULL))
    {
        return;
    }

    Ph3Calc_InterfaceUpdate(ifc, ch);
}

void Ph3CalcIfcAngleUpdate(Ph3Calc_Interface_t *ifc, const Ph3Calc_Channel_t *ch)
{
    if((ifc == NULL) || (ch == NULL))
    {
        return;
    }

    Ph3Calc_InterfaceAngleUpdate(ifc, ch);
}

DualDataShare_Ph3Ifc_t *Ph3CalcIfcUpdate_Get(void)
{
    return &gstPh3CalcIfcUpdate;
}
