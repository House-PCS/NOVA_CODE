//
// Created by Nova004693 on 24-6-20.
//

#ifndef RTTHREAD_FSM_H
#define RTTHREAD_FSM_H
#include "F2837xD_device.h"

/**
 * @brief 状态机状态转移结构体
 */
typedef struct fsm_transition_stru_t
{
    uint16_t(*condition)(void);  //转移条件

    void (*action)(void);       //转移动作

    struct fsm_state_stru_t* next_state;    //转移的下一个状态
} FsmTransition_t;

/**
 * @brief 状态机状态结构体
 */
typedef struct fsm_state_stru_t
{
    uint16_t id;             //状态ID
    uint16_t transition_num; //转移数量

    void (*func)(void);    //状态运行函数

    struct fsm_transition_stru_t* transitions;  //转移列表

} FsmState_t;

/**
 * @brief 状态机结构体
 */
typedef struct fsm_stru_t
{
    uint16_t current_state_id;               //当前状态ID
    struct fsm_state_stru_t* current_state; //当前状态
} Fsm_t;

int FsmHandler(Fsm_t* fsm);

#endif //RTTHREAD_FSM_H
