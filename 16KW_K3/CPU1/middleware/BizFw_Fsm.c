/**
 * @file BizFw_Fsm.c
 * @brief 有限状态机通用框架实现
 */

#include "BizFw_Fsm.h"

int BizFw_Fsm_Init(BizFw_Fsm_t* fsm, BizFw_FsmState_t* initial_state)
{
    if(fsm == NULL || initial_state == NULL) {
        return -1;
    }

    fsm->current_state = initial_state;
    fsm->current_state_id = initial_state->state_id;
    initial_state->running_time_count = 0U;

    return 0;
}

int BizFw_Fsm_Update(BizFw_Fsm_t* fsm, void* context)
{
    uint16_t i;

    if(fsm == NULL || fsm->current_state == NULL) {
        return -1;
    }

    if(fsm->current_state->function != NULL) {
        fsm->current_state->function(context);
        fsm->current_state->running_time_count++;
    }

    for(i = 0; i < fsm->current_state->transition_count; ++i) {
        BizFw_FsmTransition_t* transition = &fsm->current_state->transitions[i];

        if(transition->condition != NULL && transition->condition(context)) {
            BizFw_FsmState_t* leaving_state = fsm->current_state;

            if(transition->next_state == NULL) {
                return -1;
            }

            if(leaving_state->on_exit != NULL) {
                leaving_state->on_exit(context);
            }

            if(transition->action != NULL) {
                transition->action(context);
            }

            fsm->current_state = transition->next_state;
            fsm->current_state_id = fsm->current_state->state_id;
            fsm->current_state->running_time_count = 0U;

            break;
        }
    }

    return 0;
}
