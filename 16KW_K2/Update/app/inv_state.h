/*
 * inv_state.h
 *
 *  Created on: 2025年3月26日
 *      Author: Nova006352
 */

#ifndef APP_INV_STATE_H_
#define APP_INV_STATE_H_

#include "F28x_Project.h"

typedef struct inv_action_permit_stru_t
{
    uint16_t pv_disconnect_permit;          //PV离网许可
    uint16_t bus_disconnect_permit;         //bus离网许可
    uint16_t grid_disconnect_permit;        //市电离网许可
    uint16_t gen_disconnect_permit;         //发电机离网许可
    uint16_t monitor_disconnect_permit;     //监控离网许可
    uint16_t self_test_disconnect_permit;   //自检无问题离网许可
    uint16_t parallel_disconnect_permit;    //并机离网许可
    uint16_t inv_inside_disconnect_permit;  //逆变内部离网许可

    uint16_t pv_connect_permit;             //PV并网许可
    uint16_t bus_connect_permit;            //bus并网许可
    uint16_t grid_connect_permit;           //市电并网许可
    uint16_t gen_connect_permit;            //发电机并网许可
    uint16_t monitor_connect_permit;        //监控并网许可
    uint16_t gen_enable;                    //强制油机模式使能
    uint16_t parallel_connect_permit;       //并机并网许可
    uint16_t inv_inside_connect_permit;     //逆变内部并网许可

    uint16_t main_relay_permit;             //主继电器闭合许可
    uint16_t load_relay_permit;             //负载电器闭合许可
    uint16_t grid_relay_permit;             //市电电器闭合许可
    uint16_t gen_relay_permit;              //发电机电器闭合许可
} InvActionPermit_t;

void Inv_State_Int1(void);
void InvWorkPermit_Monitor(uint16_t en_inv, uint16_t en_grid, uint16_t gen_enable);
extern InvActionPermit_t gstInvAction; //逆变动作权利许可

#endif /* APP_INV_STATE_H_ */
