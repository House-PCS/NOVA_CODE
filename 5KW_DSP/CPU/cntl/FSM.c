//
// Created by Nova004693 on 24-6-20.
//

#include "FSM.h"

#include "F2837xD_device.h"

int FsmHandler(Fsm_t* fsm)
{
    if(fsm == 0)
    {
        return -1;
    }
    if(fsm->current_state == 0)
    {
        return -2;
    }

    fsm->current_state_id = fsm->current_state->id;

    /*轮询状态转移列表，判断是否需要转移状态*/
    for(int i = 0; i < fsm->current_state->transition_num; ++i)
    {
        if(fsm->current_state->transitions == 0)
        {
            break;
        }
        FsmTransition_t* transition = &fsm->current_state->transitions[i];
        if(transition->condition == 0)
        {
            continue;
        }
      /*获取此转移的条件并判断*/
        uint8_t rst = transition->condition();
        if(rst == 0)
        {
            continue;
        }
      /*如果有转移动作，则执行动作*/
        if(transition->action != 0)
        {
            transition->action();
        }

        if(transition->next_state == 0)
        {
            continue;
        }
        fsm->current_state = transition->next_state;
        break;
    }
    if(fsm->current_state->func == 0)
    {
        return -3;
    }

    fsm->current_state->func();
    return 0;
}
