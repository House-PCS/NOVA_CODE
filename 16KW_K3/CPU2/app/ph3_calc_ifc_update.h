/*
 * ph3_calc_ifc_update.h
 *
 * CPU2 ph3 calc interface update (GS4 shared RAM, consumed by CPU1).
 */
#ifndef APP_PH3_CALC_IFC_UPDATE_H_
#define APP_PH3_CALC_IFC_UPDATE_H_

#include "ph3_calc/ph3_calc_interface.h"

/** 母线滤波量 */
typedef struct ph3_calc_ifc_bus_stru_t
{
    float32 vol_filter;
    float32 curr_filter;
    float32 power_filter;
    float32 pos_vol_filter;
    float32 neg_vol_filter;
} Ph3CalcIfcBus_t;

/** CPU2 ph3 interface: grid / gen / load / inv / ct (placed in GS4 shared RAM) */
typedef struct dual_data_share_ph3_ifc_stru_t
{
    Ph3Calc_Interface_t grid;
    Ph3Calc_Interface_t gen;
    Ph3Calc_Interface_t load;
    Ph3Calc_Interface_t inv;
    Ph3Calc_Interface_t ct;
    Ph3CalcIfcBus_t bus;
    float32 rcmu_curr_rms;
    float32 mid_vol_a_rms;
    float32 mid_vol_b_rms;
    float32 mid_vol_c_rms;
} DualDataShare_Ph3Ifc_t;

extern DualDataShare_Ph3Ifc_t gstPh3CalcIfcUpdate;

void Ph3CalcIfcUpdate_Init(void);
void Ph3CalcIfcUpdate(Ph3Calc_Interface_t *ifc, const Ph3Calc_Channel_t *ch);
void Ph3CalcIfcAngleUpdate(Ph3Calc_Interface_t *ifc, const Ph3Calc_Channel_t *ch);
DualDataShare_Ph3Ifc_t *Ph3CalcIfcUpdate_Get(void);

#endif /* APP_PH3_CALC_IFC_UPDATE_H_ */
