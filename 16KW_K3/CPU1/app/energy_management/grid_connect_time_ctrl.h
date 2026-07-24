/**
 * @file grid_connect_time_ctrl.h
 * @brief 并网时间控制：离网冷却与并网超时许可
 */

#ifndef GRID_CONNECT_TIME_CTRL_H
#define GRID_CONNECT_TIME_CTRL_H

#include "energy_management_def.h"

typedef struct grid_connect_time_ctrl_output_stru_t
{
    uint16_t grid_enable;             /**< 1: 允许并网 (monitor_connect_permit) */
} GridConnectTimeCtrl_Output_t;

extern GridConnectTimeCtrl_Output_t gstGridConnectTimeCtrlOut;

void GridConnectTimeCtrl_Init(void);        
void GridConnectTimeCtrl_Update(void);

#endif // GRID_CONNECT_TIME_CTRL_H
