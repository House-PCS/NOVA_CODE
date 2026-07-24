/**
 * @file biz_fsm_flag_trigger.h
 * @brief Debounced condition-to-flag trigger macros for biz_fsm
 */

#ifndef BIZ_FSM_FLAG_TRIGGER_H
#define BIZ_FSM_FLAG_TRIGGER_H

#include "flag_dly_trigger.h"

#define BIZ_FSM_FLAG_DLY_DECLARE(id, p_flag, threshold)  FLAG_DLY_SET_DECLARE(id, p_flag, threshold)
#define BIZ_FSM_FLAG_DLY_RUN(id, cmd)                     FLAG_DLY_SET_RUN(id, cmd)
#define BIZ_FSM_FLAG_DLY_RESET(id, p_flag)               FLAG_DLY_SET_RESET(id, p_flag)

#endif /* BIZ_FSM_FLAG_TRIGGER_H */
