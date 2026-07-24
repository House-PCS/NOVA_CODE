/**
 * @file biz_fsm_cfg.h
 * @brief PCS business FSM: fault/alarm/derate and related app-layer limits (excludes algo params)
 * @details Business thresholds and FSM criteria live here; fault/alarm codes in fault.h; algo in algo/pv_cfg.h
 */

#ifndef BIZ_FSM_CFG_H
#define BIZ_FSM_CFG_H

#include "F28x_Project.h"

/* PCS FSM: bus voltage criteria / debounce */
#define BIZ_FSM_BUS_VOLT_MIN_THRESHOLD   (350.0f)
#define BIZ_FSM_BUS_VOLT_MAX_THRESHOLD   (450.0f)
#define BIZ_FSM_BUS_VOLT_DLY_COUNT       (5U)

#endif /* BIZ_FSM_CFG_H */
