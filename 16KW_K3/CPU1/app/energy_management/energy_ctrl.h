/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        energy_ctrl.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-07-20
  * @brief       N32G4FR
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2025-08-21    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#ifndef __ENERGY_CTRL_H_
#define __ENERGY_CTRL_H_

#include "stdlib.h"
#include "stdint.h"
  /*并离网状态*/
typedef enum
{
    EN_STAND_BY_MODE,//0
    EN_OFF_GRID_MODE,
    EN_OFF_GRID_TO_GRID_CONNECT_MODE,
    EN_GRID_CONNECT_MODE,
    EN_GRID_CONNECT_TO_OFF_GRID_MODE,
} EN_PARALLEL_OFF_NETWORK_STA;

/*工作模式*/
typedef enum
{
    EN_SPONTANEOUS_USE_MODE,
    EN_BAT_PRIORITY_MODE,
    EN_PEAK_VALLEY_MODE,
    EN_OIL_MODE,
} EN_WORK_MODE;

/*输入功率获取，单位W*/
typedef struct _energy_ctrl_input_stru_t
{
    /*设备预设的值*/
    EN_PARALLEL_OFF_NETWORK_STA     inv_stat;
    EN_WORK_MODE                    work_mode;
    unsigned char       ev_mode;//0-EV给设备充电，1-设备给EV充电

    unsigned char       antireverseflow_enable;//防逆流开关
    float       power_max_household_get;//最大入户功率
    float       power_max_ongrid_get;//最大并网功率
    float       power_max_gen_get;//油机最大功率

    float       power_max_bat_chg_get;//电池最大充电功率
    float       power_max_bat_dsg_get;//电池最大放电功率

    // uint8_t      bat_stat;//电池状态，0-正常，1-禁充，2-禁放
    unsigned char       peak_shift_stat;//削峰填谷下的三种状态，0-静止，1-充电，2-放电

    /*实时回读的值*/
    float       power_pv2bus_get;

    float       power_ev2bus_get;
    float       power_bat2bus_get;
    float       power_bus2pcs_get;//由上述两个变量求和计算

    //有功
    float       power_act_ct2grid_get;//CT功率，流入设备为正
    float       power_act_norload_get;//远端负载功率，可能存在负值

    float       power_act_grid2pcs_get;
    float       power_act_gen2pcs_get;
    float       power_act_eps_get;//本地负载功率，恒为正值
    float       power_act_inv2pcs_get;//由上述三个变量求和计算(eps取反)

    //无功
    float       power_react_ct2grid_get;//CT功率，流入设备为正
    float       power_react_norload_get;//远端负载功率，可能存在负值

    float       power_react_grid2pcs_get;
    float       power_react_gen2pcs_get;
    float       power_react_eps_get;//本地负载功率，恒为正值
    float       power_react_inv2pcs_get;//由上述三个变量求和计算(eps取反)

    //视在
    float       power_appar_ct2grid_get;//CT功率，流入设备为正
    float       power_appar_norload_get;//远端负载功率，可能存在负值

    float       power_appar_grid2pcs_get;
    float       power_appar_gen2pcs_get;
    float       power_appar_eps_get;//本地负载功率，恒为正值
    float       power_appar_inv2pcs_get;//由上述三个变量求和计算(eps取反)

    //并网认证测试开关
    int16_t     flag_netdisabletest_enable; //模拟断网开关，由监控卡下发，取bit0
    int16_t     flag_hardexportlimit_enable;//输出功率硬限制开关，由监控卡下发，取bit1
    int16_t     flag_softexportlimit_enable;//输出功率软限制开关，由监控卡下发，取bit2
    int16_t     flag_powerratelimit_enable; //功率变化率限制开关，由监控卡下发，取bit3
}EnergyCtrlInput_t;



typedef struct _energy_ctrl_limit_stru_t
{
    // float        power_ct2grid_max_limit;
    // float        power_ct2grid_min_limit;
    float       power_grid2pcs_max_limit;
    float       power_grid2pcs_min_limit;
    float       power_gen2pcs_max_limit;
    float       power_gen2pcs_min_limit;
    float       power_inv2pcs_max_limit;
    float       power_inv2pcs_min_limit;

    float       power_bus2pcs_max_limit;
    float       power_bus2pcs_min_limit;
    float       power_bat2bus_max_limit;//电池最大放电功率
    float       power_bat2bus_min_limit;//电池最大充电功率
    float       power_ev2bus_max_limit;//EV最大输入功率
    float       power_ev2bus_min_limit;//EV最大输出功率
}EnergyCtrlLimit_t;



typedef struct _energy_ctrl_output_stru_t
{
    float       power_pv2bus_set;
    float       power_ev2bus_set;//期望值
    float       power_bat2bus_set;//期望值，用于计算逆变和光伏如何调节

    float       power_act_grid2pcs_set;
    float       power_react_grid2pcs_set;
    float       power_appar_grid2pcs_set;

    float       power_act_gen2pcs_set;
    float       power_react_gen2pcs_set;
    float       power_appar_gen2pcs_set;

    float       power_bat2bus_target; //20260324，新增电池功率期望值，用于控制输出功率变化率
}EnergyCtrlOutput_t;

EnergyCtrlInput_t* GetEnergyInput(void);
void EnergyCtrlProcess();
EnergyCtrlOutput_t* GetEnergyOutput(void);
void CertificationTest(EnergyCtrlInput_t *p_input, EnergyCtrlOutput_t *p_output);

extern  EnergyCtrlInput_t gEnergyCtrlInput;
extern  EnergyCtrlLimit_t gEnergyCtrlLimit;
extern  EnergyCtrlOutput_t gEnergyCtrlOutput;


#define POWER_VALUE_LIMIT(x,min,max)      (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))
#define POWER_GET_MIN(power1, power2)     (((power1) < (power2)) ? (power1) : (power2))


#endif
