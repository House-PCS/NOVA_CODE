/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        energy_ctrl.c
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
#include "energy_ctrl.h"

/*科二规格*/
#define GRID_PORT_MAX_POWER     24000
#define PCS_MAX_POWER           17500
#define PV_MAX_POWER            24000
#define EV_MAX_POWER            25000

#define ENABLE_BAT_ERROR_CORR   0
#define BAT_ERROR_CORR_FACTOR   0.02
#define ENABLE_DEV_LOSS_POWER   0


EnergyCtrlInput_t gEnergyCtrlInput;
EnergyCtrlLimit_t gEnergyCtrlLimit;
EnergyCtrlOutput_t gEnergyCtrlOutput;


EnergyCtrlInput_t* GetEnergyInput(void)
{
    return &gEnergyCtrlInput;
}

EnergyCtrlOutput_t* GetEnergyOutput(void)
{
    return &gEnergyCtrlOutput;
}


/**
* @brief: 获取能量调度模块的输入参数
* @param: p_input:能量调度模块输入信息指针
* @return: none
* @note:  
*/
static void _GetEnergyInput(EnergyCtrlInput_t *p_input)
{


}





/**
* @brief: 并离网模式下各类限制计算
* @param: on_grid_flag:并离网标志，1表示并网
* @param: p_input:能量调度模块输入信息指针
* @param: p_limit:能量调度模块各端口限制参数指针
* @return: none
* @note:  
*/
static void _OnOffGridLimitCalc(unsigned char on_grid_flag, EnergyCtrlInput_t *p_input, EnergyCtrlLimit_t *p_limit)
{
    /*并网口实时功率限制计算*/
    if(on_grid_flag == 1)//并网模式，非油机模式
    {
        float nload_npcs_total_power = p_input->power_act_ct2grid_get - p_input->power_act_grid2pcs_get;
        float max_ongrid_power;//通过CT流出的最大功率(并到电网的最大功率，正值)

        p_input->power_act_norload_get = nload_npcs_total_power;

        if(p_input->flag_netdisabletest_enable == 1)
        {
            p_input->power_act_norload_get = 0;
        }
        /*并网口流入最大*/
        //先按最大入户功率判断
        p_limit->power_grid2pcs_max_limit = p_input->power_max_household_get - p_input->power_act_norload_get;
        //再按端口限制判断
        if(p_limit->power_grid2pcs_max_limit > GRID_PORT_MAX_POWER)
        {
            p_limit->power_grid2pcs_max_limit = GRID_PORT_MAX_POWER;
        }
        //再按照逆变参数判断
        if(p_limit->power_grid2pcs_max_limit > p_input->power_act_eps_get + PCS_MAX_POWER)
        {
            p_limit->power_grid2pcs_max_limit = p_input->power_act_eps_get + PCS_MAX_POWER;
        }

        /*并网口流出最大*/
        if(p_input->antireverseflow_enable == 1)//防逆流打开
        {
            max_ongrid_power = 0;
        }
        else if(p_input->work_mode == EN_PEAK_VALLEY_MODE && p_input->peak_shift_stat != 2)//削峰填谷非卖电状态
        {
            max_ongrid_power = 0;
        }
        else
        {
            max_ongrid_power = POWER_GET_MIN(p_input->power_max_household_get, p_input->power_max_ongrid_get);
        }
        //先按最大入户功率及最大并网功率判断
        p_limit->power_grid2pcs_min_limit = (0 - max_ongrid_power) - p_input->power_act_norload_get;
        //再按端口限制判断
        if(p_limit->power_grid2pcs_min_limit < (0 - GRID_PORT_MAX_POWER))
        {
            p_limit->power_grid2pcs_min_limit = (0 - GRID_PORT_MAX_POWER);
        }
        //再按照逆变参数判断
        if(p_limit->power_grid2pcs_min_limit < (p_input->power_act_eps_get - PCS_MAX_POWER))
        {
            p_limit->power_grid2pcs_min_limit = (p_input->power_act_eps_get - PCS_MAX_POWER);
        }

        /*油机口输入最大最小*/
        p_limit->power_gen2pcs_max_limit = 0;
        p_limit->power_gen2pcs_min_limit = 0;
    }
    else//离网模式
    {
        /*并网口流入最大和流出最大*/
        p_limit->power_grid2pcs_max_limit = 0;
        p_limit->power_grid2pcs_min_limit = 0;
        /*油机口输入最大最小*/
        if(p_input->work_mode != EN_OIL_MODE)//离网非油机模式
        {
            p_limit->power_gen2pcs_max_limit = 0;
            p_limit->power_gen2pcs_min_limit = 0;//防止因采样误差导致逆流给油机
        }
        else//离网油机模式
        {
            p_limit->power_gen2pcs_max_limit = p_input->power_max_gen_get;
            p_limit->power_gen2pcs_min_limit = 100;//防止因采样误差导致逆流给油机
        }
    }


    /*交流侧流入PCS的最大功率*/
    p_limit->power_inv2pcs_max_limit = p_limit->power_gen2pcs_max_limit + p_limit->power_grid2pcs_max_limit - p_input->power_act_eps_get;
    /*交流侧从PCS流出的最大功率*/
    p_limit->power_inv2pcs_min_limit = p_limit->power_gen2pcs_min_limit + p_limit->power_grid2pcs_min_limit - p_input->power_act_eps_get;

    /*直流侧流入PCS的最大功率*/
    p_limit->power_bus2pcs_max_limit = 0 - p_limit->power_inv2pcs_min_limit;
    /*直流侧从PCS流出的最大功率*/
    p_limit->power_bus2pcs_min_limit = 0 - p_limit->power_inv2pcs_max_limit;

    /*电池最大和最小功率*/
    p_limit->power_bat2bus_max_limit = p_input->power_max_bat_dsg_get;
    p_limit->power_bat2bus_min_limit = 0 - p_input->power_max_bat_chg_get;

    /*EV充电和放电最大功率*/
    if(p_input->ev_mode == 0)//EV给设备充电
    {
        p_limit->power_ev2bus_max_limit = p_limit->power_bus2pcs_max_limit - p_input->power_pv2bus_get - p_limit->power_bat2bus_min_limit;
        if(p_limit->power_ev2bus_max_limit < 0)
        {
            p_limit->power_ev2bus_max_limit = 0;
        }
        if(p_limit->power_ev2bus_max_limit < EV_MAX_POWER)
        {
            p_limit->power_ev2bus_max_limit = EV_MAX_POWER;
        }
        p_limit->power_ev2bus_min_limit = 0;
    }
    else//设备给EV充电
    {
        p_limit->power_ev2bus_max_limit = 0;
        p_limit->power_ev2bus_min_limit = p_limit->power_bus2pcs_min_limit - p_input->power_pv2bus_get - p_limit->power_bat2bus_max_limit;
        if(p_limit->power_ev2bus_min_limit > 0)
        {
            p_limit->power_ev2bus_min_limit = 0;
        }
        if(p_limit->power_ev2bus_min_limit > 0 - EV_MAX_POWER)
        {
            p_limit->power_ev2bus_min_limit = 0 - EV_MAX_POWER;
        }
    }
}



/**
* @brief: 基于电池状态对电池功率的修正
* @param: p_input:能量调度模块输入信息指针
* @param: p_limit:能量调度模块各端口限制参数指针
* @param: p_output:能量调度计算后各模块的功率信息指针
* @return: none
* @note:  
*/
static void _BatPowerLimitConvert(const EnergyCtrlInput_t *p_input,EnergyCtrlLimit_t *p_limit, EnergyCtrlOutput_t *p_output)
{
    if(p_input->power_max_bat_chg_get == 0 && p_input->power_max_bat_dsg_get == 0)//禁充禁放
    {
        p_output->power_bat2bus_set = 0;
    }
    else if(p_input->power_max_bat_chg_get == 0)//禁充
    {
        if(p_output->power_bat2bus_set < 0)
        {
            p_output->power_bat2bus_set = 100;
        }
    }
    else if(p_input->power_max_bat_dsg_get == 0)//禁放
    {
        if(p_output->power_bat2bus_set > 0)
        {
            p_output->power_bat2bus_set = -100;
        }
    }
    p_output->power_bat2bus_set = POWER_VALUE_LIMIT(p_output->power_bat2bus_set, p_limit->power_bat2bus_min_limit, p_limit->power_bat2bus_max_limit);
}


/**
* @brief: 直流母线侧功率限制
* @param: p_input:能量调度模块输入信息指针
* @param: p_limit:能量调度模块各端口限制参数指针
* @param: p_output:能量调度计算后各模块的功率信息指针
* @return: none
* @note:  
*/
static void _DcBusPowerLimit(const EnergyCtrlInput_t *p_input,EnergyCtrlLimit_t *p_limit, EnergyCtrlOutput_t *p_output)
{
    if((p_input->power_pv2bus_get + p_input->power_ev2bus_get) + p_output->power_bat2bus_set > PCS_MAX_POWER)
    {
        p_output->power_bat2bus_set = PCS_MAX_POWER - (p_input->power_pv2bus_get + p_input->power_ev2bus_get);
    }
    else if((p_input->power_pv2bus_get + p_input->power_ev2bus_get) + p_output->power_bat2bus_set < -PCS_MAX_POWER)
    {
        p_output->power_bat2bus_set = -PCS_MAX_POWER - (p_input->power_pv2bus_get + p_input->power_ev2bus_get);
    }
    //再次对电池功率进行限制计算
    _BatPowerLimitConvert(p_input, p_limit, p_output);
}


/**
* @brief: 电池侧功率修正
* @param: origin:修正前的电池功率
* @param: error:上一拍实际电池功率和电池设置功率的差值
* @return: 修正值
* @note:  
*/
static float _BatPowerCorr(float origin, float error)
{
    static float accu_corr_value = 0;//累积修正值
    float res;
#if (ENABLE_BAT_ERROR_CORR == 0)
    accu_corr_value = 0;
#else
    /*方案1*/
    accu_corr_value += error * BAT_ERROR_CORR_FACTOR;
    /*方案2*/
    //accu_corr_value = error;

#endif
    if(accu_corr_value > 2000)
    {
        accu_corr_value = 2000;
    }
    else if(accu_corr_value < -2000)
    {
        accu_corr_value = -2000;
    }
    res = origin - accu_corr_value;

    return res;
}


/**
* @brief: 电池侧功率修正2--仅修正电池功率是否超过界限
* @param: origin:修正前的电池功率
* @param: get:上一拍实际电池功率
* @param: p_limit:能量调度模块各端口限制参数指针
* @return: 修正值
* @note:  
*/
static float _BatPowerCorr2(float origin, float get, EnergyCtrlLimit_t *p_limit)
{
    static float accu_corr_value = 0;//累积修正值
    float res;
#if (ENABLE_BAT_ERROR_CORR == 0)
    accu_corr_value = 0;
#else
    if(get > p_limit->power_bat2bus_max_limit)
    {
        accu_corr_value = (get - p_limit->power_bat2bus_max_limit);
    }
    else if(get < p_limit->power_bat2bus_min_limit)
    {
        accu_corr_value = (get - p_limit->power_bat2bus_min_limit);
    }
    else
    {
        accu_corr_value = 0;
    }
#endif
    res = origin - accu_corr_value;

    return res;
}





/**
* @brief: 并网模式下功率调度计算
* @param: p_input:能量调度模块输入信息指针
* @param: p_limit:能量调度模块各端口限制参数指针
* @param: p_output:能量调度计算后各模块的功率信息指针
* @return: none
* @note:  
*/
static void _OnGridEnergyCtrlCalc(EnergyCtrlInput_t *p_input, EnergyCtrlLimit_t *p_limit, EnergyCtrlOutput_t *p_output)
{
    float correct_grid2pcs_power;//结合限制，应当的并网口功率
    float pv_dist_value = 0;//PV尝试扰动值
    float bat_chg_margin, bat_dsg_margin;//电池充电余量(即电池功率预设值-最小功率限制)和放电余量(最大功率限制-电池功率预设值)，都是正值
    float bat_power_error = p_input->power_bat2bus_get - p_output->power_bat2bus_set;//电池实际功率和预设误差
    float power_bat2bus_set_corr;//修正后的电池功率

    if(p_input->flag_netdisabletest_enable == 1)
    {
        p_input->power_act_norload_get = 0;
    }

    p_output->power_act_gen2pcs_set = 0;
#if ENABLE_DEV_LOSS_POWER == 0
    float power_loss = 0;
#else
    float power_loss = (p_input->power_pv2bus_get + p_input->power_ev2bus_get) + p_input->power_bat2bus_get + p_input->power_act_grid2pcs_get - p_input->power_act_eps_get;//设备损耗功率
#endif
    switch(p_input->work_mode)
    {
        case EN_SPONTANEOUS_USE_MODE://自发自用
        {
            /*自发自用处于并网模式下，光伏和电池共同负责本地负载和远端负载，优先由PV给本地负载和远端负载供电，当PV能量不足时，电池弥补，当电池供电不足时，由电网弥补*/
            float nload_npcs_total_power = p_input->power_act_norload_get;//p_input->power_act_ct2grid_get - p_input->power_act_grid2pcs_get;
            // float total_load_power = p_input->power_act_norload_get + p_input->power_act_eps_get;

            float total_load_power = nload_npcs_total_power + p_input->power_act_eps_get;
            total_load_power = POWER_VALUE_LIMIT(total_load_power, -PCS_MAX_POWER, PCS_MAX_POWER);//这里的总负载功率指的是经过逆变的功率，计算电池功率，所以要用逆变器最大功率限制
            p_output->power_bat2bus_target = total_load_power + power_loss - (p_input->power_pv2bus_get + p_input->power_ev2bus_get);
            // p_output->power_bat2bus_set = total_load_power + power_loss - (p_input->power_pv2bus_get + p_input->power_ev2bus_get);
            break;
        }
        case EN_BAT_PRIORITY_MODE://电池优先
        {
            /*电池优先模式下，光伏和电网共同负责本地负载和远端负载，优先由PV给本地负载和远端负载供电，当PV能量不足时，电网弥补，电网因限制导致不足时，电池弥补(基本不会发生)*/
            p_output->power_bat2bus_target = 0 - p_input->power_max_bat_chg_get;
            // p_output->power_bat2bus_set = 0 - p_input->power_max_bat_chg_get;
            break;
        }
        case EN_PEAK_VALLEY_MODE://削峰填谷
        {
            /*削峰填谷模式，静态模式下，不向电网卖电(能量富余且无需充电则降光伏)，优先由PV给本地负载和远端负载供电，当PV能量不足时，电池弥补，当电池供电不足时，由电网弥补。买电模式下，不向电网卖电(能量富余且无需充电则降光伏)，优先由PV给本地负载和远端负载供电以及电池补电，当PV能量不足时，电网弥补，电网因限制不足时，电池才介入(基本不会发生)。卖电模式下，优先由PV+电池给本地负载和远端负载供电，如能量有富余，则按照不超过最大并网功率输出给电网(满足并网限制前提下电池出全力)*/
            if(p_input->peak_shift_stat == 0)//静态模式，类似自发自用但不并网
            {
                float nload_npcs_total_power = p_input->power_act_norload_get;//p_input->power_act_ct2grid_get - p_input->power_act_grid2pcs_get;
                // float total_load_power = p_input->power_act_norload_get + p_input->power_act_eps_get;
                float total_load_power = nload_npcs_total_power + p_input->power_act_eps_get;
                total_load_power = POWER_VALUE_LIMIT(total_load_power, -PCS_MAX_POWER, PCS_MAX_POWER);//这里的总负载功率指的是经过逆变的功率，计算电池功率，所以要用逆变器最大功率限制
                p_output->power_bat2bus_target = total_load_power + power_loss - (p_input->power_pv2bus_get + p_input->power_ev2bus_get);//不向电网卖电的限制由p_limit->power_grid2pcs_min_limit限制
                // p_output->power_bat2bus_set = total_load_power + power_loss - (p_input->power_pv2bus_get + p_input->power_ev2bus_get);//不向电网卖电的限制由p_limit->power_grid2pcs_min_limit限制
            }
            else if(p_input->peak_shift_stat == 1)//买电模式，类似电池优先
            {
                p_output->power_bat2bus_target = 0 - p_input->power_max_bat_chg_get;//不向电网卖电的限制由p_limit->power_grid2pcs_min_limit限制
                // p_output->power_bat2bus_set = 0 - p_input->power_max_bat_chg_get;//不向电网卖电的限制由p_limit->power_grid2pcs_min_limit限制
            }
            else if(p_input->peak_shift_stat == 2)//卖电模式
            {
                float grid2pcs_min_power = p_limit->power_grid2pcs_min_limit;//卖电情况下并网端口最小值，一般为负值
                p_output->power_bat2bus_target = power_loss + p_input->power_act_eps_get - grid2pcs_min_power - (p_input->power_pv2bus_get + p_input->power_ev2bus_get);
                // p_output->power_bat2bus_set = power_loss + p_input->power_act_eps_get - grid2pcs_min_power - (p_input->power_pv2bus_get + p_input->power_ev2bus_get);
            }
            break;
        }
        default:break;
    }

    // 20260324，新增并网认证测试，电池功率限制
    CertificationTest(p_input, p_output);
    //////////////////////////////////////////////////////////////////////////////////////////////////////////

    _BatPowerLimitConvert(p_input, p_limit, p_output);//结合电池状况，对电池功率进行限制
    //_DcBusPowerLimit(p_input, p_limit, p_output);//结合逆变器状况，对电池功率进行限制
    power_bat2bus_set_corr = _BatPowerCorr(p_output->power_bat2bus_set, bat_power_error);
    //power_bat2bus_set_corr = _BatPowerCorr2(p_output->power_bat2bus_set, p_input->power_bat2bus_get, p_limit);
    bat_chg_margin = p_output->power_bat2bus_set - p_limit->power_bat2bus_min_limit;
    bat_dsg_margin = p_limit->power_bat2bus_max_limit - p_output->power_bat2bus_set;

    p_output->power_act_grid2pcs_set = p_input->power_act_eps_get + power_loss - (p_input->power_pv2bus_get + p_input->power_ev2bus_get) - power_bat2bus_set_corr;

    correct_grid2pcs_power = p_output->power_act_grid2pcs_set;
    correct_grid2pcs_power = POWER_VALUE_LIMIT(correct_grid2pcs_power, p_limit->power_grid2pcs_min_limit, p_limit->power_grid2pcs_max_limit);

    //并网量超过限制(p_output->power_act_grid2pcs_set < correct_grid2pcs_power)，则按照电池限制吸收，如电池不足以吸收，则降光伏(电池吸收后剩余的能量)
    if(correct_grid2pcs_power - p_output->power_act_grid2pcs_set > 0)
    {
        float gen2pcs_diff = correct_grid2pcs_power - p_output->power_act_grid2pcs_set;
        float bat_absorb_power = POWER_GET_MIN(bat_chg_margin, gen2pcs_diff);//电池吸收部分

        p_output->power_act_grid2pcs_set += bat_absorb_power;
        p_output->power_bat2bus_set -= bat_absorb_power;
        if(gen2pcs_diff > bat_chg_margin)//电池不足以吸收,尝试降光伏
        {
            float pv_reduce_power = POWER_GET_MIN(gen2pcs_diff - bat_chg_margin, p_input->power_pv2bus_get);

            p_output->power_pv2bus_set = p_input->power_pv2bus_get - pv_reduce_power;
            p_output->power_act_grid2pcs_set += pv_reduce_power;
        }
    }
    //电网吸收量超过限制(p_output->power_act_grid2pcs_set > correct_grid2pcs_power)，则按照电池限制弥补，如电池不足以弥补，则尝试增加光伏(电池弥补后剩余的能量)
    else if(correct_grid2pcs_power - p_output->power_act_grid2pcs_set < 0)
    {
        float gen2pcs_diff = p_output->power_act_grid2pcs_set - correct_grid2pcs_power;
        float bat_comp_power = POWER_GET_MIN(bat_dsg_margin, gen2pcs_diff);//电池补偿部分

        p_output->power_act_grid2pcs_set -= bat_comp_power;
        p_output->power_bat2bus_set += bat_comp_power;
        if(gen2pcs_diff > bat_dsg_margin)//电池不足以弥补，尝试增加光伏(不在此处增加，通过扰动缓慢增加)
        {
            //p_output->power_pv2bus_set = p_input->power_pv2bus_get + (p_output->power_act_grid2pcs_set - correct_grid2pcs_power - bat_dsg_margin);
        }
    }
    else
    {
        p_output->power_pv2bus_set = p_input->power_pv2bus_get;
    }
    p_output->power_ev2bus_set = p_input->power_ev2bus_get;

    if(p_output->power_pv2bus_set == p_input->power_pv2bus_get)//当期望的光伏等于当前实际值时，可尝试增加光伏
    {
        pv_dist_value = 100;
    }
    p_output->power_pv2bus_set += pv_dist_value;

    /*防止因光伏太大导致设备逆变过载*/
    if((p_output->power_pv2bus_set + p_output->power_ev2bus_set) + p_output->power_bat2bus_set > PCS_MAX_POWER + pv_dist_value + power_loss)
    {
        p_output->power_pv2bus_set = PCS_MAX_POWER - p_output->power_bat2bus_set - p_output->power_ev2bus_set;
    }
    /*光伏不能为负值*/
    if(p_output->power_pv2bus_set < 0)
    {
        p_output->power_pv2bus_set = 0;
    }
    /*光伏不能超过阈值*/
    if(p_output->power_pv2bus_set > PV_MAX_POWER)
    {
        p_output->power_pv2bus_set = PV_MAX_POWER;
    }
}




/**
* @brief: 离网模式下功率调度计算
* @param: p_input:能量调度模块输入信息指针
* @param: p_limit:能量调度模块各端口限制参数指针
* @param: p_output:能量调度计算后各模块的功率信息指针
* @return: none
* @note:  
*/
static void _OffGridEnergyCtrlCalc(EnergyCtrlInput_t *p_input, EnergyCtrlLimit_t *p_limit, EnergyCtrlOutput_t *p_output)
{
    float pv_dist_value = 0;//PV尝试扰动值
    float bat_chg_margin, bat_dsg_margin;
    float bat_power_error = p_input->power_bat2bus_get - p_output->power_bat2bus_set;//电池实际功率和预设误差
    float power_bat2bus_set_corr;//修正后的电池功率
    float power_loss;//设备损耗功率

    p_output->power_act_grid2pcs_set = 0;

    if(p_input->work_mode != EN_OIL_MODE)//离网非油机模式
    {
#if ENABLE_DEV_LOSS_POWER == 1
        power_loss = (p_input->power_pv2bus_get + p_input->power_ev2bus_get) + p_input->power_bat2bus_get - p_input->power_act_eps_get;
#else
        power_loss = 0;
#endif
        p_output->power_bat2bus_set = p_input->power_act_eps_get + power_loss - (p_input->power_pv2bus_get + p_input->power_ev2bus_get);
        _BatPowerLimitConvert(p_input, p_limit, p_output);//结合电池状况，对电池功率进行限制
        power_bat2bus_set_corr = _BatPowerCorr(p_output->power_bat2bus_set, bat_power_error);
        //power_bat2bus_set_corr = _BatPowerCorr2(p_output->power_bat2bus_set, p_input->power_bat2bus_get, p_limit);

        p_output->power_pv2bus_set = p_input->power_pv2bus_get - ((p_input->power_pv2bus_get + p_input->power_ev2bus_get) + power_bat2bus_set_corr - p_input->power_act_eps_get - power_loss);
        p_output->power_ev2bus_set = p_input->power_ev2bus_get;
        p_output->power_act_gen2pcs_set = 0;
    }
    else//离网油机模式
    {
        float correct_gen2pcs_power;//结合限制，应当的油机口功率
#if ENABLE_DEV_LOSS_POWER == 1
        power_loss = (p_input->power_pv2bus_get + p_input->power_ev2bus_get) + p_input->power_bat2bus_get + p_input->power_act_gen2pcs_get - p_input->power_act_eps_get;
#else
        power_loss = 0;
#endif
        p_output->power_bat2bus_set = 0-p_input->power_max_bat_chg_get;
        _BatPowerLimitConvert(p_input, p_limit, p_output);//结合电池状况，对电池功率进行限制
        power_bat2bus_set_corr = _BatPowerCorr(p_output->power_bat2bus_set, bat_power_error);
        //power_bat2bus_set_corr = _BatPowerCorr2(p_output->power_bat2bus_set, p_input->power_bat2bus_get, p_limit);
        bat_chg_margin = p_output->power_bat2bus_set - p_limit->power_bat2bus_min_limit;
        bat_dsg_margin = p_limit->power_bat2bus_max_limit - p_output->power_bat2bus_set;

        p_output->power_act_gen2pcs_set = p_input->power_act_eps_get + power_loss - (p_input->power_pv2bus_get + p_input->power_ev2bus_get) - power_bat2bus_set_corr;
        correct_gen2pcs_power = p_output->power_act_gen2pcs_set;
        correct_gen2pcs_power = POWER_VALUE_LIMIT(correct_gen2pcs_power, p_limit->power_gen2pcs_min_limit, p_limit->power_gen2pcs_max_limit);

        //油机获取功率低于油机下限(p_output->power_act_gen2pcs_set < correct_gen2pcs_power)，则降光伏，油机获取功率高于油机上限(p_output->power_act_gen2pcs_set > correct_gen2pcs_power)，则尝试增加光伏
        if(correct_gen2pcs_power - p_output->power_act_gen2pcs_set > 0)//能量富余
        {
            float gen2pcs_diff = correct_gen2pcs_power - p_output->power_act_gen2pcs_set;
            float bat_absorb_power = POWER_GET_MIN(bat_chg_margin, gen2pcs_diff);//电池吸收部分

            p_output->power_act_gen2pcs_set += bat_absorb_power;
            p_output->power_bat2bus_set -= bat_absorb_power;
            if(gen2pcs_diff > bat_chg_margin)//电池不足以吸收,尝试降光伏
            {
                float pv_reduce_power = POWER_GET_MIN(gen2pcs_diff - bat_chg_margin, p_input->power_pv2bus_get);

                p_output->power_pv2bus_set = p_input->power_pv2bus_get - pv_reduce_power;
                p_output->power_act_gen2pcs_set += pv_reduce_power;
            }
        }
        //p_output->power_pv2bus_set = p_input->power_pv2bus_get - (correct_gen2pcs_power - p_output->power_act_gen2pcs_set);
        //p_output->power_ev2bus_set = p_input->power_ev2bus_get;
        else if(correct_gen2pcs_power - p_output->power_act_gen2pcs_set < 0)//能量不足
        {
            float gen2pcs_diff = p_output->power_act_gen2pcs_set - correct_gen2pcs_power;
            float bat_comp_power = POWER_GET_MIN(bat_dsg_margin, gen2pcs_diff);//电池补偿部分

            p_output->power_act_gen2pcs_set -= bat_comp_power;
            p_output->power_bat2bus_set += bat_comp_power;
            if(gen2pcs_diff > bat_dsg_margin)//电池不足以弥补，尝试增加光伏(不在此处增加，通过扰动缓慢增加)
            {
                //p_output->power_pv2bus_set = p_input->power_pv2bus_get + (p_output->power_act_gen2pcs_set - correct_gen2pcs_power - bat_dsg_margin);
            }
        }
        else
        {
            p_output->power_pv2bus_set = p_input->power_pv2bus_get;
        }
        p_output->power_ev2bus_set = p_input->power_ev2bus_get;
        //油机不允许瞬间过冲，需要直接按照限制配置
        //p_output->power_bat2bus_set += (p_output->power_act_gen2pcs_set - correct_gen2pcs_power);
        //p_output->power_act_gen2pcs_set = correct_gen2pcs_power;
    }

    if(p_output->power_pv2bus_set == p_input->power_pv2bus_get)//当期望的光伏等于当前实际值时，可尝试增加光伏
    {
        pv_dist_value = 100;
    }
    p_output->power_pv2bus_set += pv_dist_value;

    /*防止因光伏太大导致设备逆变过载*/
    if((p_output->power_pv2bus_set + p_output->power_ev2bus_set) + p_output->power_bat2bus_set > PCS_MAX_POWER + pv_dist_value + power_loss)
    {
        p_output->power_pv2bus_set = PCS_MAX_POWER - p_output->power_bat2bus_set - p_output->power_ev2bus_set;
    }
    /*光伏不能为负值*/
    if(p_output->power_pv2bus_set < 0)
    {
        p_output->power_pv2bus_set = 0;
    }
    /*光伏不能超过阈值*/
    if(p_output->power_pv2bus_set > PV_MAX_POWER)
    {
        p_output->power_pv2bus_set = PV_MAX_POWER;
    }
}


/**
* @brief: 离网切并网模式下功率调度计算
* @param: p_output:能量调度计算后各模块的功率信息指针
* @return: none
* @note:  
*/
static void _OffToOnEnergyCtrlCalc(EnergyCtrlOutput_t *p_output)
{
    //暂不做处理
}


/**
* @brief: 并网切离网模式下功率调度计算
* @param: p_output:能量调度计算后各模块的功率信息指针
* @return: none
* @note:  
*/
static void _OnToOffEnergyCtrlCalc(EnergyCtrlOutput_t *p_output)
{
    p_output->power_act_grid2pcs_set = 0;
    p_output->power_react_grid2pcs_set = 0;
    p_output->power_appar_gen2pcs_set = 0;
    p_output->power_act_gen2pcs_set = 0;
    p_output->power_react_gen2pcs_set = 0;
    p_output->power_appar_gen2pcs_set = 0;
    p_output->power_pv2bus_set = 0;
    p_output->power_ev2bus_set = 0;
}


/**
* @brief: 功率实时控制大轮询
* @param: none
* @return: none
* @note:  
*/
void EnergyCtrlProcess()
{
    //判断逆变是否完成启动，暂时屏蔽
    // bool inv_enable = InvSwitchStateGet();
    if(1)//(inv_enable == true)
    {
        _GetEnergyInput(&gEnergyCtrlInput);

        switch(gEnergyCtrlInput.inv_stat)
        {
            case EN_STAND_BY_MODE:
            {
                _OnToOffEnergyCtrlCalc(&gEnergyCtrlOutput);
                break;
            }
            case EN_OFF_GRID_MODE:
            {
                _OnOffGridLimitCalc(0, &gEnergyCtrlInput, &gEnergyCtrlLimit);
                _OffGridEnergyCtrlCalc(&gEnergyCtrlInput, &gEnergyCtrlLimit, &gEnergyCtrlOutput);
                break;
            }
            case EN_OFF_GRID_TO_GRID_CONNECT_MODE:
            {
                _OffToOnEnergyCtrlCalc(&gEnergyCtrlOutput);
                break;
            }
            case EN_GRID_CONNECT_MODE:
            {
                _OnOffGridLimitCalc(1, &gEnergyCtrlInput, &gEnergyCtrlLimit);
                _OnGridEnergyCtrlCalc(&gEnergyCtrlInput, &gEnergyCtrlLimit, &gEnergyCtrlOutput);
                break;
            }
            case EN_GRID_CONNECT_TO_OFF_GRID_MODE:
            {
                _OnToOffEnergyCtrlCalc(&gEnergyCtrlOutput);
                break;
            }
            default:break;
        }
    }
    else
    {
        _OnToOffEnergyCtrlCalc(&gEnergyCtrlOutput);
    }

    // MpptSetPvPwr(gEnergyCtrlOutput.power_pv2bus_set);
    // if(gEnergyCtrlInput.work_mode == EN_OIL_MODE)//油机模式
    // {
        // SetPwrRef(gEnergyCtrlOutput.power_act_gen2pcs_set, gEnergyCtrlOutput.power_react_gen2pcs_set);
        // InvWorkPermit_Monitor(inv_enable, ?, 1);
    // }
    // else//非油机模式
    // {
        // SetPwrRef(gEnergyCtrlOutput.power_act_grid2pcs_set, gEnergyCtrlOutput.power_react_grid2pcs_set);
        // InvWorkPermit_Monitor(inv_enable, ?, 0);
    // }
}


/**
* @brief: 并网认证测试相关功率调度代码，常态运行不使用
* @param: none
* @return: none
* @note:  
*/
// int16_t flag_netdisabletest_trigger  = 0; //模拟断网触发标志位
// int16_t flag_netdisabletest_recover  = 0; //模拟断网恢复标志位
// int16_t flag_hardexportlimit_trigger = 0; //输出功率硬限制触发标志位
// int16_t flag_hardexportlimit_recover = 0; //输出功率硬限制恢复标志位
// int16_t flag_softexportlimit_trigger = 0; //输出功率软限制触发标志位
// int16_t flag_powerratelimit_trigger  = 0; //功率变化率限制触发标志位
// int16_t state_netdisabletest_new  = 0; //模拟断网开关当前状态
// int16_t state_netdisabletest_last = 0; //模拟断网开关历史状态
// int16_t state_hardexportlimit_trigger_new  = 0; //输出功率硬限制触发标志位当前状态
// int16_t state_hardexportlimit_trigger_last = 0; //输出功率硬限制触发标志位历史状态
// int32_t cnt_overhardexportlimit = 0; //超过输出功率硬限制运行范围计数器
// int32_t cnt_oversoftexportlimit = 0; //超过输出功率软限制运行范围计数器
// int32_t cnt_hardexportlimit_trigger = 0; //输出功率硬限制触发计数器
// int16_t flag_offgrid_trigger   = 0; //主动脱网触发标志位
// int16_t flag_offgrid_completed = 0; //主动脱网完成标志位
void CertificationTest(EnergyCtrlInput_t *p_input, EnergyCtrlOutput_t *p_output)
{
    static int16_t flag_netdisabletest_trigger  = 0; //模拟断网触发标志位
    static int16_t flag_netdisabletest_recover  = 0; //模拟断网恢复标志位
    static int16_t flag_hardexportlimit_trigger = 0; //输出功率硬限制触发标志位
    static int16_t flag_hardexportlimit_recover = 0; //输出功率硬限制恢复标志位
    static int16_t flag_softexportlimit_trigger = 0; //输出功率软限制触发标志位
    static int16_t flag_powerratelimit_trigger  = 0; //功率变化率限制触发标志位
    static int16_t state_netdisabletest_new  = 0; //模拟断网开关当前状态
    static int16_t state_netdisabletest_last = 0; //模拟断网开关历史状态
    static int16_t state_hardexportlimit_trigger_new  = 0; //输出功率硬限制触发标志位当前状态
    static int16_t state_hardexportlimit_trigger_last = 0; //输出功率硬限制触发标志位历史状态
    static int32_t cnt_overhardexportlimit = 0; //超过输出功率硬限制运行范围计数器
    static int32_t cnt_oversoftexportlimit = 0; //超过输出功率软限制运行范围计数器
    static int32_t cnt_hardexportlimit_trigger = 0; //输出功率硬限制触发计数器
    static int16_t flag_offgrid_trigger   = 0; //主动脱网触发标志位
    static int16_t flag_offgrid_completed = 0; //主动脱网完成标志位
    // static int16_t flag_netdisabletest_trigger  = 0; //模拟断网触发标志位
    // static int16_t flag_netdisabletest_recover  = 0; //模拟断网恢复标志位
    // static int16_t flag_hardexportlimit_trigger = 0; //输出功率硬限制触发标志位
    // static int16_t flag_softexportlimit_trigger = 0; //输出功率软限制触发标志位
    // static int16_t flag_powerratelimit_trigger  = 0; //功率变化率限制触发标志位
    // static int16_t state_netdisabletest_new  = 0; //模拟断网开关当前状态
    // static int16_t state_netdisabletest_last = 0; //模拟断网开关历史状态
    // static int32_t cnt_overhardexportlimit = 0; //超过输出功率硬限制运行范围计数器
    // static int32_t cnt_oversoftexportlimit = 0; //超过输出功率软限制运行范围计数器
    // static int16_t flag_offgrid_trigger   = 0; //主动脱网触发标志位
    // static int16_t flag_offgrid_completed = 0; //主动脱网完成标志位

    static float powerchangestep = 4.44f; // 最大视在功率Sn = 16000W，6分钟变化100% Sn，每100ms变化一次，功率变化率步长 = 16000 / (6*60/0.1) = 4.44

    state_netdisabletest_new = p_input->flag_netdisabletest_enable;
    if(state_netdisabletest_new == 1 && state_netdisabletest_last == 0) // 20260325，新增模拟断网触发判断，上升沿触发
    {
        flag_netdisabletest_trigger = 1;
    }
    if(state_netdisabletest_new == 0 && state_netdisabletest_last == 1) // 20260325，新增模拟断网恢复判断，下降沿恢复
    {
        flag_netdisabletest_recover = 1;
    }
    state_netdisabletest_last = state_netdisabletest_new;

    if(p_input->flag_hardexportlimit_enable == 1 && p_input->flag_softexportlimit_enable == 1) // 20260325，新增硬限制开关和软限制开关同时使能，特殊处理，CT功率过大或模拟断网触发脱网保护
    {
        state_hardexportlimit_trigger_new = flag_hardexportlimit_trigger;
        if(state_hardexportlimit_trigger_new == 0 && state_hardexportlimit_trigger_last == 1) // 20260325，新增输出功率硬限制触发后恢复判断，下降沿恢复
        {
            flag_hardexportlimit_recover = 1;
        }
        state_hardexportlimit_trigger_last = state_hardexportlimit_trigger_new;

        if((-(p_input->power_act_ct2grid_get) - 100) >= p_input->power_max_ongrid_get)
        {
            cnt_overhardexportlimit++;
            if(cnt_overhardexportlimit >= 10 && cnt_overhardexportlimit < 30)
            {
                p_output->power_bat2bus_target = p_input->power_act_eps_get;
                flag_hardexportlimit_trigger = 0;
            }
            else if(cnt_overhardexportlimit >= 30)
            {
                cnt_overhardexportlimit = 30;
                p_output->power_bat2bus_target = p_input->power_act_eps_get;
                flag_hardexportlimit_trigger = 1;
            }
        }
        else
        {
            cnt_overhardexportlimit--;
            if(cnt_overhardexportlimit <= 1)
            {
                cnt_overhardexportlimit = 1;
                flag_hardexportlimit_trigger = 0;
            }
        }


        if(flag_hardexportlimit_trigger == 1 || flag_netdisabletest_trigger == 1)
        {
            p_output->power_bat2bus_target = p_input->power_act_eps_get;
            flag_offgrid_trigger = 1; // 20260326，新增逆变器主动脱网触发标志位，需要PCS实际代码触发逆变器脱网，仿真平台无法测试
        }
        if(flag_hardexportlimit_trigger == 1 || flag_netdisabletest_recover == 1)
        {
            cnt_hardexportlimit_trigger++;
            if(cnt_hardexportlimit_trigger >= 10 && cnt_hardexportlimit_trigger < 30)
            {
                flag_powerratelimit_trigger = 1;
            }
            if(cnt_hardexportlimit_trigger >= 30)
            {
                cnt_hardexportlimit_trigger = 30;
            }
        }

        if(flag_netdisabletest_trigger == 1)
        {
            if(flag_netdisabletest_recover == 1)
            {
                flag_powerratelimit_trigger = 1;
                if((p_output->power_bat2bus_set + 10) >= p_output->power_bat2bus_target && (p_output->power_bat2bus_set - 10) <= p_output->power_bat2bus_target)
                {
                    cnt_oversoftexportlimit++;
                    if(cnt_oversoftexportlimit >= 30)
                    {
                        cnt_oversoftexportlimit = 30;
                        flag_offgrid_trigger = 0;
                        flag_offgrid_completed = 0;
                        flag_netdisabletest_trigger = 0;
                        flag_netdisabletest_recover = 0;
                        flag_hardexportlimit_trigger = 0;
                        flag_hardexportlimit_recover = 0;
                    }
                }
                else
                {
                    cnt_oversoftexportlimit--;
                    if(cnt_oversoftexportlimit <= 1)
                    {
                        cnt_oversoftexportlimit = 1;
                    }
                }
            }
            else
            {
                flag_powerratelimit_trigger = 0;
                cnt_oversoftexportlimit--;
                if(cnt_oversoftexportlimit <= 1)
                {
                    cnt_oversoftexportlimit = 1;
                }
            }
        }
        else
        {
            if(cnt_hardexportlimit_trigger == 30 && flag_hardexportlimit_recover == 1) // 20260326，新增逆变器主动脱网完成标志位，需要PCS实际代码在逆变器脱网完成后置1，仿真平台无法测试
            {
                flag_powerratelimit_trigger = 1;
                if((p_output->power_bat2bus_set + 10) >= p_output->power_bat2bus_target && (p_output->power_bat2bus_set - 10) <= p_output->power_bat2bus_target)
                {
                    cnt_oversoftexportlimit++;
                    if(cnt_oversoftexportlimit >= 30)
                    {
                        cnt_hardexportlimit_trigger = 0;
                        cnt_oversoftexportlimit = 30;
                        flag_offgrid_trigger = 0;
                        flag_offgrid_completed = 0;
                        flag_hardexportlimit_trigger = 0;
                        flag_hardexportlimit_recover = 0;
                        flag_powerratelimit_trigger = 0;
                    }
                }
                else
                {
                    cnt_oversoftexportlimit--;
                    if(cnt_oversoftexportlimit <= 1)
                    {
                        cnt_oversoftexportlimit = 1;
                    }
                }
            }
            else
            {
                cnt_oversoftexportlimit--;
                if(cnt_oversoftexportlimit <= 1)
                {
                    cnt_oversoftexportlimit = 1;
                }
            }
        }
    }

    else if(p_input->flag_hardexportlimit_enable == 1 && p_input->flag_softexportlimit_enable == 0) // 20260325，新增硬限制开关单独使能，进入输出功率硬限制判断和保护逻辑
    {
        state_hardexportlimit_trigger_new = flag_hardexportlimit_trigger;
        if(state_hardexportlimit_trigger_new == 0 && state_hardexportlimit_trigger_last == 1) // 20260325，新增输出功率硬限制触发后恢复判断，下降沿恢复
        {
            flag_hardexportlimit_recover = 1;
        }
        state_hardexportlimit_trigger_last = state_hardexportlimit_trigger_new;

        if((p_input->power_act_eps_get + p_input->power_act_norload_get + p_input->power_max_ongrid_get + 100) <= 16000)
        {
            cnt_overhardexportlimit++;
            if(cnt_overhardexportlimit >= 10 && cnt_overhardexportlimit < 30)
            {
                p_output->power_bat2bus_target = p_input->power_act_eps_get;
                flag_hardexportlimit_trigger = 0;
            }
            else if(cnt_overhardexportlimit >= 30)
            {
                cnt_overhardexportlimit = 30;
                p_output->power_bat2bus_target = p_input->power_act_eps_get;
                flag_hardexportlimit_trigger = 1;
            }
        }
        else
        {
            cnt_overhardexportlimit--;
            if(cnt_overhardexportlimit <= 1)
            {
                cnt_overhardexportlimit = 1;
                flag_hardexportlimit_trigger = 0;
            }
        }
        if(flag_hardexportlimit_trigger == 1 || flag_netdisabletest_trigger == 1)
        {
            p_output->power_bat2bus_target = p_input->power_act_eps_get;
            flag_offgrid_trigger = 1; // 20260326，新增逆变器主动脱网触发标志位，需要PCS实际代码触发逆变器脱网，仿真平台无法测试
        }
        if(flag_hardexportlimit_trigger == 1 || flag_netdisabletest_recover == 1)
        {
            cnt_hardexportlimit_trigger++;
            if(cnt_hardexportlimit_trigger >= 10 && cnt_hardexportlimit_trigger < 30)
            {
                flag_powerratelimit_trigger = 1;
            }
            if(cnt_hardexportlimit_trigger >= 30)
            {
                cnt_hardexportlimit_trigger = 30;
            }
        }

        if(flag_netdisabletest_trigger == 1)
        {
            if(flag_netdisabletest_recover == 1)
            {
                flag_powerratelimit_trigger = 1;
                if((p_output->power_bat2bus_set + 10) >= p_output->power_bat2bus_target && (p_output->power_bat2bus_set - 10) <= p_output->power_bat2bus_target)
                {
                    cnt_oversoftexportlimit++;
                    if(cnt_oversoftexportlimit >= 30)
                    {
                        cnt_oversoftexportlimit = 30;
                        flag_offgrid_trigger = 0;
                        flag_offgrid_completed = 0;
                        flag_netdisabletest_trigger = 0;
                        flag_netdisabletest_recover = 0;
                        flag_hardexportlimit_trigger = 0;
                        flag_hardexportlimit_recover = 0;
                    }
                }
                else
                {
                    cnt_oversoftexportlimit--;
                    if(cnt_oversoftexportlimit <= 1)
                    {
                        cnt_oversoftexportlimit = 1;
                    }
                }
            }
            else
            {
                flag_powerratelimit_trigger = 0;
                cnt_oversoftexportlimit--;
                if(cnt_oversoftexportlimit <= 1)
                {
                    cnt_oversoftexportlimit = 1;
                }
            }
        }
        else
        {
            if(cnt_hardexportlimit_trigger == 30 && flag_hardexportlimit_recover == 1) // 20260326，新增逆变器主动脱网完成标志位，需要PCS实际代码在逆变器脱网完成后置1，仿真平台无法测试
            {
                flag_powerratelimit_trigger = 1;
                if((p_output->power_bat2bus_set + 10) >= p_output->power_bat2bus_target && (p_output->power_bat2bus_set - 10) <= p_output->power_bat2bus_target)
                {
                    cnt_oversoftexportlimit++;
                    if(cnt_oversoftexportlimit >= 30)
                    {
                        cnt_hardexportlimit_trigger = 0;
                        cnt_oversoftexportlimit = 30;
                        flag_offgrid_trigger = 0;
                        flag_offgrid_completed = 0;
                        flag_hardexportlimit_trigger = 0;
                        flag_hardexportlimit_recover = 0;
                        flag_powerratelimit_trigger = 0;
                    }
                }
                else
                {
                    cnt_oversoftexportlimit--;
                    if(cnt_oversoftexportlimit <= 1)
                    {
                        cnt_oversoftexportlimit = 1;
                    }
                }
            }
            else
            {
                cnt_oversoftexportlimit--;
                if(cnt_oversoftexportlimit <= 1)
                {
                    cnt_oversoftexportlimit = 1;
                }
            }
        }

    }

    else if(p_input->flag_hardexportlimit_enable == 0 && p_input->flag_softexportlimit_enable == 1) // 20260325，新增软限制开关单独使能，进入输出功率软限制判断和保护逻辑
    {
        if(flag_netdisabletest_recover == 1)
        {
            flag_powerratelimit_trigger = 1;
            if((p_output->power_bat2bus_set + 10) >= p_output->power_bat2bus_target && (p_output->power_bat2bus_set - 10) <= p_output->power_bat2bus_target)
            {
                cnt_oversoftexportlimit++;
                if(cnt_oversoftexportlimit >= 30)
                {
                    cnt_oversoftexportlimit = 30;
                    flag_netdisabletest_trigger = 0;
                    flag_netdisabletest_recover = 0;
                }
            }
            else
            {
                cnt_oversoftexportlimit--;
                if(cnt_oversoftexportlimit <= 1)
                {
                    cnt_oversoftexportlimit = 1;
                }
            }
        }
        else
        {
            flag_powerratelimit_trigger = 0;
            cnt_oversoftexportlimit--;
            if(cnt_oversoftexportlimit <= 1)
            {
                cnt_oversoftexportlimit = 1;
            }
        }
    }

    else if(p_input->flag_powerratelimit_enable  == 1) // 20260325，新增功率变化率限制开关使能，强制开启功率变化率限制，功率调度正常运行
    {
        flag_powerratelimit_trigger = 1;
    }

    else // 20260325，新增无并网认证开关使能，不进行处理，功率调度正常运行
    {
        flag_powerratelimit_trigger = 0;
    }

    if(flag_powerratelimit_trigger  == 1)
    {
        if(p_output->power_bat2bus_set + 10 <= p_output->power_bat2bus_target)
        {
            p_output->power_bat2bus_set = p_output->power_bat2bus_set + powerchangestep;
        }
        else if(p_output->power_bat2bus_set - 10 >= p_output->power_bat2bus_target)
        {
            p_output->power_bat2bus_set = p_output->power_bat2bus_set - powerchangestep;
        }
        else
        {
            p_output->power_bat2bus_set = p_output->power_bat2bus_target;
        }
    }
    else
    {
        p_output->power_bat2bus_set = p_output->power_bat2bus_target;
    }

}
