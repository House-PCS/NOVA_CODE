/**
 * @file BizFw_Fsm.h
 * @brief
 */

#ifndef BIZFW_FSM_H
#define BIZFW_FSM_H

#include "F28x_Project.h"

typedef struct bizfw_fsm_transition_stru_t
{
    uint16_t (*condition)(void* context);
    void (*action)(void* context);
    struct bizfw_fsm_state_stru_t* next_state;
} BizFw_FsmTransition_t;

typedef struct bizfw_fsm_state_stru_t
{
    uint16_t state_id;
    uint16_t transition_count;
    void (*function)(void* context);
    void (*on_exit)(void* context); /**< 离开本状态时调用（转移条件已满足、切换前） */
    BizFw_FsmTransition_t* transitions;
    uint32_t running_time_count;
} BizFw_FsmState_t;

typedef struct bizfw_fsm_stru_t
{
    uint16_t current_state_id;
    BizFw_FsmState_t* current_state;
} BizFw_Fsm_t;

int BizFw_Fsm_Init(BizFw_Fsm_t* fsm, BizFw_FsmState_t* initial_state);
int BizFw_Fsm_Update(BizFw_Fsm_t* fsm, void* context);

#endif /* BIZFW_FSM_H */
