/**
 * @file pwr_ctrl_func.h
 * @brief 功率控制功能：调度逻辑与继电器控制
 */

#ifndef PWR_CTRL_FUNC_H
#define PWR_CTRL_FUNC_H

#include "energy_management_def.h"

void PwrCtrlInterface_Init(void);
void EnergyScheduling(void);

#endif // PWR_CTRL_FUNC_H
