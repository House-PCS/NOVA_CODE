
/*
 * cla_control.h
 *
 *  Created on: 2024-08-27
 *      Author: Nova007456
 */
#ifndef MIDDLEWARE_CLA_ALGO_H_
#define MIDDLEWARE_CLA_ALGO_H_
#endif

#include "F28x_Project.h"
#include "F2837xD_Cla_defines.h"
#include "stdint.h"
#include "CLAMath.h"
#include "system.h"
#include "fault.h"
#include <sample_update.h>
#include "interrupt.h"
#include "cla_cpu_share.h"
#include "AlgoFw/package/cntl/inverter_3ph/inverter_3ph_calc_cla.h"
#include "AlgoFw/component/Comp_3phDecouple_cla.h"
#include "AlgoFw/component/Comp_Clark_cla.h"
#include "AlgoFw/component/Comp_Park_cla.h"
#include "pwr_ctrl_interface.h"
#include "inv_3ph_pwm.h"

extern CLAAdc_t  gstClaAdcResult;
extern Inv3Ph_Vloop_t gVloop_p_Cla;
extern Inv3Ph_Vloop_t gVloop_n_Cla;
extern Inv3Ph_Iloop_t gIloop_p_Cla;
extern Inv3Ph_Iloop_t gIloop_n_Cla;
extern Inv3Ph_Iloop_t gIloop_z_Cla;
extern Inv3Ph_Calc_Data_t gInv3Ph_CalcData_Cla;
//extern PwrCtrlInteface_t gstPwrCtrlInterface;





void SetClaDebugData(uint16_t buff_num,float value);

void InvClaCtrl_Init(void);
void InvClaCtrl_Run(void);
void InvClaCtrl_ClearLoops(void);
