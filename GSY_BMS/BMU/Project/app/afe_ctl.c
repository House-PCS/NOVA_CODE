/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        afe_ctl.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-8-20
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-8-20    <td>V1.0.0    <td>          <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "afe_ctl.h"
#include "ntc_convert.h"

static EN_AFE_CTL_FUNC _gAFECtlStat = EN_AFE_CTL_IDLE;

AFESampleInfo_t gAFESampleInfo;

#define AFE_TEMP_V_MAX   3000
#define AFE_TEMP_R_UP   10000

#define AFE_BTEMP_V_MAX  3000
#define AFE_BTEMP_R_UP  15000

#define AFE_BALANCE_V_DIFF  100

const static uint16_t _gCellCHMask[AFE_NUM] = {0x7F, 0x7F};


/** 
 * @brief    本地均衡判断阈值计算(获取最低电芯电压)
 * @param    mask:均衡通道掩码指针，如果存在未使用的通道，则不能开启均衡，对于32串工商业，则mask指向两个0x7F的数组(最后一个通道未使用)
 * @return   最小值
 * @note:    
 */
static uint16_t _BalanceCtlCalc(const uint16_t *p_mask)
{
    uint16_t min_voltage = 0xFFFF;

    /*获取电压最小值*/
    for(uint8_t i = 0; i < AFE_NUM; i++)
    {
        for(uint8_t j = 0; j < AFE_CELL_NUM; j++)
        {
            if(((p_mask[i] >> j) & 0x01) == 1)
            {
                if(gAFESampleInfo.cell_voltage[i][j] < min_voltage)
                {
                    min_voltage = gAFESampleInfo.cell_voltage[i][j];
                }
            }
        }
    }
    return min_voltage;
}




/** 
 * @brief    afe获取最大电压
 * @param    NONE
 * @return   操作结果
 * @note:    
 */
static EN_AFE_OP_RES _AFEVoltageSampleProcess()
{
    static uint32_t time_count = 0;
    static EN_AFE_V_STA stat = EN_AFE_V_STA_CMD;
    
    switch(stat)
    {
        case EN_AFE_V_STA_CMD:
        {
            DevAFEVoltageSampleCmd();
            stat = EN_AFE_V_STA_WAIT;
            time_count = tim_get_systime();
            return EN_AFE_OP_RES_ING;
            break;
        }
        case EN_AFE_V_STA_WAIT:
        {
            EN_AFE_COMM_STATUS res = DevAFEVoltageSampleStat(&gAFESampleInfo.cell_voltage[0][0]);
            if(res == EN_AFE_COMM_STATUS_COMPLETE)
            {
                stat = EN_AFE_V_STA_CMD;
                return EN_AFE_OP_RES_COMPLETE;
            }
            else if(res == EN_AFE_COMM_STATUS_ERROR)
            {
                stat = EN_AFE_V_STA_CMD;
                return EN_AFE_OP_RES_ERROR;
            }
            else
            {
                if((uint32_t)(tim_get_systime() - time_count) > 10)//暂定10ms作为超时时间
                {
                    stat = EN_AFE_V_STA_CMD;
                    return EN_AFE_OP_RES_ERROR;
                }
            }
            break;
        }
        default:break;
    }
    return EN_AFE_OP_RES_ING;
}


/** 
 * @brief    afe温度采集轮询处理
 * @param    NONE
 * @return   操作结果
 * @note:    
 */
static EN_AFE_OP_RES _AFETempSampleProcess()
{
    static uint32_t time_count = 0;
    static uint8_t sample_cnt = 0;//采样计数，温度采样需要分8次(两个8选1同时进行),该参数也可放在形参中，每次只执行一组
    static EN_AFE_T_STA stat = EN_AFE_T_STA_CH_SEL;
    
    switch(stat)
    {
        case EN_AFE_T_STA_CH_SEL:
        {
			if((uint32_t)(tim_get_systime() - time_count) > 1)
			{
				//工商业两个AFE GPIO使用情况一样
				DevAFEGpioIOValueCfg(0, EN_AFE_GPIO_1, (sample_cnt >> 2) & 0x01);
				DevAFEGpioIOValueCfg(0, EN_AFE_GPIO_2, (sample_cnt >> 1) & 0x01);
				DevAFEGpioIOValueCfg(0, EN_AFE_GPIO_3, (sample_cnt >> 0) & 0x01);
				DevAFEGpioIOValueCfg(1, EN_AFE_GPIO_1, (sample_cnt >> 2) & 0x01);
				DevAFEGpioIOValueCfg(1, EN_AFE_GPIO_2, (sample_cnt >> 1) & 0x01);
				DevAFEGpioIOValueCfg(1, EN_AFE_GPIO_3, (sample_cnt >> 0) & 0x01);
				if(DevAFEGpioIOCtl() == true)
				{
					stat = EN_AFE_T_STA_CMD_1;
					time_count = tim_get_systime();
					return EN_AFE_OP_RES_ING;
				}
				else
				{
					sample_cnt = 0;
					stat = EN_AFE_T_STA_CH_SEL;
					return EN_AFE_OP_RES_ERROR;
				} 
			}
            break;
        }
        case EN_AFE_T_STA_CMD_1:
        {
            if((uint32_t)(tim_get_systime() - time_count) > 2)//稍微加点延时，确保8选1切换完成
            {
                DevAFEAuxVoltageSampleCmd(EN_AFE_GPIO_6);
                time_count = tim_get_systime();
                stat = EN_AFE_T_STA_WAIT_1;
            }
            return EN_AFE_OP_RES_ING;
        }
        case EN_AFE_T_STA_WAIT_1:
        {
            uint16_t volt_value[AFE_NUM];
            EN_AFE_COMM_STATUS res = DevAFEAuxVoltageSampleStat(volt_value, EN_AFE_GPIO_6);
            if(res == EN_AFE_COMM_STATUS_COMPLETE)
            {
                //volt_value转换成温度值赋给gAFESampleInfo.cell_temp
                for(uint8_t i = 0; i < AFE_NUM; i++)
                {
                    gAFESampleInfo.cell_temp[i][sample_cnt] = TemperatureGet(volt_value[i], AFE_TEMP_V_MAX, AFE_TEMP_R_UP, EN_NTC_B_3435, EN_NTC_T25_10K);
                    if(volt_value[i] > AFE_TEMP_V_MAX - 200)//认为NTC接触不良
                    {
                        gAFESampleInfo.temp_ow_stat[i] |= (1 << sample_cnt);
                    }
                    else
                    {
                        gAFESampleInfo.temp_ow_stat[i] &= ~(1 << sample_cnt);
                    }
                }
                time_count = tim_get_systime();
                stat = EN_AFE_T_STA_CMD_2;
                return EN_AFE_OP_RES_ING;
            }
            else if(res == EN_AFE_COMM_STATUS_ERROR)
            {
                sample_cnt = 0;
                stat = EN_AFE_T_STA_CH_SEL;
                return EN_AFE_OP_RES_ERROR;
            }
            else
            {
                if((uint32_t)(tim_get_systime() - time_count) > 10)//暂定10ms作为超时时间
                {
                    sample_cnt = 0;
                    stat = EN_AFE_T_STA_CH_SEL;
                    return EN_AFE_OP_RES_ERROR;
                }
                return EN_AFE_OP_RES_ING;
            }
        }
        case EN_AFE_T_STA_CMD_2:
        {
            if(sample_cnt >= 5)
            {
                stat = EN_AFE_T_STA_CH_SEL;
                sample_cnt++;
                if(sample_cnt >= 8)
                {
                    sample_cnt = 0;
                    return EN_AFE_OP_RES_COMPLETE;
                }
                else
                {
                    return EN_AFE_OP_RES_ING;
                }
            }
            DevAFEAuxVoltageSampleCmd(EN_AFE_GPIO_7);
            time_count = tim_get_systime();
            stat = EN_AFE_T_STA_WAIT_2;
            return EN_AFE_OP_RES_ING;
        }
        case EN_AFE_T_STA_WAIT_2:
        {
            uint16_t volt_value[AFE_NUM];
            EN_AFE_COMM_STATUS res = DevAFEAuxVoltageSampleStat(volt_value, EN_AFE_GPIO_7);
            if(res == EN_AFE_COMM_STATUS_COMPLETE)
            {
                //volt_value转换成温度值赋给gAFESampleInfo.cell_temp
                for(uint8_t i = 0; i < AFE_NUM; i++)
                {
                    gAFESampleInfo.cell_temp[i][sample_cnt + 8] = TemperatureGet(volt_value[i], AFE_TEMP_V_MAX, AFE_TEMP_R_UP, EN_NTC_B_3435, EN_NTC_T25_10K);
//					TemperatureGet(volt_value[i], AFE_TEMP_V_MAX, AFE_TEMP_R_UP, EN_NTC_B_3435, EN_NTC_T25_10K);
                    if(volt_value[i] > AFE_TEMP_V_MAX - 200)//认为NTC接触不良
                    {
                        gAFESampleInfo.temp_ow_stat[i] |= (1 << (sample_cnt + 8));
                    }
                    else
                    {
                        gAFESampleInfo.temp_ow_stat[i] &= ~(1 << (sample_cnt + 8));
                    }
                }
                time_count = tim_get_systime();
                stat = EN_AFE_T_STA_CH_SEL;
                sample_cnt++;//如果采用每次只采样一组，则直接返回EN_AFE_OP_RES_COMPLETE
                if(sample_cnt >= 8)
                {
                    sample_cnt = 0;
                    return EN_AFE_OP_RES_COMPLETE;
                }
                else
                {
                    return EN_AFE_OP_RES_ING;
                }
            }
            else if(res == EN_AFE_COMM_STATUS_ERROR)
            {
                sample_cnt = 0;
                stat = EN_AFE_T_STA_CH_SEL;
                return EN_AFE_OP_RES_ERROR;
            }
            else
            {
                if((uint32_t)(tim_get_systime() - time_count) > 10)//暂定10ms作为超时时间
                {
                    sample_cnt = 0;
                    stat = EN_AFE_T_STA_CH_SEL;
                    return EN_AFE_OP_RES_ERROR;
                }
                return EN_AFE_OP_RES_ING;
            }
        }
        default:break;
    }
    return EN_AFE_OP_RES_ING;
}


/** 
 * @brief    afe均衡温度轮询处理
 * @param    NONE
 * @return   操作结果
 * @note:    
 */
static EN_AFE_OP_RES _AFEBalanceTempSampleProcess()
{
    static uint32_t time_count = 0;
    static EN_AFE_BT_STA stat = EN_AFE_BT_STA_CMD_1;
    
    switch(stat)
    {
        case EN_AFE_BT_STA_CMD_1:
        {
            DevAFEAuxVoltageSampleCmd(EN_AFE_GPIO_9);
            time_count = tim_get_systime();
            stat = EN_AFE_BT_STA_WAIT_1;
            return EN_AFE_OP_RES_ING;
            break;
        }
        case EN_AFE_BT_STA_WAIT_1:
        {
            uint16_t volt_value[AFE_NUM];
            EN_AFE_COMM_STATUS res = DevAFEAuxVoltageSampleStat(volt_value, EN_AFE_GPIO_9);
            if(res == EN_AFE_COMM_STATUS_COMPLETE)
            {
                //volt_value转换成温度值赋给gAFESampleInfo.balance_temp
                for(uint8_t i = 0; i < AFE_NUM; i++)
                {
                    gAFESampleInfo.balance_temp[i][0] = TemperatureGet(volt_value[i], AFE_BTEMP_V_MAX, AFE_BTEMP_R_UP, EN_NTC_B_3450, EN_NTC_T25_10K);
                    if(volt_value[i] > AFE_BTEMP_V_MAX - 200)//认为NTC接触不良
                    {
                        gAFESampleInfo.balance_temp_ow_stat[i] |= (1 << 0);
                    }
                    else
                    {
                        gAFESampleInfo.balance_temp_ow_stat[i] &= ~(1 << 0);
                    }
                }
                time_count = tim_get_systime();
                stat = EN_AFE_BT_STA_CMD_2;
                return EN_AFE_OP_RES_ING;
            }
            else if(res == EN_AFE_COMM_STATUS_ERROR)
            {
                stat = EN_AFE_BT_STA_CMD_1;
                return EN_AFE_OP_RES_ERROR;
            }
            else
            {
                if((uint32_t)(tim_get_systime() - time_count) > 10)//暂定10ms作为超时时间
                {
                    stat = EN_AFE_BT_STA_CMD_1;
                    return EN_AFE_OP_RES_ERROR;
                }
                return EN_AFE_OP_RES_ING;
            }
            break;
        }
        case EN_AFE_BT_STA_CMD_2:
        {
            DevAFEAuxVoltageSampleCmd(EN_AFE_GPIO_8);
            time_count = tim_get_systime();
            stat = EN_AFE_BT_STA_WAIT_2;
            return EN_AFE_OP_RES_ING;
            break;
        } 
        case EN_AFE_BT_STA_WAIT_2:
        {
            uint16_t volt_value[AFE_NUM];
            EN_AFE_COMM_STATUS res = DevAFEAuxVoltageSampleStat(volt_value, EN_AFE_GPIO_8);
            if(res == EN_AFE_COMM_STATUS_COMPLETE)
            {
                //volt_value转换成温度值赋给gAFESampleInfo.balance_temp
                for(uint8_t i = 0; i < AFE_NUM; i++)
                {
                    gAFESampleInfo.balance_temp[i][1] = TemperatureGet(volt_value[i], AFE_BTEMP_V_MAX, AFE_BTEMP_R_UP, EN_NTC_B_3450, EN_NTC_T25_10K);
                    if(volt_value[i] > AFE_BTEMP_V_MAX - 200)//认为NTC接触不良
                    {
                        gAFESampleInfo.balance_temp_ow_stat[i] |= (1 << 1);
                    }
                    else
                    {
                        gAFESampleInfo.balance_temp_ow_stat[i] &= ~(1 << 1);
                    }
                }
                time_count = tim_get_systime();
                stat = EN_AFE_BT_STA_CMD_1;
                return EN_AFE_OP_RES_COMPLETE;
            }
            else if(res == EN_AFE_COMM_STATUS_ERROR)
            {
                stat = EN_AFE_BT_STA_CMD_1;
                return EN_AFE_OP_RES_ERROR;
            }
            else
            {
                if((uint32_t)(tim_get_systime() - time_count) > 10)//暂定10ms作为超时时间
                {
                    stat = EN_AFE_BT_STA_CMD_1;
                    return EN_AFE_OP_RES_ERROR;
                }
                return EN_AFE_OP_RES_ING;
            }
            break;
        }     
        default:break;
    }
    return EN_AFE_OP_RES_ING;
}


/** 
 * @brief    afe均衡轮询处理(分多组，每次只均衡一组)
 * @param    NONE
 * @return   操作结果
 * @note:    
 */
static EN_AFE_OP_RES _AFEBalanceProcess()
{
    static uint32_t time_count = 0;
    static uint8_t  cnt = 0;//用于本地均衡，每个AFE分两次，一次均衡八个通道(最后一次七个通道)
    static EN_AFE_BALANCE_STA stat = EN_AFE_BALANCE_STA_COND_JUDGE;
    
    if(gAFESampleInfo.balance_type == 2)//不均衡
    {
        if(gAFESampleInfo.balance_bcu_open == true)
        {
            if(DevAFEBalanceStop(gAFESampleInfo.balance_stat_flag) == true)
            { 
                gAFESampleInfo.balance_bcu_open = false;
                return EN_AFE_OP_RES_COMPLETE;
            }
            else
            {
                return EN_AFE_OP_RES_ERROR;
            }
        }
        else
        {
            return EN_AFE_OP_RES_COMPLETE;
        }
    }
    if(gAFESampleInfo.balance_open_condition == true)//当前在充电模式下，允许均衡
    {
        if(gAFESampleInfo.balance_type == 1)//由BCU控制均衡
        {
            uint8_t i;
            for(i = 0; i < AFE_NUM; i++)
            {
                if(gAFESampleInfo.balance_bcu_ctl[i] != 0)
                {
                    break;
                }
            }
            if(i == AFE_NUM)//所有通道都未开启
            {
                if(DevAFEBalanceStop(gAFESampleInfo.balance_stat_flag) == true)
                {
                    gAFESampleInfo.balance_bcu_open = false;
                    return EN_AFE_OP_RES_COMPLETE;
                }
                else
                {
                    return EN_AFE_OP_RES_ERROR;
                }
            }
            else
            {
                if(DevAFEBalanceStart(gAFESampleInfo.balance_bcu_ctl, gAFESampleInfo.balance_stat_flag) == true)
                {
                    gAFESampleInfo.balance_bcu_open = true;
                    return EN_AFE_OP_RES_COMPLETE;
                }
                else
                {
                    return EN_AFE_OP_RES_ERROR;//相邻通道不能同时开启
                }
            }
        }
        else if(gAFESampleInfo.balance_type == 0)//本地自行控制均衡
        {
            uint16_t min_voltage = _BalanceCtlCalc(_gCellCHMask);
            uint16_t dcc_cell[AFE_NUM];
            for(uint8_t i = 0; i < AFE_NUM; i++)
            {
                dcc_cell[i] = 0;//先清零
                if(cnt == 0)//均衡偶数通道
                {
                    for(uint8_t j = 0; j < AFE_CELL_NUM; j += 2)
                    {
                        if((gAFESampleInfo.cell_voltage[i][j] - min_voltage > AFE_BALANCE_V_DIFF) && (((_gCellCHMask[i] >> j) & 0x01) == 0x01))
                        {
                            dcc_cell[i] |= (1 << j);
                        }
                    }
                }
                else if(cnt == 1)//均衡奇数通道
                {
                    for(uint8_t j = 1; j < AFE_CELL_NUM; j += 2)
                    {
                        if((gAFESampleInfo.cell_voltage[i][j] - min_voltage > AFE_BALANCE_V_DIFF) && (((_gCellCHMask[i] >> j) & 0x01) == 0x01))
                        {
                            dcc_cell[i] |= (1 << j);
                        }
                    }
                }
                cnt++;
                if(cnt >= 1)
                {
                    cnt = 0;
                }
                if(DevAFEBalanceStart(dcc_cell, gAFESampleInfo.balance_stat_flag) == true)
                {
                    gAFESampleInfo.balance_bcu_open = true;
                    return EN_AFE_OP_RES_COMPLETE;
                }
                else
                {
                    return EN_AFE_OP_RES_ERROR;//相邻通道不能同时开启
                }
            }
        }
    }
    else//非充电模式不允许均衡,强制关闭
    {
        if(DevAFEBalanceStop(gAFESampleInfo.balance_stat_flag) == true)
        {
            gAFESampleInfo.balance_bcu_open = false;
            return EN_AFE_OP_RES_COMPLETE;
        }
        else
        {
            return EN_AFE_OP_RES_ERROR;
        }
    }

    return EN_AFE_OP_RES_ING;
}


/** 
 * @brief    afe断线检测轮询处理
 * @param    NONE
 * @return   操作结果
 * @note:    
 */
static EN_AFE_OP_RES _AFEOpenWireDetectProcess()
{
    static uint32_t time_count = 0;
    static EN_AFE_OW_STA stat = EN_AFE_OW_STA_CMD_EVEN;
    uint8_t type = 0; //0为Cell模式，1为S-Volt模式
    
    switch(stat)
    {
        case EN_AFE_OW_STA_CMD_EVEN: 
        {
            if(DevAFEWireCheckPrep() == true)
            {
                DevAFEWireCheckCmd(0, type);
                stat = EN_AFE_OW_STA_WAIT_EVEN;
                time_count = tim_get_systime();
                return EN_AFE_OP_RES_ING;
            }
            else
            {
                return EN_AFE_OP_RES_ERROR;
            }
            break;
        }
        case EN_AFE_OW_STA_WAIT_EVEN:
        {
            EN_AFE_COMM_STATUS res = DevAFEWireCheckStat(0, type);
            if(res == EN_AFE_COMM_STATUS_COMPLETE)
            {
                stat = EN_AFE_OW_STA_CMD_ODD;
                return EN_AFE_OP_RES_ING;
            }
            else if(res == EN_AFE_COMM_STATUS_ERROR)
            {
                stat = EN_AFE_OW_STA_CMD_EVEN;
                return EN_AFE_OP_RES_ERROR;
            }
            else
            {
                if((uint32_t)(tim_get_systime() - time_count) > 100)//暂定10ms作为超时时间
                {
                    stat = EN_AFE_OW_STA_CMD_EVEN;
                    return EN_AFE_OP_RES_ERROR;
                }
            }
            break;
        }
        case EN_AFE_OW_STA_CMD_ODD:
        {
            DevAFEWireCheckPrep();
            DevAFEWireCheckCmd(1, type);
            stat = EN_AFE_OW_STA_WAIT_ODD;
            time_count = tim_get_systime();
            return EN_AFE_OP_RES_ING;
            break;
        }
        case EN_AFE_OW_STA_WAIT_ODD:
        {
            EN_AFE_COMM_STATUS res = DevAFEWireCheckStat(1, type);
            if(res == EN_AFE_COMM_STATUS_COMPLETE)
            {
                /*将断线检测信息传递给gAFESampleInfo.cell_ow_stat*/
                DevAFEWireCheckResultGet(gAFESampleInfo.cell_ow_stat, type);
                stat = EN_AFE_OW_STA_CMD_EVEN;
                return EN_AFE_OP_RES_COMPLETE;
            }
            else if(res == EN_AFE_COMM_STATUS_ERROR)
            {
                stat = EN_AFE_OW_STA_CMD_EVEN;
                return EN_AFE_OP_RES_ERROR;
            }
            else
            {
                if((uint32_t)(tim_get_systime() - time_count) > 100)//暂定10ms作为超时时间
                {
                    stat = EN_AFE_OW_STA_CMD_EVEN;
                    return EN_AFE_OP_RES_ERROR;
                }
            }
            break;
        }
        default:break;
    }
    return EN_AFE_OP_RES_ING;
}

//调度策略及时长配置，可根据业务情况更改
static AFECtlSche_t _gAFECtlSche[5] = 
{
    {EN_AFE_CTL_V_GET, 30, 0},
    {EN_AFE_CTL_OW_DET, 3000, 0},
    {EN_AFE_CTL_B_SET, 30, 0},
    {EN_AFE_CTL_T_GET, 300, 0},
    {EN_AFE_CTL_BT_GET, 300, 0},
};


/** 
 * @brief    afe操作大轮询
 * @param    NONE
 * @return   NULL
 * @note:    根据业务情况选择放在主循环还是定时器里，如果放在定时器里，函数里面需要增加计数器，根据计数器的值不同，执行不同的操作，
             任何操作完成后，必须先回到IDLE，在IDLE下进行操作调度。
 */
void AFECtlProcess()
{
    static uint32_t sub_timecount[EN_AFE_CTL_NUM];//用于计算各种操作所需时长
    static uint8_t sche_cnt = 0;
    
    switch(_gAFECtlStat)
    {
        case EN_AFE_CTL_IDLE:
        {
            /*AFE操作调度*/
            //当前的策略是哪个操作时间到了就执行哪个，按顺序遍历
            if((uint32_t)(tim_get_systime() - _gAFECtlSche[sche_cnt].inverval_count) >= _gAFECtlSche[sche_cnt].min_interval)
            {
                _gAFECtlStat = _gAFECtlSche[sche_cnt].ctl_type;
                _gAFECtlSche[sche_cnt].inverval_count = tim_get_systime();
            }
            sche_cnt++;
            sche_cnt %= sizeof(_gAFECtlSche)/sizeof(_gAFECtlSche[0]);
            break;
        }
        case EN_AFE_CTL_V_GET:  //电压采样
        {
            /*先关闭均衡*/
            if(gAFESampleInfo.balance_bcu_open == true)
            {
                if(DevAFEBalanceStop(gAFESampleInfo.balance_stat_flag) == true)
                {
                    gAFESampleInfo.balance_bcu_open = false;
                }
                else    //均衡关不掉
                {
                    gAFESampleInfo.balance_error_num++;
                    _gAFECtlStat = EN_AFE_CTL_IDLE;
                    break;
                }
            }
            EN_AFE_OP_RES res = _AFEVoltageSampleProcess();
            if(res != EN_AFE_OP_RES_ING)
            {
                _gAFECtlStat = EN_AFE_CTL_IDLE;
                if(res == EN_AFE_OP_RES_ERROR)
                {
                    gAFESampleInfo.cell_v_error_num++;
                }
                else if(res == EN_AFE_OP_RES_COMPLETE)
                {
                    gAFESampleInfo.volt_sample_flag = true;
                    //记录操作时长
                    gAFESampleInfo.cell_v_duration_time = (uint32_t)(tim_get_systime() - sub_timecount[EN_AFE_CTL_V_GET]);
                }
            }
            break;
        }
        case EN_AFE_CTL_T_GET:  //温度采样
        {
            EN_AFE_OP_RES res = _AFETempSampleProcess();
            if(res != EN_AFE_OP_RES_ING)
            {
                _gAFECtlStat = EN_AFE_CTL_IDLE;
                if(res == EN_AFE_OP_RES_ERROR)
                {
                    gAFESampleInfo.cell_t_error_num++;
                }
                else if(res == EN_AFE_OP_RES_COMPLETE)
                {
                    gAFESampleInfo.temp_sample_flag = true;
                    //记录操作时长
                    gAFESampleInfo.cell_t_duration_time = (uint32_t)(tim_get_systime() - sub_timecount[EN_AFE_CTL_T_GET]);
                }
            }
            break;
        }
        case EN_AFE_CTL_BT_GET: //均衡温度采样
        {
            EN_AFE_OP_RES res = _AFEBalanceTempSampleProcess();
            if(res != EN_AFE_OP_RES_ING)
            {
                _gAFECtlStat = EN_AFE_CTL_IDLE;
                if(res == EN_AFE_OP_RES_ERROR)
                {
                    gAFESampleInfo.bt_error_num++;
                }
                else if(res == EN_AFE_OP_RES_COMPLETE)
                {
                    gAFESampleInfo.balance_temp_sample_flag = true;
                    //记录操作时长
                    gAFESampleInfo.bt_duration_time = (uint32_t)(tim_get_systime() - sub_timecount[EN_AFE_CTL_BT_GET]);
                }
            }
            break;
        }
        case EN_AFE_CTL_B_SET:  //均衡设置
        {
            EN_AFE_OP_RES res = _AFEBalanceProcess();
            if(res != EN_AFE_OP_RES_ING)
            {
                _gAFECtlStat = EN_AFE_CTL_IDLE;
                if(res == EN_AFE_OP_RES_ERROR)
                {
                    gAFESampleInfo.balance_error_num++;
                }
                else if(res == EN_AFE_OP_RES_COMPLETE)
                {
                    //记录操作时长
                    gAFESampleInfo.balance_duration_time = (uint32_t)(tim_get_systime() - sub_timecount[EN_AFE_CTL_B_SET]);
                }
            }
            break;
        }
        case EN_AFE_CTL_OW_DET: //断线检测
        {
            /*先关闭均衡*/
            if(gAFESampleInfo.balance_bcu_open == true)
            {
                if(DevAFEBalanceStop(gAFESampleInfo.balance_stat_flag) == true)
                {
                    gAFESampleInfo.balance_bcu_open = false;
                }
                else//均衡关不掉
                {
                    gAFESampleInfo.balance_error_num++;
                    _gAFECtlStat = EN_AFE_CTL_IDLE;
                    break;
                }
            }
            EN_AFE_OP_RES res = _AFEOpenWireDetectProcess();
            if(res != EN_AFE_OP_RES_ING)
            {
                _gAFECtlStat = EN_AFE_CTL_IDLE;
                if(res == EN_AFE_OP_RES_ERROR)
                {
                    gAFESampleInfo.ow_error_num++;
                }
                else if(res == EN_AFE_OP_RES_COMPLETE)
                {
                    gAFESampleInfo.ow_sample_flag = true;
                    //记录操作时长
                    gAFESampleInfo.ow_duration_time = (uint32_t)(tim_get_systime() - sub_timecount[EN_AFE_CTL_OW_DET]);
                }
            }
            break;
        }
        default:break;
    }
}









